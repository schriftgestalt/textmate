#include "editor.h"
#include "transform.h"
#include "indent.h"
#include <bundles/src/bundles.h>
#include <command/src/runner.h>
#include <text/src/case.h>
#include <text/src/my_ctype.h>
#include <text/src/classification.h>
#include <text/src/utf8.h>
#include <text/src/hexdump.h>
#include <text/src/newlines.h>
#include <text/src/parse.h>
#include <text/src/tokenize.h>
#include <text/src/trim.h>
#include <io/src/exec.h>

namespace ng
{
	static std::string sanitized_utf8 (std::string str)
	{
		str.erase(utf8::remove_malformed(str.begin(), str.end()), str.end());
		return str;
	}

	find::options_t convert (std::map<std::string, std::string> const& options)
	{
		static struct { std::string key; find::options_t flag; } const optionMap[] =
		{
			{ "fullWordMatch",       find::full_words         },
			{ "ignoreCase",          find::ignore_case        },
			{ "ignoreWhitespace",    find::ignore_whitespace  },
			{ "regularExpression",   find::regular_expression },
			{ "wrapAround",          find::wrap_around        },
		};

		find::options_t res = find::none;
		for(auto const& option : optionMap)
		{
			std::map<std::string, std::string>::const_iterator it = options.find(option.key);
			if(it != options.end() && it->second == "1")
				res = res | option.flag;
		}
		return res;
	}

	template <typename _OutputIter>
	_OutputIter transpose_selections (buffer_t const& _buffer, ranges_t const& _selections, _OutputIter out)
	{
		ranges_t sel;
		for(auto const& range : _selections)
		{
			size_t from = range.min().index, to = range.max().index;
			if(from == to)
			{
				text::pos_t const& pos = _buffer.convert(from);

				if(from == 0 || from == _buffer.size())
				{
				}
				else if(pos.column == 0)
				{
					from = _buffer.begin(pos.line - 1);
					to = pos.line+1 == _buffer.lines() ? _buffer.size() : _buffer.begin(pos.line + 1);
				}
				else if(from == _buffer.eol(pos.line))
				{
					from = _buffer.begin(pos.line);
					to = pos.line+2 == _buffer.lines() ? _buffer.size() : _buffer.begin(pos.line + 2);
				}
				else
				{
					from = from - _buffer[from-1].size();
					to   = to + _buffer[to].size();
				}
				*out++ = std::make_pair(range_t(from, to), transform::transpose(_buffer.substr(from, to)));
			}
			else if(range.columnar) // TODO from.line != to.line
			{
				std::vector<std::string> strings;
				std::vector<range_t> ranges;

				for(auto const& r : dissect_columnar(_buffer, range))
				{
					strings.push_back(_buffer.substr(r.min().index, r.max().index));
					ranges.push_back(r);
				}

				for(size_t i = 0; i < ranges.size(); ++i)
					*out++ = std::make_pair(ranges[i], strings[ranges.size()-1 - i]);
			}
			else
			{
				*out++ = std::make_pair(range_t(from, to), transform::transpose(_buffer.substr(from, to)));
			}
		}
		return out;
	}

	// =============================
	// = Preserve Selection Helper =
	// =============================

	static size_t const kColumnar = 1 << 0;
	static size_t const kReversed = 1 << 1;

	struct preserve_selection_helper_t : callback_t
	{
		preserve_selection_helper_t (buffer_t& buffer, ranges_t const& marks) : _buffer(buffer)
		{
			for(auto const& range : marks)
			{
				if(range.empty())
				{
					_marks.emplace_back(range.first, mark_t::kUnpairedMark);
					_marks.emplace_back(range.first, mark_t::kEndMark);
				}
				else
				{
					size_t userInfo = (range.columnar ? kColumnar : 0) | (range.last < range.first ? kReversed : 0);
					_marks.emplace_back(range.min(), mark_t::kBeginMark, userInfo);
					_marks.emplace_back(range.max(), mark_t::kEndMark, userInfo);
				}
			}

			_buffer.add_callback(this);
		}

		~preserve_selection_helper_t ()
		{
			_buffer.remove_callback(this);
		}

		ranges_t get ()
		{
			ranges_t sel;
			for(size_t i = 0; i < _marks.size(); i += 2)
			{
				ASSERT(i+1 < _marks.size() && _marks[i+1].type == mark_t::kEndMark);
				if(_marks[i].type == mark_t::kUnpairedMark)
				{
					sel.push_back(_marks[i].position);
				}
				else
				{
					index_t first = _marks[i].position;
					index_t last  = _marks[i+1].position;
					bool columnar = _marks[i].user_info & kColumnar;
					if(_marks[i].user_info & kReversed)
						std::swap(first, last);
					sel.push_back(range_t(first, last, columnar));
				}
			}
			return sel;
		}

		void will_replace (size_t from, size_t to, char const* buf, size_t len)
		{
			for(auto& mark : _marks)
			{
				size_t& index = mark.position.index;
				if(std::clamp(index, from, to) == index)
				{
					if(mark.type == mark_t::kUnpairedMark || index != from && index != to)
					{
						index = from + len - std::min(to - index, len);
					}
					else
					{
						index = from;
						if(mark.type == mark_t::kEndMark)
							index += len;
					}
				}
				else if(from < index)
				{
					ASSERT_LT(to, index);
					index = index + len - (to - from);
				}
			}
		}

	private:
		struct mark_t
		{
			index_t position;
			enum mark_type { kBeginMark, kUnpairedMark, kEndMark } type;
			size_t user_info;

			mark_t (index_t const& position, mark_type type, size_t user_info = 0) : position(position), type(type), user_info(user_info) { }
		};

		buffer_t& _buffer;
		std::vector<mark_t> _marks;
	};

	// =======================================================
	// = Transform ranges in buffer, accounting for snippets =
	// =======================================================

	template <typename F>
	std::multimap<range_t, std::string> map (ng::buffer_t const& buffer, ng::ranges_t const& selections, F op)
	{
		std::multimap<range_t, std::string> replacements;
		for(auto const& range : dissect_columnar(buffer, selections))
			replacements.emplace(range, op(buffer.substr(range.min().index, range.max().index)));
		return replacements;
	}

	static ranges_t replace_helper (ng::buffer_t& buffer, snippet_controller_t& snippets, std::multimap<range_t, std::string> const& replacements)
	{
		ranges_t res;

		ssize_t adjustment = 0;
		for(auto const& p1 : replacements)
		{
			range_t orgRange = p1.first.sorted();
			if(orgRange.first.index == orgRange.last.index && p1.second.empty())
			{
				res.push_back(orgRange + adjustment);
				continue;
			}

			std::string const pad = orgRange.freehanded && orgRange.first.carry ? std::string(orgRange.first.carry, ' ') : "";
			orgRange.first.carry = orgRange.last.carry = 0;
			ng::range_t original(orgRange.first.index + pad.size(), orgRange.first.index + pad.size() + p1.second.size());
			bool goodCaretMatch = false;

			std::vector< std::pair<range_t, std::string> > const& real = snippets.replace(orgRange.first.index, orgRange.last.index, pad + p1.second);
			for(auto const& p2 : real)
			{
				range_t const& range   = p2.first;
				std::string const& str = p2.second;

				size_t from = range.first.index + adjustment, to = range.last.index + adjustment;
				size_t caret = buffer.replace(from, to, str);
				if(snippets.empty() && range == ng::range_t(orgRange.first.index, orgRange.last.index))
				{
					original = range_t(from + pad.size(), caret, false, orgRange.freehanded, true);
				}
				else if(!snippets.empty() && !goodCaretMatch && snippets.current().min().index <= from + pad.size() && caret <= snippets.current().max().index)
				{
					// FIXME This is a heuristic to figure out which of the returned ranges correspond to our caret, as multiple ranges will be returned when the snippet has mirrors. Ideally we would update the API to return the new caret range. Though I am postponing that to a potential refactor that would also allow snippets to work with multiple carets.

					original = ng::range_t(from + pad.size(), caret, false, orgRange.freehanded, true);
					goodCaretMatch = snippets.current().min().index < from + pad.size() && caret < snippets.current().max().index || (from + pad.size() != caret && (snippets.current().min().index < from + pad.size() || caret < snippets.current().max().index));
				}
				adjustment += str.size() - (to - from);
			}
			res.push_back(original);
			res.last().color = orgRange.color;
		}
		return res;
	}

	template <typename F>
	ng::ranges_t apply (ng::buffer_t& buffer, ng::ranges_t const& selections, snippet_controller_t& snippets, F op)
	{
		ng::ranges_t res;
		for(auto const& range : replace_helper(buffer, snippets, map(buffer, selections, op)))
			res.push_back(range.color ? range : range.max());
		return res;
	}

	// ============
	// = editor_t =
	// ============

	static ng::buffer_t dummy;

	void editor_t::setup ()
	{
		set_clipboard(create_simple_clipboard());
		set_find_clipboard(create_simple_clipboard());
		set_replace_clipboard(create_simple_clipboard());
		set_yank_clipboard(create_simple_clipboard());
	}

	editor_t::editor_t () : _buffer(dummy)
	{
		setup();
	}

	editor_t::editor_t (buffer_t& buffer) : _buffer(buffer)
	{
		setup();
	}

	void editor_t::sanitize_selection ()
	{
		_selections = ng::sanitize(_buffer, _selections);
	}

	clipboard_t::entry_ptr editor_t::copy (ng::buffer_t const& buffer, ng::ranges_t const& selections)
	{
		std::string indent = NULL_STR;
		bool complete      = false;

		if(selections.size() == 1)
		{
			range_t const& sel = selections.last();
			text::pos_t const& from = buffer.convert(sel.min().index);
			text::pos_t const& to   = buffer.convert(sel.max().index);

			if(from.line != to.line)
			{
				if(from.column != 0)
				{
					std::string const& leading = buffer.substr(buffer.begin(from.line), sel.min().index);
					if(text::is_blank(leading.data(), leading.data() + leading.size()))
						indent = leading;
				}

				if(to.column != 0 && sel.max().index == buffer.eol(to.line))
					complete = true;
			}
		}

		std::vector<std::string> v;
		for(auto const& range : dissect_columnar(buffer, selections))
			v.push_back(buffer.substr(range.min().index, range.max().index));
		bool columnar = selections.size() == 1 && selections.last().columnar;
		return std::make_shared<clipboard_t::entry_t>(v, indent, complete, columnar);
	}

	static bool suitable_for_reindent (std::string const& str)
	{
		return oak::contains(str.begin(), str.end(), '\n');
	}

	// ============
	// = Snippets =
	// ============

	bool editor_t::disallow_tab_expansion () const
	{
		if(!_snippets.empty() && _snippets.current() == ranges().last() && !_snippets.in_last_placeholder() || ranges().last().unanchored)
			return true;
		return false;
	}

	ranges_t editor_t::replace (std::multimap<range_t, std::string> const& replacements, bool selectInsertions)
	{
		ranges_t res = replace_helper(_buffer, _snippets, replacements);
		return selectInsertions ? res : ng::move(_buffer, res, kSelectionMoveToEndOfSelection);
	}

	ranges_t editor_t::snippet (size_t from, size_t to, std::string const& str, std::map<std::string, std::string> const& variables, bool disableIndent)
	{
		struct callback_t : snippet::run_command_callback_t
		{
			std::map<std::string, std::string> environment;
			callback_t (std::map<std::string, std::string> const& environment) : environment(environment) { }

			std::string run_command (std::string const& cmd)
			{
				__block int status = 0;
				__block std::string output, error;

				std::string script = cmd;
				command::fix_shebang(&script);
				std::string scriptPath = path::temp("snippet_command", script);

				if(io::process_t process = io::spawn(std::vector<std::string>{ scriptPath }, environment))
				{
					close(process.in);

					dispatch_group_t group = dispatch_group_create();
					dispatch_group_async(group, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
						io::exhaust_fd(process.out, &output);
					});
					dispatch_group_async(group, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
						io::exhaust_fd(process.err, &error);
					});
					dispatch_group_async(group, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
						if(waitpid(process.pid, &status, 0) != process.pid)
							perror("editor_t::snippet: waitpid");
					});
					dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
					dispatch_release(group);
				}

				unlink(scriptPath.c_str());

				std::string const& res = WIFEXITED(status) && WEXITSTATUS(status) == 0 ? output : error;
				return utf8::is_valid(res.begin(), res.end()) ? res : sanitized_utf8(res);
			}

		} callback(variables);

		std::string indent = disableIndent ? "" : _buffer.substr(_buffer.begin(_buffer.convert(from).line), from);
		size_t i = 0;
		while(i < indent.size() && text::is_space(indent[i]))
			++i;
		indent.resize(i);

		snippet::snippet_t const& snippet = snippet::parse(str, variables, indent, _buffer.indent(), &callback);

		std::multimap<range_t, std::string> map;
		map.emplace(range_t(from, to), snippet.text);
		_snippets.push(snippet, this->replace(map, true).last());
		return _snippets.current();
	}

	void editor_t::clear_snippets ()
	{
		_snippets.clear();
	}

	std::vector<std::string> const& editor_t::choices () const
	{
		return _snippets.choices();
	}

	std::string editor_t::placeholder_content (ng::range_t* placeholderSelection) const
	{
		if(_snippets.empty())
			return NULL_STR;
		ng::range_t range = _snippets.current();
		for(auto const& r : _selections)
		{
			if(placeholderSelection && range.min() <= r.min() && r.max() <= range.max())
				*placeholderSelection = ng::range_t(r.min().index - range.min().index, r.max().index - range.min().index);
		}
		return _buffer.substr(range.min().index, range.max().index);
	}

	void editor_t::set_placeholder_content (std::string const& str, size_t selectFrom)
	{
		std::multimap<range_t, std::string> map;
		map.emplace(_snippets.current(), str);
		ng::ranges_t res = this->replace(map, true);
		for(auto& range : res)
			range.min().index += selectFrom;
		_selections = res;
	}

	// ============

	ng::ranges_t editor_t::paste (ng::buffer_t& buffer, ng::ranges_t const& selections, snippet_controller_t& snippets, clipboard_t::entry_ptr entry)
	{
		if(!entry)
			return selections;

		std::vector<std::string> strings = entry->contents();
		size_t const fragments           = strings.size();
		std::string str                  = fragments == 1 ? strings.back() : entry->content();
		str.erase(text::convert_line_endings(str.begin(), str.end(), text::estimate_line_endings(str.begin(), str.end())), str.end());

		std::map<std::string, std::string> options = entry->options();
		std::string const& indent = options[kClipboardOptionIndent];
		bool const complete       = options[kClipboardOptionComplete] == "1";
		bool const columnar       = options[kClipboardOptionColumnar] == "1";

		if((selections.size() != 1 || selections.last().columnar) && (fragments > 1 || oak::contains(str.begin(), str.end(), '\n')))
		{
			std::vector<std::string> words = fragments == 1 ? text::split(str, "\n") : strings;
			if(fragments == 1 && words.size() > 1 && words.back().empty())
				words.pop_back();

			size_t i = 0;
			std::multimap<range_t, std::string> insertions;
			for(auto const& range : dissect_columnar(buffer, selections))
				insertions.emplace(range, words[i++ % words.size()]);
			return ng::move(buffer, replace_helper(buffer, snippets, insertions), kSelectionMoveToEndOfSelection);
		}

		if(fragments > 1 && selections.size() == 1)
		{
			if(columnar)
			{
				index_t caret = dissect_columnar(buffer, selections).last().min();
				size_t n      = buffer.convert(caret.index).line;
				size_t col    = visual_distance(buffer, buffer.begin(n), caret);

				std::multimap<range_t, std::string> insertions;
				for(auto const& line : strings)
				{
					if(n+1 < buffer.lines())
					{
						insertions.emplace(visual_advance(buffer, buffer.begin(n), col), line);
					}
					else if(n < buffer.lines())
					{
						// we special-case this to ensure we do not insert at last line with carry, as that will cause potential following insertions to have a lower index, since those will be at EOB w/o a carry
						index_t pos = visual_advance(buffer, buffer.begin(n), col);
						insertions.emplace(index_t(pos.index), std::string(pos.carry, ' ') + line);
					}
					else
					{
						insertions.emplace(index_t(buffer.size()), "\n" + std::string(col, ' ') + line);
					}
					++n;
				}
				return ng::move(buffer, replace_helper(buffer, snippets, insertions), kSelectionMoveToEndOfSelection).first();
			}
			else
			{
				std::multimap<range_t, std::string> insertions;
				ng::range_t caret = dissect_columnar(buffer, selections).last();
				for(auto const& line : strings)
				{
					insertions.emplace(caret, line);
					caret = caret.max();
				}
				return ng::move(buffer, replace_helper(buffer, snippets, insertions), kSelectionMoveToEndOfSelection);
			}
		}

		if(selections.size() == 1 && suitable_for_reindent(str))
		{
			size_t const index = selections.last().min().index;
			size_t const line  = buffer.convert(index).line;
			std::string const& leftOfCaret = buffer.substr(buffer.begin(line), index);

			if(text::is_blank(leftOfCaret.data(), leftOfCaret.data() + leftOfCaret.size()))
			{
				size_t const tabSize    = buffer.indent().tab_size();
				size_t const indentSize = buffer.indent().indent_size();

				if(indent != NULL_STR)
					str = indent + str;
				if(complete)
				{
					size_t const rightIndex = selections.last().max().index;
					size_t const lastLine  = buffer.convert(rightIndex).line;
					std::string const& rightOfCaret = buffer.substr(rightIndex, buffer.eol(lastLine));
					if(!text::is_blank(rightOfCaret.data(), rightOfCaret.data() + rightOfCaret.size()))
						str += '\n';
				}

				int minIndent = INT_MAX;

				auto const& v = text::to_lines(str.data(), str.data() + str.size());
				for(auto const& it : v)
				{
					if(!text::is_blank(it.first, it.second))
						minIndent = std::min(indent::leading_whitespace(it.first, it.second, tabSize), minIndent);
				}

				plist::any_t pasteBehaviorValue = bundles::value_for_setting("indentOnPaste", buffer.scope(index));
				std::string const* pasteBehavior = boost::get<std::string>(&pasteBehaviorValue);
				if(pasteBehavior && *pasteBehavior == "simple")
				{
					int currentIndent = indent::leading_whitespace(leftOfCaret.data(), leftOfCaret.data() + leftOfCaret.size(), tabSize);
					if(currentIndent)
					{
						str = transform::shift(currentIndent-minIndent, buffer.indent())(str);
						size_t len = str.size();
						while(len > 0 && (str[len-1] == '\t' || str[len-1] == ' '))
							--len;
						str = str.substr(0, len);
						if(!str.empty() && str.back() == '\n')
							str += leftOfCaret;
					}
				}
				else if(!pasteBehavior || *pasteBehavior != "disable")
				{
					indent::fsm_t fsm = indent::create_fsm(buffer, line, indentSize, tabSize);
					auto const patterns = indent::patterns_for_scope(buffer.scope(index));
					for(auto const& it : v)
					{
						if(fsm.is_ignored(std::string(it.first, it.second), patterns))
							continue;
						size_t indent = fsm.scan_line(std::string(it.first, it.second), patterns);
						int oldIndent = indent::leading_whitespace(it.first, it.second, tabSize);
						transform::shift shifter(std::max(((int)indent)-oldIndent, -minIndent), buffer.indent());
						str = shifter(str);
						break;
					}

					if(!str.empty() && str.back() == '\n')
						str += leftOfCaret;
				}
				return ng::move(buffer, replace_helper(buffer, snippets, map(buffer, range_t(buffer.begin(line), selections.last().max()), transform::replace(str))), kSelectionMoveToEndOfSelection);
			}
		}
		return ng::move(buffer, replace_helper(buffer, snippets, map(buffer, selections, transform::replace(str))), kSelectionMoveToEndOfSelection);
	}

	void editor_t::insert (std::string const& str, bool selectInsertion)
	{
		ng::ranges_t res = replace_helper(_buffer, _snippets, map(_buffer, _selections, transform::replace(str)));
		_selections = selectInsertion ? res : ng::move(_buffer, res, kSelectionMoveToEndOfSelection);
	}

	struct indent_helper_t : ng::callback_t
	{
		indent_helper_t (editor_t& editor, buffer_t& buffer, indent_correction_t indentCorrections) : _disabled(indentCorrections == kIndentCorrectNever), _indent_correct(indentCorrections), _editor(editor), _buffer(buffer)
		{
			_disabled = _disabled || editor._selections.size() != 1 || editor._selections.last().columnar;
			if(_disabled)
				return;

			_buffer.add_callback(this);
		}

		void will_replace (size_t from, size_t to, char const* buf, size_t len)
		{
			text::pos_t pos = _buffer.convert(from);

			std::string const line = _buffer.substr(_buffer.begin(pos.line), _buffer.eol(pos.line));
			if(_indent_correct == kIndentCorrectNonEmptyLines && (line.empty() || text::is_blank(line.data(), line.data() + line.size())))
				return;

			indent::fsm_t fsm = indent::create_fsm(_buffer, pos.line, _buffer.indent().indent_size(), _buffer.indent().tab_size());
			bool ignored = fsm.is_ignored(line, indent::patterns_for_line(_buffer, pos.line));
			int actual = indent::leading_whitespace(line.data(), line.data() + line.size(), _buffer.indent().tab_size());
			size_t desired = fsm.scan_line(line, indent::patterns_for_line(_buffer, pos.line));
			if(ignored || actual == desired)
				_lines.emplace(pos.line, actual);
		}

		~indent_helper_t ()
		{
			if(_disabled)
				return;

			_buffer.remove_callback(this);

			std::multimap<range_t, std::string> replacements;
			for(auto const& pair : _lines)
			{
				size_t n = pair.first;
				size_t bol = _buffer.begin(n);
				size_t eos = bol;

				std::string const line = _buffer.substr(bol, _buffer.eol(n));
				int actual = indent::leading_whitespace(line.data(), line.data() + line.size(), _buffer.indent().tab_size());
				if(actual != pair.second)
					continue;

				indent::fsm_t fsm = indent::create_fsm(_buffer, n, _buffer.indent().indent_size(), _buffer.indent().tab_size());
				auto const patterns = indent::patterns_for_line(_buffer, n);
				size_t desired = fsm.scan_line(line, patterns);
				if(!fsm.is_ignored(line, patterns) && desired != actual)
				{
					while(eos != _buffer.size() && text::is_whitespace(_buffer[eos]))
						eos += _buffer[eos].size();
					replacements.emplace(range_t(bol, eos), indent::create(desired, _buffer.indent().tab_size(), _buffer.indent().soft_tabs()));
				}
			}

			if(!replacements.empty())
			{
				preserve_selection_helper_t helper(_buffer, _editor._selections);
				_editor.replace(replacements);
				_editor._selections = helper.get();
			}
		}

	private:
		bool _disabled;
		indent_correction_t _indent_correct;
		editor_t& _editor;
		buffer_t& _buffer;
		std::map<size_t, int> _lines;
	};

	static std::string find_paired (std::string const& str, scope::context_t const& scope)
	{
		plist::any_t typingPairs = bundles::value_for_setting("smartTypingPairs", scope);
		if(plist::array_t const* typingPairsArray = boost::get<plist::array_t>(&typingPairs))
		{
			for(auto const& pair : *typingPairsArray)
			{
				if(plist::array_t const* pairArray = boost::get<plist::array_t>(&pair))
				{
					if(pairArray->size() == 2)
					{
						std::string const* left  = boost::get<std::string>(&(*pairArray)[0]);
						std::string const* right = boost::get<std::string>(&(*pairArray)[1]);
						if(left && *left == str && right)
							return *right;
					}
				}
			}
		}
		return NULL_STR;
	}

	void editor_t::insert_with_pairing (std::string const& str, indent_correction_t indentCorrections, bool autoPairing, std::string const& scopeAttributes)
	{
		if(autoPairing && !has_selection())
		{
			size_t const caret = _selections.last().last.index;
			if(_buffer.pairs().is_last(caret) && caret + str.size() <= _buffer.size() && str == _buffer.substr(caret, caret + str.size()))
			{
				_selections = ng::move(_buffer, _selections, kSelectionMoveRight);
				_buffer.pairs().remove(caret);
				return;
			}
		}

		indent_helper_t indent_helper(*this, _buffer, indentCorrections);
		std::string const autoInsert = autoPairing ? find_paired(str, scope(scopeAttributes)) : NULL_STR;
		if(autoInsert != NULL_STR && has_selection())
		{
			_selections = replace_helper(_buffer, _snippets, map(_buffer, _selections, transform::surround(str, autoInsert)));
		}
		else if(autoInsert == NULL_STR || has_selection() || _selections.size() != 1 || _selections.last().columnar || (_selections.last().min().index < _buffer.size() && text::is_word_char(_buffer[_selections.last().min().index])))
		{
			insert(str);
		}
		else
		{
			if(str == autoInsert && str.size() == 1)
			{
				size_t const lineNo = _buffer.convert(_selections.last().last.index).line;
				std::string line = _buffer.substr(_buffer.begin(lineNo), _buffer.eol(lineNo));
				if(std::count(line.begin(), line.end(), str[0]) % 2 == 1)
					return insert(str);
			}

			insert(str + autoInsert);
			_selections = ng::move(_buffer, _selections, kSelectionMoveLeft);
			size_t const caret = _selections.last().last.index;
			_buffer.pairs().add_pair(caret - str.size(), caret);
		}
	}

	void editor_t::move_selection_to (ng::index_t const& index, bool selectInsertion)
	{
		std::vector<std::string> v;
		std::multimap<range_t, std::string> insertions;
		for(auto const& range : dissect_columnar(_buffer, _selections))
		{
			v.push_back(_buffer.substr(range.min().index, range.max().index));
			insertions.emplace(range, "");
		}
		insertions.emplace(index, text::join(v, "\n"));
		ranges_t sel, tmp = this->replace(insertions, selectInsertion);
		for(auto const& range : tmp)
		{
			if(!range.empty())
				sel.push_back(range);
		}
		_selections = sel.empty() ? tmp : sel;
	}

	void editor_t::snippet (std::string const& str, std::map<std::string, std::string> const& variables, bool disableIndent)
	{
		size_t from = _selections.last().min().index;
		size_t to   = _selections.last().max().index;
		_selections = this->snippet(from, to, str, variables, disableIndent);
	}

	void editor_t::perform (action_t action, layout_movement_t const* layout, indent_correction_t indentCorrections, std::string const& scopeAttributes)
	{
		static std::string const kSingleMarkType = "•";
		preserve_selection_helper_t selectionHelper(_buffer, _selections);

		if(action == kDeleteBackward && !has_selection() && _selections.size() == 1)
		{
			size_t caret = _selections.last().last.index;
			size_t from  = caret - (0 < caret ? _buffer[caret-1].size() : 0);
			if(caret && _buffer.pairs().is_first(from))
			{
				size_t other = _buffer.pairs().counterpart(from);
				size_t to    = other + _buffer[other].size();
				if(other == caret)
				{
					_selections = range_t(from, to);
					action = kDeleteSelection;
				}
				else
				{
					std::multimap<range_t, std::string> insertions;
					insertions.emplace(range_t(from, caret), "");
					insertions.emplace(range_t(other, to), "");
					_selections = this->replace(insertions).first();
					action = kNop;
				}
			}
		}

		switch(action)
		{
			case kDeleteBackward:                               _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendLeft,               layout); break;
			case kDeleteBackwardByDecomposingPreviousCharacter: _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendLeft,               layout); break;
			case kDeleteForward:                                _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendRight,              layout); break;
			case kDeleteSubWordLeft:                            _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToBeginOfSubWord,   layout); break;
			case kDeleteSubWordRight:                           _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToEndOfSubWord,     layout); break;
			case kDeleteWordBackward:                           _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToBeginOfWord,      layout); break;
			case kDeleteWordForward:                            _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToEndOfWord,        layout); break;
			case kDeleteToBeginningOfIndentedLine:              _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToBeginOfIndentedLine, layout); break;
			case kDeleteToEndOfIndentedLine:                    _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToEndOfIndentedLine, layout); break;
			case kDeleteToBeginningOfLine:                      _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToBeginOfSoftLine,  layout); break;
			case kDeleteToEndOfLine:                            _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToEndOfSoftLine,    layout); break;
			case kDeleteToBeginningOfParagraph:                 _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToBeginOfParagraph, layout); break;
			case kDeleteToEndOfParagraph:                       _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToEndOfParagraph,   layout); break;

			case kChangeCaseOfWord:                             _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToEndOfWord,        layout); break;
			case kChangeCaseOfLetter:                           _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendRight,              layout); break;
			case kUppercaseWord:                                _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToWord,             layout); break;
			case kLowercaseWord:                                _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToWord,             layout); break;
			case kCapitalizeWord:                               _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToLineExclLF,       layout); break;
			case kReformatText:                                 _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToParagraph,        layout); break;
			case kReformatTextAndJustify:                       _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToParagraph,        layout); break;
			case kUnwrapText:                                   _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToParagraph,        layout); break;
			case kMoveSelectionUp:                              _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToLine,             layout); break;
			case kMoveSelectionDown:                            _selections = ng::extend_if_empty(_buffer, _selections, kSelectionExtendToLine,             layout); break;
			case kShiftLeft:                                    _selections = ng::extend(_buffer, _selections, kSelectionExtendToLineExclLF,                layout); break;
			case kShiftRight:                                   _selections = ng::extend(_buffer, _selections, kSelectionExtendToLineExclLF,                layout); break;
		}

		static std::set<action_t> const deleteActions      = { kDeleteBackward, kDeleteForward };
		static std::set<action_t> const yankAppendActions  = { kDeleteSubWordRight, kDeleteWordForward,  kDeleteToEndOfIndentedLine,       kDeleteToEndOfLine,       kDeleteToEndOfParagraph };
		static std::set<action_t> const yankPrependActions = { kDeleteSubWordLeft,  kDeleteWordBackward, kDeleteToBeginningOfIndentedLine, kDeleteToBeginningOfLine, kDeleteToBeginningOfParagraph };
		if(deleteActions.find(action) != deleteActions.end())
			action = kDeleteSelection;
		else if(yankAppendActions.find(action) != yankAppendActions.end())
			action = _extend_yank_clipboard ? kAppendSelectionToYankPboard : kCopySelectionToYankPboard;
		else if(yankPrependActions.find(action) != yankPrependActions.end())
			action = _extend_yank_clipboard ? kPrependSelectionToYankPboard : kCopySelectionToYankPboard;
		_extend_yank_clipboard = false;

		switch(action)
		{
			case kMoveBackward:                                 _selections = ng::move(_buffer, _selections, kSelectionMoveLeft,              layout); break;
			case kMoveForward:                                  _selections = ng::move(_buffer, _selections, kSelectionMoveRight,             layout); break;
			case kMoveUp:                                       _selections = ng::move(_buffer, _selections, kSelectionMoveUp,                layout); break;
			case kMoveDown:                                     _selections = ng::move(_buffer, _selections, kSelectionMoveDown,              layout); break;
			case kMoveSubWordLeft:                              _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfSubWord,  layout); break;
			case kMoveSubWordRight:                             _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfSubWord,    layout); break;
			case kMoveWordBackward:                             _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfWord,     layout); break;
			case kMoveWordForward:                              _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfWord,       layout); break;
			case kMoveToBeginningOfIndentedLine:                _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfIndentedLine, layout); break;
			case kMoveToEndOfIndentedLine:                      _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfIndentedLine, layout); break;
			case kMoveToBeginningOfLine:                        _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfSoftLine, layout); break;
			case kMoveToEndOfLine:                              _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfSoftLine,   layout); break;
			case kMoveToBeginningOfParagraph:                   _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfLine,     layout); break;
			case kMoveToEndOfParagraph:                         _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfLine,       layout); break;
			case kMoveToBeginningOfBlock:                       _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfTypingPair, layout); break;
			case kMoveToEndOfBlock:                             _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfTypingPair, layout); break;
			case kMoveToBeginningOfColumn:                      _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfColumn,   layout); break;
			case kMoveToEndOfColumn:                            _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfColumn,     layout); break;
			case kMoveToBeginningOfDocument:                    _selections = ng::move(_buffer, _selections, kSelectionMoveToBeginOfDocument, layout); break;
			case kMoveToEndOfDocument:                          _selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfDocument,   layout); break;
			case kPageUp:                                       _selections = ng::move(_buffer, _selections, kSelectionMovePageUp,            layout); break;
			case kPageDown:                                     _selections = ng::move(_buffer, _selections, kSelectionMovePageDown,          layout); break;

			case kMoveBackwardAndModifySelection:               _selections = ng::extend(_buffer, _selections, kSelectionExtendLeft,               layout); break;
			case kMoveForwardAndModifySelection:                _selections = ng::extend(_buffer, _selections, kSelectionExtendRight,              layout); break;
			case kMoveUpAndModifySelection:                     _selections = ng::extend(_buffer, _selections, kSelectionExtendUp,                 layout); break;
			case kMoveDownAndModifySelection:                   _selections = ng::extend(_buffer, _selections, kSelectionExtendDown,               layout); break;
			case kMoveSubWordLeftAndModifySelection:            _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfSubWord,   layout); break;
			case kMoveSubWordRightAndModifySelection:           _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfSubWord,     layout); break;
			case kMoveWordBackwardAndModifySelection:           _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfWord,      layout); break;
			case kMoveWordForwardAndModifySelection:            _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfWord,        layout); break;
			case kMoveToBeginningOfIndentedLineAndModifySelection: _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfIndentedLine, layout); break;
			case kMoveToEndOfIndentedLineAndModifySelection:    _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfIndentedLine, layout); break;
			case kMoveToBeginningOfLineAndModifySelection:      _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfSoftLine,  layout); break;
			case kMoveToEndOfLineAndModifySelection:            _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfSoftLine,    layout); break;
			case kMoveToBeginningOfParagraphAndModifySelection: _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfParagraph, layout); break;
			case kMoveToEndOfParagraphAndModifySelection:       _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfParagraph,   layout); break;
			case kMoveParagraphBackwardAndModifySelection:      _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfLine,      layout); break;
			case kMoveParagraphForwardAndModifySelection:       _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfLine,        layout); break;
			case kMoveToBeginningOfBlockAndModifySelection:     _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfTypingPair, layout); break;
			case kMoveToEndOfBlockAndModifySelection:           _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfTypingPair,  layout); break;
			case kMoveToBeginningOfColumnAndModifySelection:    _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfColumn,    layout); break;
			case kMoveToEndOfColumnAndModifySelection:          _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfColumn,      layout); break;
			case kMoveToBeginningOfDocumentAndModifySelection:  _selections = ng::extend(_buffer, _selections, kSelectionExtendToBeginOfDocument,  layout); break;
			case kMoveToEndOfDocumentAndModifySelection:        _selections = ng::extend(_buffer, _selections, kSelectionExtendToEndOfDocument,    layout); break;
			case kPageUpAndModifySelection:                     _selections = ng::extend(_buffer, _selections, kSelectionExtendPageUp,             layout); break;
			case kPageDownAndModifySelection:                   _selections = ng::extend(_buffer, _selections, kSelectionExtendPageDown,           layout); break;

			case kSelectAll:                                    _selections = ng::extend(_buffer, _selections, kSelectionExtendToAll,              layout); break;
			case kSelectCurrentScope:                           _selections = ng::extend(_buffer, _selections, kSelectionExtendToScope,            layout); break;
			case kSelectBlock:                                  _selections = ng::extend(_buffer, _selections, kSelectionExtendToTypingPair,       layout); break;
			case kSelectHardLine:                               _selections = ng::extend(_buffer, _selections, kSelectionExtendToLine,             layout); break;
			case kSelectLine:                                   _selections = ng::extend(_buffer, _selections, kSelectionExtendToSoftLine,         layout); break;
			case kSelectParagraph:                              _selections = ng::extend(_buffer, _selections, kSelectionExtendToParagraph,        layout); break;
			case kSelectWord:                                   _selections = ng::extend(_buffer, _selections, kSelectionExtendToWord,             layout); break;
			case kToggleColumnSelection:                        _selections = ng::toggle_columnar(_selections);                                             break;
			case kDeselectLast:                                 _selections = ng::deselect_last(_selections);                                               break;

			case kFindNext:
			case kFindPrevious:
			case kFindNextAndModifySelection:
			case kFindPreviousAndModifySelection:
			case kFindAll:
			case kFindAllInSelection:
			{
				if(clipboard_t::entry_ptr findEntry = find_clipboard()->current())
				{
					find::options_t options = convert(findEntry->options());
					if(action == kFindNextAndModifySelection || action == kFindPreviousAndModifySelection)
						options = options | find::extend_selection;
					if(action == kFindPrevious || action == kFindPreviousAndModifySelection)
						options = options | find::backwards;
					else if(action == kFindAll || action == kFindAllInSelection)
						options = options | find::all_matches;

					find(findEntry->content(), options, (action == kFindAll || action == kFindAllInSelection) && has_selection());
				}
			}
			break;

			case kReplaceAll:
			case kReplaceAllInSelection:
			{
				clipboard_t::entry_ptr findEntry    = find_clipboard()->current();
				clipboard_t::entry_ptr replaceEntry = replace_clipboard()->current();
				if(findEntry && replaceEntry)
				{
					find::options_t options = convert(findEntry->options());
					if(action == kReplaceAll || action == kReplaceAllInSelection)
						options = options | find::all_matches;

					replace_all(findEntry->content(), replaceEntry->content(), options, action == kReplaceAllInSelection);
				}
			}
			break;

			case kReplace:
			case kReplaceAndFind:
			{
				if(clipboard_t::entry_ptr replaceEntry = replace_clipboard()->current())
				{
					std::string replacement = replaceEntry->content();
					if(!match_captures().empty())
						replacement = format_string::expand(replacement, match_captures());
					insert(replacement, true);
				}

				if(action == kReplaceAndFind)
					perform(kFindNext, layout, indentCorrections, scopeAttributes);
			}
			break;

			case kCopySelectionToYankPboard:
			case kAppendSelectionToYankPboard:
			case kPrependSelectionToYankPboard:
			{
				clipboard_t::entry_ptr entry = copy(_buffer, _selections);
				if(action != kCopySelectionToYankPboard && !yank_clipboard()->empty())
				{
					if(clipboard_t::entry_ptr oldEntry = yank_clipboard()->current())
					{
						if(action == kAppendSelectionToYankPboard)
							entry = std::make_shared<clipboard_t::entry_t>(oldEntry->content() + entry->content());
						else if(action == kPrependSelectionToYankPboard)
							entry = std::make_shared<clipboard_t::entry_t>(entry->content() + oldEntry->content());
					}
				}
				yank_clipboard()->push_back(entry);
				_extend_yank_clipboard = true;
			}
			// continue

			case kDeleteSelection:
			{
				indent_helper_t indent_helper(*this, _buffer, indentCorrections);
				_selections = apply(_buffer, _selections, _snippets, &transform::null);
			}
			break;

			case kDeleteBackwardByDecomposingPreviousCharacter: _selections = apply(_buffer, _selections, _snippets, &transform::decompose);  break;

			case kSetMark:
			{
				_buffer.remove_all_marks(kSingleMarkType);
				_buffer.set_mark(_selections.last().last.index, kSingleMarkType);
			}
			break;

			case kDeleteToMark:
			{
				std::map<size_t, std::string> const& marks = _buffer.get_marks(0, _buffer.size(), kSingleMarkType);
				if(marks.size() == 1)
				{
					std::multimap<range_t, std::string> replacements;
					replacements.emplace(range_t(_selections.last().last, marks.begin()->first), "");
					_selections = this->replace(replacements);
				}
			}
			break;

			case kSelectToMark:
			{
				std::map<size_t, std::string> const& marks = _buffer.get_marks(0, _buffer.size(), kSingleMarkType);
				if(marks.size() == 1)
					_selections = ng::range_t(_selections.last().last, marks.begin()->first);
			}
			break;

			case kSwapWithMark:
			{
				std::map<size_t, std::string> marks = _buffer.get_marks(0, _buffer.size(), kSingleMarkType);
				if(marks.size() == 1)
				{
					_buffer.remove_all_marks(kSingleMarkType);
					_buffer.set_mark(_selections.last().last.index, kSingleMarkType);
					_selections = ng::index_t(marks.begin()->first);
				}
			}
			break;

			case kCut:                                          clipboard()->push_back(copy(_buffer, _selections)); _selections = apply(_buffer, _selections, _snippets, &transform::null); break;
			case kCopy:                                         clipboard()->push_back(copy(_buffer, _selections));                                                                         break;

			case kCopySelectionToFindPboard:
			{
				std::set<std::string> set;
				for(auto const& range : dissect_columnar(_buffer, _selections))
				{
					if(!range.empty())
						set.insert(_buffer.substr(range.min().index, range.max().index));
				}

				if(set.size() > 1)
				{
					std::string regexp;
					for(std::string const& str : set)
						regexp.append("|" + regexp::escape(str));
					find_clipboard()->push_back(regexp.substr(1), { { "regularExpression", "1" } });
				}
				else if(!set.empty())
				{
					find_clipboard()->push_back(*set.begin());
				}
			}
			break;

			case kCopySelectionToReplacePboard:                 replace_clipboard()->push_back(copy(_buffer, dissect_columnar(_buffer, _selections).first()));                                                                 break;
			case kPaste:                                        _selections = paste(_buffer, _selections, _snippets, clipboard()->current());                                               break;
			case kPastePrevious:                                _selections = paste(_buffer, _selections, _snippets, clipboard()->previous());                                              break;
			case kPasteNext:                                    _selections = paste(_buffer, _selections, _snippets, clipboard()->next());                                                  break;
			case kPasteWithoutReindent:                         insert(clipboard()->current()->content());                                                                                  break;

			case kYank:
			{
				if(clipboard_t::entry_ptr entry = yank_clipboard()->current())
					insert(entry->content());
			}
			break;

			case kInsertTab:
			{
				_snippets.drop_for_pos(_selections.last().last.index);
				if(_snippets.next())
						_selections = _snippets.current();
				else	_selections = insert_tab_with_indent(_buffer, _selections, _snippets);
			}
			break;

			case kInsertBacktab:
			{
				_snippets.drop_for_pos(_selections.last().last.index);
				if(_snippets.previous())
					_selections = _snippets.current();
			}
			break;

			case kInsertTabIgnoringFieldEditor:                 insert("\t");                 break;
			case kInsertNewline:                                _selections = insert_newline_with_indent(_buffer, _selections, _snippets); break;
			case kInsertNewlineIgnoringFieldEditor:             insert("\n");                 break;

			case kTranspose:
			{
				std::multimap<range_t, std::string> replacements;
				auto inserter = std::inserter(replacements, replacements.end());
				if(_selections.size() > 1 && not_empty(_buffer, _selections))
				{
					std::multiset<range_t> ranges(_selections.begin(), _selections.end());
					std::vector<std::string> strings;
					std::transform(ranges.begin(), ranges.end(), back_inserter(strings), [this](range_t const& r){ return _buffer.substr(r.min().index, r.max().index); });
					std::next_permutation(strings.begin(), strings.end());
					std::transform(ranges.begin(), ranges.end(), strings.begin(), inserter, [](range_t const& r, std::string const& str){ return std::make_pair(r, str); });
				}
				else
				{
					transpose_selections(_buffer, _selections, inserter);
				}
				_selections = this->replace(replacements, not_empty(_buffer, _selections));
			}
			break;

			case kTransposeWords:
			break;

			case kIndent:
			{
				range_t r = _selections.last();
				text::pos_t p0 = _buffer.convert(r.first.index);
				text::pos_t p1 = _buffer.convert(r.last.index);
				if(p1 < p0)
					std::swap(p0, p1);
				size_t from = p0.line;
				size_t to   = p1.line + (p1.column != 0 || p0.line == p1.line ? 1 : 0);

				indent::fsm_t fsm = indent::create_fsm(_buffer, from, _buffer.indent().indent_size(), _buffer.indent().tab_size());

				std::multimap<range_t, std::string> replacements;
				for(size_t n = from; n < to; ++n)
				{
					size_t bol = _buffer.begin(n);
					size_t eos = bol;

					std::string const line = _buffer.substr(bol, _buffer.eol(n));
					if(text::is_blank(line.data(), line.data() + line.size()))
						continue;

					while(eos != _buffer.size() && text::is_whitespace(_buffer[eos]))
						eos += _buffer[eos].size();

					replacements.emplace(range_t(bol, eos), indent::create(fsm.scan_line(line, indent::patterns_for_line(_buffer, n)), _buffer.indent().tab_size(), _buffer.indent().soft_tabs()));
				}

				if(!replacements.empty())
					_selections = this->replace(replacements);
			}
			break;

			case kShiftLeft:               _selections = apply(_buffer, _selections, _snippets, transform::shift(-_buffer.indent().indent_size(), _buffer.indent())); break;
			case kShiftRight:              _selections = apply(_buffer, _selections, _snippets, transform::shift(+_buffer.indent().indent_size(), _buffer.indent())); break;

			case kChangeCaseOfWord:        _selections = apply(_buffer, _selections, _snippets, &text::opposite_case);   break;
			case kChangeCaseOfLetter:      _selections = apply(_buffer, _selections, _snippets, &text::opposite_case);   break;

			case kUppercaseWord:           _selections = apply(_buffer, _selections, _snippets, &text::uppercase);       break;
			case kLowercaseWord:           _selections = apply(_buffer, _selections, _snippets, &text::lowercase);       break;
			case kCapitalizeWord:          _selections = apply(_buffer, _selections, _snippets, &transform::capitalize); break;

			case kReformatText:
			case kReformatTextAndJustify:
			{
				size_t wrapColumn = layout ? layout->effective_wrap_column() : 80;

				if(_selections.last().columnar)
				{
					text::pos_t const& fromPos = _buffer.convert(_selections.last().min().index);
					text::pos_t const& toPos   = _buffer.convert(_selections.last().max().index);

					size_t fromCol = visual_distance(_buffer, _buffer.begin(fromPos.line), _selections.last().min());
					size_t toCol   = visual_distance(_buffer, _buffer.begin(toPos.line), _selections.last().max());
					if(toCol < fromCol)
						std::swap(fromCol, toCol);
					else if(fromCol == toCol && toCol != 0)
						fromCol = 0;
					else if(fromCol == toCol)
						toCol = std::max(fromCol + 10, wrapColumn);

					size_t const from = _buffer.begin(fromPos.line);
					size_t const to   = _buffer.eol(toPos.line);

					std::string str = _buffer.substr(from, to);
					str = text::trim(str);
					str = transform::unwrap(str);
					if(action == kReformatTextAndJustify)
							str = transform::justify(toCol - fromCol, _buffer.indent().tab_size(), false)(str);
					else	str = transform::reformat(toCol - fromCol, _buffer.indent().tab_size(), false)(str);
					str = transform::shift(fromCol, text::indent_t(8, 8, true))(str);

					std::multimap<range_t, std::string> replacements;
					replacements.emplace(range_t(from, to), str);
					ng::range_t range = this->replace(replacements, true).first();
					_selections = range_t(visual_advance(_buffer, _buffer.begin(_buffer.convert(range.min().index).line), fromCol), visual_advance(_buffer, _buffer.begin(_buffer.convert(range.max().index).line), toCol), true);

					action = kNop; // skip selection preserving
				}
				else
				{
					if(action == kReformatTextAndJustify)
							_selections = apply(_buffer, _selections, _snippets, transform::justify(wrapColumn, _buffer.indent().tab_size()));
					else	_selections = apply(_buffer, _selections, _snippets, transform::reformat(wrapColumn, _buffer.indent().tab_size()));
				}
			}
			break;

			case kUnwrapText:              _selections = apply(_buffer, _selections, _snippets, &transform::unwrap); break;

			case kComplete:
			case kNextCompletion:          next_completion(scopeAttributes);      break;
			case kPreviousCompletion:      previous_completion(scopeAttributes);  break;

			case kMoveSelectionUp:         move_selection( 0, -1); break;
			case kMoveSelectionDown:       move_selection( 0, +1); break;
			case kMoveSelectionLeft:       move_selection(-1,  0); break;
			case kMoveSelectionRight:      move_selection(+1,  0); break;
		}

		static std::set<action_t> const preserveSelectionActions = { kCapitalizeWord, kUppercaseWord, kLowercaseWord, kIndent, kShiftLeft, kShiftRight, kReformatText, kReformatTextAndJustify, kUnwrapText };
		if(preserveSelectionActions.find(action) != preserveSelectionActions.end())
			_selections = selectionHelper.get();
	}

	void editor_t::perform_replacements (std::multimap<std::pair<size_t, size_t>, std::string> const& replacements)
	{
		std::multimap<range_t, std::string> tmp;
		for(auto pair : replacements)
			tmp.emplace(range_t(pair.first.first, pair.first.second), pair.second);

		if(!tmp.empty())
		{
			preserve_selection_helper_t helper(_buffer, _selections);
			this->replace(tmp);
			_selections = helper.get();
		}
	}

	ng::ranges_t editor_t::insert_tab_with_indent (ng::buffer_t& buffer, ng::ranges_t const& selections, snippet_controller_t& snippets)
	{
		bool smartTabEnabled = selections.size() == 1 && !selections.last().columnar;

		std::string estimatedIndent = NULL_STR;
		std::multimap<range_t, std::string> insertions;
		for(auto const& range : dissect_columnar(buffer, selections))
		{
			size_t const from               = range.min().index;
			size_t const firstLine          = buffer.convert(from).line;
			std::string const& leftOfCaret  = buffer.substr(buffer.begin(firstLine), from);

			if(smartTabEnabled && text::is_blank(leftOfCaret.data(), leftOfCaret.data() + leftOfCaret.size()))
			{
				size_t to = range.max().index;

				if(estimatedIndent == NULL_STR)
				{
					size_t const lastLine           = buffer.convert(to).line;
					std::string const& rightOfCaret = buffer.substr(to, buffer.end(lastLine));

					indent::fsm_t fsm       = indent::create_fsm(buffer, firstLine, buffer.indent().indent_size(), buffer.indent().tab_size());
					std::string const& line = leftOfCaret + rightOfCaret;
					size_t existingIndent   = indent::leading_whitespace(line.data(), line.data() + line.size(), buffer.indent().tab_size());
					size_t newIndent        = std::max(fsm.scan_line(line, indent::patterns_for_line(buffer, firstLine)), existingIndent + buffer.indent().indent_size());

					estimatedIndent = indent::create(newIndent - indent::leading_whitespace(leftOfCaret.data(), leftOfCaret.data() + leftOfCaret.size(), buffer.indent().tab_size()), buffer.indent().tab_size(), buffer.indent().soft_tabs());
				}

				while(to != buffer.size() && text::is_whitespace(buffer[to]))
					to += buffer[to].size();

				insertions.emplace(ng::range_t(from, to), estimatedIndent);
			}
			else
			{
				size_t col = visual_distance(buffer, buffer.begin(firstLine), from);
				insertions.emplace(range, buffer.indent().create(col));
			}
		}
		return ng::move(buffer, replace_helper(buffer, snippets, insertions), kSelectionMoveToEndOfSelection);
	}

	ng::ranges_t editor_t::insert_newline_with_indent (ng::buffer_t& buffer, ng::ranges_t const& selections, snippet_controller_t& snippets)
	{
		std::multimap<range_t, std::string> insertions;
		for(auto const& range : dissect_columnar(buffer, selections))
		{
			size_t const from              = range.min().index;
			size_t const firstLine         = buffer.convert(from).line;
			std::string const& leftOfCaret = buffer.substr(buffer.begin(firstLine), from);
			auto const patterns            = indent::patterns_for_line(buffer, firstLine);

			size_t to                       = range.max().index;
			size_t const lastLine           = buffer.convert(to).line;
			std::string const& rightOfCaret = buffer.substr(to, buffer.end(lastLine));

			indent::fsm_t fsm(buffer.indent().indent_size(), buffer.indent().tab_size());
			if(!fsm.is_seeded(leftOfCaret, patterns))
			{
				size_t n = firstLine;
				while(n-- > 0 && !fsm.is_seeded(indent::line(buffer, n), indent::patterns_for_line(buffer, n)))
					continue;
			}

			size_t existingIndent = indent::leading_whitespace(rightOfCaret.data(), rightOfCaret.data() + rightOfCaret.size(), buffer.indent().tab_size());
			size_t newIndent      = fsm.scan_line(rightOfCaret, patterns);

			if(text::is_blank(leftOfCaret.data(), leftOfCaret.data() + leftOfCaret.size()))
					newIndent = indent::leading_whitespace(leftOfCaret.data(), leftOfCaret.data() + leftOfCaret.size(), buffer.indent().tab_size()) + existingIndent;
			else	existingIndent = 0;

			insertions.emplace(range, existingIndent < newIndent ? "\n" + indent::create(newIndent - existingIndent, buffer.indent().tab_size(), buffer.indent().soft_tabs()) : "\n");
		}
		return ng::move(buffer, replace_helper(buffer, snippets, insertions), kSelectionMoveToEndOfSelection);
	}

	void editor_t::move_selection (int deltaX, int deltaY)
	{
		ssize_t adjustment = 0;
		std::map<size_t, std::string> clips;
		for(auto const& range : dissect_columnar(_buffer, _selections).sorted())
		{
			range_t r = range + adjustment;
			std::string const& str = _buffer.substr(r.min().index, r.max().index);
			std::multimap<range_t, std::string> replacements;
			replacements.emplace(r, "");
			ranges_t res = this->replace(replacements);
			clips.emplace(res.first().first.index, str);
			adjustment -= str.size();
		}

		std::multimap<range_t, std::string> replacements;
		for(auto const& pair : clips)
		{
			text::pos_t pos = _buffer.convert(pair.first);
			int line        = pos.line;
			int col         = visual_distance(_buffer, _buffer.begin(line), pair.first, false);

			line = std::clamp(line + deltaY, 0, int(_buffer.lines()-1));
			col  = std::max(col + deltaX, 0);
			replacements.emplace(visual_advance(_buffer, _buffer.begin(line), col, false), pair.second);
		}
		_selections = this->replace(replacements, true);
	}

	bool editor_t::handle_result (std::string const& uncheckedOut, output::type placement, output_format::type format, output_caret::type outputCaret, ng::ranges_t const& inputRanges, std::map<std::string, std::string> const& environment)
	{
		std::string const& out = utf8::is_valid(uncheckedOut.begin(), uncheckedOut.end()) ? uncheckedOut : sanitized_utf8(uncheckedOut);
		ng::range_t inputRange = inputRanges ? inputRanges.last() : ng::range_t();

		if(inputRanges.size() > 1 && format == output_format::text && placement != output::replace_document)
		{
			std::vector<std::string> words = text::split(out, "\n");
			if(words.size() == 2 && words.back().empty())
				words.pop_back();

			size_t i = 0;
			std::multimap<range_t, std::string> insertions;
			for(auto const& range : dissect_columnar(_buffer, inputRanges))
				insertions.emplace(range, words[i++ % words.size()]);
			_selections = replace_helper(_buffer, _snippets, insertions);
			if(outputCaret != output_caret::select_output)
				_selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfSelection);
			return true;
		}

		if(inputRange.columnar)
		{
			text::pos_t const fromPos = _buffer.convert(inputRange.min().index);
			text::pos_t const toPos   = _buffer.convert(inputRange.max().index);

			size_t fromCol = visual_distance(_buffer, _buffer.begin(fromPos.line), inputRange.min());
			size_t toCol   = visual_distance(_buffer, _buffer.begin(toPos.line), inputRange.max());

			if(toCol < fromCol)
			{
				std::swap(fromCol, toCol);
				inputRange.first = visual_advance(_buffer, _buffer.begin(fromPos.line), fromCol);
				inputRange.last  = visual_advance(_buffer, _buffer.begin(toPos.line), toCol);
			}
		}

		range_t range;
		switch(placement)
		{
			case output::replace_input:     range = inputRange;                 break;
			case output::replace_document:  range = range_t(0, _buffer.size()); break;
			case output::at_caret:          range = _selections.last().last;    break;
			case output::after_input:       range = inputRange.max();           break;
			case output::replace_selection: range = _selections.last();         break;
		}

		size_t caret = _selections.last().last.index;
		size_t line = 0, column = 0, offset = 0;
		if(range && format == output_format::text && outputCaret == output_caret::interpolate_by_char)
		{
			ASSERT_LE(range.min().index, caret); ASSERT_LE(caret, range.max().index);
			offset = caret - range.min().index; // TODO we should let offset be number of code points (for non-combining marks)
		}
		else if(format == output_format::text && outputCaret == output_caret::interpolate_by_line)
		{
			line   = _buffer.convert(caret).line;
			column = visual_distance(_buffer, _buffer.begin(line), caret);
		}

		switch(format)
		{
			case output_format::snippet:
			case output_format::snippet_no_auto_indent:
			{
				if(range)
					_selections = range;
				snippet(out, environment, format == output_format::snippet_no_auto_indent);
			}
			break;

			case output_format::text:
			{
				size_t bol = 0, eol = out.find('\n');
				if(placement == output::replace_input && range.columnar && eol != std::string::npos)
				{
					std::multimap<range_t, std::string> replacements;
					for(auto const& r : dissect_columnar(_buffer, range))
					{
						replacements.emplace(r, out.substr(bol == std::string::npos ? out.size() : bol, eol == std::string::npos ? eol : eol - bol));
						bol = eol == std::string::npos ? eol : eol + 1;
						eol = bol == std::string::npos ? bol : out.find('\n', bol);
					}

					_selections = replace_helper(_buffer, _snippets, replacements);
					if(outputCaret != output_caret::select_output)
						_selections = ng::move(_buffer, _selections, kSelectionMoveToEndOfSelection);
				}
				else
				{
					if(range)
						_selections = range;

					insert(out, outputCaret == output_caret::select_output);
				}

				if(range && outputCaret == output_caret::interpolate_by_char)
				{
					offset = utf8::find_safe_end(out.begin(), out.begin() + std::min(offset, out.size())) - out.begin();
					_selections = range_t(range.min().index + offset);
				}
				else if(outputCaret == output_caret::interpolate_by_line)
				{
					_selections = visual_advance(_buffer, _buffer.begin(std::min(line, _buffer.lines()-1)), column);
				}
			}
			break;

			// case output_format::completion_list:
			// {
			// 	std::vector<std::string> completions;
			// 	for(auto const& line : text::tokenize(out.begin(), out.end(), '\n'))
			// 	{
			// 		if(!line.empty())
			// 			completions.push_back(line);
			// 	}
			// 	completion(completions, selection().last());
			// }
			// break;
		}
		return true;
	}

	void editor_t::delete_tab_trigger (std::string const& str)
	{
		ranges_t ranges;
		for(auto const& range : _selections)
		{
			size_t from = range.min().index;
			ranges.push_back(range_t(from - std::min(str.size(), from), range.max()));
		}
		_selections = apply(_buffer, ranges, _snippets, &transform::null);
	}

	scope::context_t editor_t::scope (std::string const& scopeAttributes) const
	{
		return ng::scope(_buffer, _selections, scopeAttributes);
	}

	std::map<std::string, std::string> editor_t::editor_variables (std::string const& scopeAttributes) const
	{
		std::map<std::string, std::string> map = {
			{ "TM_TAB_SIZE",  std::to_string(_buffer.indent().tab_size()) },
			{ "TM_SOFT_TABS", _buffer.indent().soft_tabs() ? "YES" : "NO" },
			{ "TM_SELECTION", to_s(_buffer, _selections)                  },
		};

		scope::context_t const& s = scope(scopeAttributes);
		map.emplace("TM_SCOPE", to_s(s.right));

		if(_selections.size() == 1)
		{
			range_t const range = _selections.last();
			if(range.empty())
			{
				size_t const caret = range.last.index;
				text::pos_t const& pos = _buffer.convert(caret);

				map.emplace("TM_LINE_INDEX",    std::to_string(pos.column));
				map.emplace("TM_LINE_NUMBER",   std::to_string(pos.line+1));
				map.emplace("TM_COLUMN_NUMBER", std::to_string(visual_distance(_buffer, _buffer.begin(pos.line), caret)+1));
				map.emplace("TM_CURRENT_LINE",  _buffer.substr(_buffer.begin(pos.line), _buffer.eol(pos.line)));
				map.emplace("TM_SCOPE_LEFT",    to_s(s.left));

				range_t wordRange = ng::word_at(_buffer, range);
				if(!wordRange.empty())
					map.emplace("TM_CURRENT_WORD", _buffer.substr(wordRange.min().index, wordRange.max().index));
			}
			else
			{
				if(32 + range.max().index - range.min().index < ARG_MAX)
				{
					bool first = true;
					for(auto const& r : dissect_columnar(_buffer, range))
					{
						if(std::exchange(first, false))
								map["TM_SELECTED_TEXT"] = "";
						else	map["TM_SELECTED_TEXT"] += "\n";
						map["TM_SELECTED_TEXT"] += _buffer.substr(r.min().index, r.max().index);
					}
				}
				else
				{
					map.emplace("TM_SELECTED_TEXT", text::format("Error: Selection exceeds %s. Command should read selection from stdin.", text::format_size(ARG_MAX-32).c_str()));
				}
			}
		}
		return map;
	}

	// ========
	// = Find =
	// ========

	void editor_t::find (std::string const& searchFor, find::options_t options, bool searchOnlySelection)
	{
		set_match_captures(std::map<std::string, std::string>());

		ranges_t res;
		if(options & find::all_matches)
		{
			for(auto const& pair : ng::find_all(_buffer, searchFor, options, searchOnlySelection ? _selections : ranges_t()))
				res.push_back(pair.first);

			if(searchOnlySelection && res.sorted() == _selections.sorted())
			{
				res = ranges_t();
				for(auto const& pair : ng::find_all(_buffer, searchFor, options, ranges_t()))
					res.push_back(pair.first);
			}
		}
		else
		{
			auto matches = ng::find(_buffer, _selections, searchFor, options);
			for(auto const& pair : matches)
				res.push_back(pair.first);

			if(matches.size() == 1)
				set_match_captures(matches.begin()->second);
		}

		if(!res.empty())
			_selections = res;
	}

	ranges_t editor_t::replace_all (std::string const& searchFor, std::string const& replaceWith, find::options_t options, bool searchOnlySelection)
	{
		ranges_t res;
		if(options & find::all_matches)
		{
			preserve_selection_helper_t helper(_buffer, _selections);
			std::multimap<range_t, std::string> replacements;
			for(auto const& pair : ng::find_all(_buffer, searchFor, options, searchOnlySelection ? _selections : ranges_t()))
				replacements.emplace(pair.first, options & find::regular_expression ? format_string::expand(replaceWith, pair.second) : replaceWith);
			res = this->replace(replacements, true);
			_selections = helper.get();
		}
		return res;
	}

} /* ng */

#include "indent.h"
#include <text/src/my_ctype.h>
#include <text/src/format.h>
#include <oak/oak.h>
#include <oak/debug.h>

namespace indent
{
	std::string create (size_t size, size_t tabSize, bool softTabs)
	{
		return softTabs ? std::string(size, ' ') : std::string(size / tabSize, '\t') + std::string(size % tabSize, ' ');
	}

	int leading_whitespace (char const* it, char const* last, size_t tabSize)
	{
		int res = 0;
		for(; it != last && text::is_space(*it); ++it)
			res += *it == '\t' ? tabSize - (res % tabSize) : 1;
		return res;
	}

	static size_t classify (std::string const& line, std::map<pattern_type, regexp::pattern_t> const& patterns)
	{
		size_t res = 0;
		for(auto pair : patterns)
		{
			if(search(pair.second, line.data(), line.data() + line.size()))
				res |= pair.first;
		}

		if(res & kIgnore)
			res = kIgnore;
		else if(res & kZeroIndent)
			res = kZeroIndent;
		else if(res & kIncrease)
			res &= ~kIncreaseNext;

		return res;
	}

	static bool is_blank (std::string const& line)                                           { return text::is_blank(line.data(), line.data() + line.size()); }
	static size_t leading_whitespace (std::string const& line, size_t tabSize)               { return leading_whitespace(line.data(), line.data() + line.size(), tabSize); }

	// =========
	// = fsm_t =
	// =========

	bool fsm_t::is_seeded (std::string const& line, std::map<pattern_type, regexp::pattern_t> const& patterns)
	{
		bool res = true;
		size_t type = classify(line, patterns);
		if(is_blank(line) || (type & (kIgnore|kZeroIndent)))
		{
			res = false;
		}
		else if(++_seen == 1)
		{
			_level       = leading_whitespace(line, _tab_size);
			_carry       = 0;
			_last_type   = type;
			_last_indent = _level;

			if(type & kIncrease)
				_level += _indent_size;
			if(type & kIncreaseNext)
				_carry += _indent_size;

			res = false;
		}
		else if((type & kIncreaseNext) && !(_last_type & (kIncrease | kDecrease)))
		{
			_level = leading_whitespace(line, _tab_size);
			if(_last_type & kIncreaseNext && _level < _last_indent)
				_carry += _last_indent - _level;
			_last_indent = _level;

			res = false;
		}

		return res;
	}

	bool fsm_t::is_ignored (std::string const& line, std::map<pattern_type, regexp::pattern_t> const& patterns) const
	{
		return is_blank(line) || (classify(line, patterns) & kIgnore);
	}

	size_t fsm_t::scan_line (std::string const& line, std::map<pattern_type, regexp::pattern_t> const& patterns)
	{
		int type = classify(line, patterns);
		ssize_t res = _level + _carry;
		if(type & kZeroIndent)
		{
			res = 0;
		}
		else if(!(type & kIgnore))
		{
			if(type & (kIncrease | kDecrease))
				_carry = 0;
			if(type & kDecrease && (!(type & kIncrease) || _level > 0))
				_level -= _indent_size;

			res = _level + _carry;

			if(type & kIncrease)
				_level += _indent_size;
			_carry = type & kIncreaseNext ? _carry + _indent_size : 0;
		}
		return res < 0 ? 0 : res;
	}

} /* indent */

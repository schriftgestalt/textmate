# TextMate

## Download

You can [download TextMate from here](https://macromates.com/download).

## Feedback

You can use [the TextMate mailing list](https://lists.macromates.com/listinfo/textmate) or [#textmate][] IRC channel on [freenode.net][] for questions, comments, and bug reports.

You can also [contact MacroMates](https://macromates.com/support).

Before you submit a bug report please read the [writing bug reports](https://github.com/textmate/textmate/wiki/writing-bug-reports) instructions.

## Screenshot

![textmate](https://raw.github.com/textmate/textmate/gh-pages/images/screenshot.png)

# Building

TextMate builds with **Xcode**. The Xcode project is self-contained — it compiles the checked-in generated sources directly and links the dependencies below.

## Setup

Install the build dependencies with [Homebrew][]:

```sh
brew install boost capnp google-sparsehash
```

`ragel` and `multimarkdown` are only needed if you regenerate the checked-in generated sources (see below); they are not required for a normal build.

Make sure you have a full checkout including submodules:

```sh
git clone --recursive https://github.com/textmate/textmate.git
cd textmate
```

### Build the Onigmo dependency

The regular-expression library (Onigmo) is vendored and built with its own autotools setup, producing the static `libonig.a` that the app links against. Build it once after checkout:

```sh
cd vendor/Onigmo/vendor
./configure && make
cd -
```

## Building the app

Open `Applications/TextMate/TextMate.xcodeproj` in Xcode and build/run the `TextMate` scheme (⌘R), or from the command line:

```sh
xcodebuild -project Applications/TextMate/TextMate.xcodeproj -scheme TextMate -configuration Debug build
```

## Regenerating generated sources (optional)

Some sources are generated and committed to the repository, so a normal build does not run any code generators. Regenerate them only when you edit the corresponding schema/source:

 * **Cap’n Proto** — `*.capnp` → `*.capnp.c++` / `*.capnp.h` (requires `capnp`)
 * **Ragel** — `Frameworks/plist/src/ascii.rl` → `ascii.cc` (requires `ragel`)

# Legal

The source for TextMate is released under the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

TextMate is a trademark of Allan Odgaard.

[boost]:         http://www.boost.org/
[multimarkdown]: http://fletcherpenney.net/multimarkdown/
[ragel]:         http://www.complang.org/ragel/
[capnp]:         https://github.com/capnproto/capnproto.git
[Homebrew]:      http://brew.sh/
[sparsehash]:    https://code.google.com/p/sparsehash/
[#textmate]:     irc://irc.freenode.net/#textmate
[freenode.net]:  http://freenode.net/

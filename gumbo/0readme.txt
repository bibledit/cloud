The old gumbo parser from https://github.com/google/gumbo-parser is no longer being maintained.
The new gumbo parser from https://codeberg.org/gumbo-parser/gumbo-parser is being maintained.
The new gumbo parser has been integrated in the Bibledit source tree.
This gives more control over it.

How it was integrated:
1. Clone the repository.
2. Run ./autogen.sh
3. Run ./configure
4. Run make
5. Copy all code from the src folder to the gumbo folder in the Bibledit source tree.
Note that the configure action generates a header file that is required to build.

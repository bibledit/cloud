The gumbo parser is no longer being maintained.
See https://github.com/google/gumbo-parser for more details.
The gumbo parser has been integrated in the Bibledit source tree.
This gives more control over it.
Eventually it might be better to look for another parser.

How it was integrated:
1. Clone the repository.
2. Run ./autogen.sh
3. Run ./configure
4. Run make
5. Copy all code from the src folder to the gumbo folder in the Bibledit source tree.
Note that the above generates a header file that is required to build.

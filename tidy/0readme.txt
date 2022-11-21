Html Tidy
=========

https://github.com/htacg/tidy-html5

Crashes
=======

The tidy library is not built well on Debian and derivates at the time of writing this.

Running the unit tests for the Easy English Bible Commentary calls the tidy library.

Here is the crash that the unit test causes on Debian:

foo@bar:~/cloud$ ./unittest
Running unittests
unittest: /build/tidy-html5-93GQBN/tidy-html5-5.6.0/src/config.c:497: prvTidySetOptionBool: Assertion `option_defs[ optId ].type == TidyBoolean' failed.
Aborted

Embedded library
================

The html tidy library is now embedded in the source tree.
This gives Bibledit more control over how this library is built.
See information about how to build it: https://github.com/htacg/tidy-html5/issues/30
This eliminates the crash above.


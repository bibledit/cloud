Html Tidy
=========

https://github.com/htacg/tidy-html5

Crashes
=======

The tidy library is not built properly on Debian and derivates at the time of writing this.

Running the unit tests for the Easy English Bible Commentary calls the tidy library.

Here is the crash that the unit test causes on Debian:

foo@bar:~/cloud$ ./unittest
Running unittests
unittest: /build/tidy-html5-93GQBN/tidy-html5-5.6.0/src/config.c:497: prvTidySetOptionBool: Assertion `option_defs[ optId ].type == TidyBoolean' failed.
Aborted

See more about this crash here: https://github.com/htacg/tidy-html5/issues/30

Embedded library
================

The html tidy library is now embedded in the source tree.
This gives Bibledit more control over how this library is built.
See information about how to build it: https://github.com/htacg/tidy-html5/issues/30
This eliminates the crash above.

Source code preparation
=======================

After importing the source code from GitHub, apply the following modifications to it.

$ cd tidy
Silence several warnings:
$ for file in *.c ; do mv $file tmpc; echo '#pragma clang diagnostic ignored "-Wconversion"' >> $file; cat tmpc >> $file; rm tmpc; done
$ for file in *.h ; do mv $file tmpc; echo '#pragma clang diagnostic ignored "-Wunused-parameter"' >> $file; cat tmpc >> $file; rm tmpc; done
$ for file in *.c ; do mv $file tmpc; echo '#pragma clang diagnostic ignored "-Wcomma"' >> $file; cat tmpc >> $file; rm tmpc; done
Silence this warning: Integer constant not in range of enumerated type <type>
$ for file in *.c ; do mv $file tmpc; echo '#pragma clang diagnostic ignored "-Wassign-enum"' >> $file; cat tmpc >> $file; rm tmpc; done
Silence this warning: Declaration shadows a local variable
$ for file in *.c ; do mv $file tmpc; echo '#pragma clang diagnostic ignored "-Wshadow"' >> $file; cat tmpc >> $file; rm tmpc; done
Silence this warning: Comparison of integers of different signs: <...>
$ for file in *.c ; do mv $file tmpc; echo '#pragma clang diagnostic ignored "-Wsign-compare"' >> $file; cat tmpc >> $file; rm tmpc; done

Remove the assert code that causes the crashes described above:
$ for file in *.c ; do sed -i.bak '/assert/d' $file; done
$ find . -name "*.bak" -delete
Then check changes in git and compile errors, and revert unwanted changes.

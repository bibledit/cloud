#!/bin/sh

FILE=`ls -t /tmp/bibledit-dev/bibles/Sample/1/1 | head -1`
cat /tmp/bibledit-dev/bibles/Sample/1/1/$FILE | head


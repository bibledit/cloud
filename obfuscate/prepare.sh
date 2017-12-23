#!/bin/sh

# Checks for certain words whether they occur in the filenames and in the data.


# Definitions for what to change.
Bibledit=Scripturedit
bibledit=scripturedit
Bible=Scripture
bible=scripture


# Change the program name and installation location.
sed -i '' "s/bibledit/$bibledit/g" configure.ac
if [ $? -ne 0 ]; then exit; fi
sed -i '' "s/Bibledit/$Bibledit/g" configure.ac
if [ $? -ne 0 ]; then exit; fi
# Todo ./reconfigure
if [ $? -ne 0 ]; then exit; fi


# Change any files with the fragment "Bible" in them to "Scripture".
# At the time of writing this script, there was only one file.
# The wildcard ; matches any directory.
# The wildcard * is the well-known one.
# The #1 matches the first wildcard, #2 the second wildcard, and so on.
mmv ";*Bible*" "#1#2$Bible#3"
if [ $? -ne 0 ]; then exit; fi


# Deal with "freebible.html" and where it is called.
mmv ";*freebible*" "#1#2free$bible#3"
if [ $? -ne 0 ]; then exit; fi
find . -name "*.html" -print0 | xargs -0 sed -i '' -e "s/freebible/free$bible/g"
if [ $? -ne 0 ]; then exit; fi


# Deal with "bibledit.xpm" and where it is called.
mmv ";*bibledit.xpm*" "#1#2$bibledit.xpm#3"
if [ $? -ne 0 ]; then exit; fi
sed -i '' "s/bibledit.xpm/$bibledit.xpm/g" Makefile.am executable/bibledit.cpp
if [ $? -ne 0 ]; then exit; fi


# Deal with "bibledit.png" and where it is called.
mmv ";*bibledit.png*" "#1#2$bibledit.png#3"
if [ $? -ne 0 ]; then exit; fi
sed -i '' "s/bibledit.png/$bibledit.png/g" Makefile.am setup/index.html
if [ $? -ne 0 ]; then exit; fi


# Deal with "quickbible.html" and where it is called.
mmv ";*quickbible.html*" "#1#2quick${bible}.html#3"
if [ $? -ne 0 ]; then exit; fi
sed -i '' "s/\"quickbible\"/\"quick${bible}\"/g" export/quickbible.cpp
if [ $? -ne 0 ]; then exit; fi







echo Ready preparing $Bibledit

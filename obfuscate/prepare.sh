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


# The bibledit.desktop has the following lines, among others:
# Name=Bibledit
# Comment=Bible Editor
# Exec=bibledit
# Update those.
sed -i '' "s/Bibledit/${Bibledit}/g" bibledit.desktop
if [ $? -ne 0 ]; then exit; fi
sed -i '' "s/Bible/${Bible}/g" bibledit.desktop
if [ $? -ne 0 ]; then exit; fi
sed -i '' "s/bibledit/${bibledit}/g" bibledit.desktop
if [ $? -ne 0 ]; then exit; fi
# Change the name of the file also.
mmv "bibledit.desktop" "${bibledit}.desktop"
if [ $? -ne 0 ]; then exit; fi
# Change the builder that uses this desktop file.
sed -i '' "s/bibledit.desktop/$bibledit.desktop/g" Makefile.am
if [ $? -ne 0 ]; then exit; fi


# Remove the binary "bibledit" itself.
# The obfuscated binary will have a different name.
rm -f bibledit
if [ $? -ne 0 ]; then exit; fi


# Remove the internationalization file.
# It is not needed.
rm -f locale/bibledit.pot
if [ $? -ne 0 ]; then exit; fi


# Rename the "bibles" folder where to store the Bibles in.
mv bibles ${bible}s
if [ $? -ne 0 ]; then exit; fi
# Update the references to this folder in the code.
sed -i '' "s/\"bibles\"/\"${bible}s\"/g" database/bibles.cpp setup/logic.cpp
if [ $? -ne 0 ]; then exit; fi


# Rename the "databases/config/bible" folder where to store the Bibles configuration data.
mv databases/config/bible databases/config/${bible}
if [ $? -ne 0 ]; then exit; fi
# Update the references to this folder in the code.
sed -i '' "s/\"bible\"/\"${bible}\"/g" database/config/bible.cpp
if [ $? -ne 0 ]; then exit; fi


# Remove the man file as not necessary in this situation.
rm -f man/bibledit.1
sed -i '' "/man_MANS/g" Makefile.am


# Change any files with the fragment "Bible" in them to "Scripture".
# At the time of writing this script, there was only one file.
# The wildcard ; matches any directory.
# The wildcard * is the well-known one.
# The #1 matches the first wildcard, #2 the second wildcard, and so on.
# mmv ";*Bible*" "#1#2$Bible#3"
# if [ $? -ne 0 ]; then exit; fi


# Deal with "freebible.html" and where it is called.
# mmv ";*freebible*" "#1#2free$bible#3"
# if [ $? -ne 0 ]; then exit; fi
# find . -name "*.html" -print0 | xargs -0 sed -i '' -e "s/freebible/free$bible/g"
# if [ $? -ne 0 ]; then exit; fi


# Deal with "bibledit.xpm" and where it is called.
# mmv ";*bibledit.xpm*" "#1#2$bibledit.xpm#3"
# if [ $? -ne 0 ]; then exit; fi
# sed -i '' "s/bibledit.xpm/$bibledit.xpm/g" Makefile.am executable/bibledit.cpp
# if [ $? -ne 0 ]; then exit; fi


# Deal with "bibledit.png" and where it is called.
# mmv ";*bibledit.png*" "#1#2$bibledit.png#3"
# if [ $? -ne 0 ]; then exit; fi
# sed -i '' "s/bibledit.png/$bibledit.png/g" Makefile.am setup/index.html
# if [ $? -ne 0 ]; then exit; fi


# Deal with "quickbible.html" and where it is called.
# mmv ";*quickbible.html*" "#1#2quick${bible}.html#3"
# if [ $? -ne 0 ]; then exit; fi
# sed -i '' "s/\"quickbible\"/\"quick${bible}\"/g" export/quickbible.cpp
# if [ $? -ne 0 ]; then exit; fi




# Todo ./reconfigure
if [ $? -ne 0 ]; then exit; fi




echo Ready preparing $Bibledit

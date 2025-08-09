#!/bin/bash

# Copyright (©) 2003-2025 Teus Benschop.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


# Checks for certain words whether they occur in the filenames and in the data.


# Exit script on error.
set -e


# Definitions for what to change.
Bibledit=Scripturedit
bibledit=scripturedit
Bible=Scripture
bible=scripture


echo Changing the program name and installation location to ${bibledit}/${Bibledit}
sed -i.bak "s/bibledit/$bibledit/g" configure.ac
sed -i.bak "s/Bibledit/$Bibledit/g" configure.ac
# Remove backup file(s).
rm *.bak


echo Updating and renaming bibledit.desktop to ${bibledit}/${Bibledit}
# The bibledit.desktop has the following lines, among others:
# Name=Bibledit
# Comment=Bible Editor
# Exec=bibledit
# Update those.
sed -i.bak "s/Bibledit/${Bibledit}/g" bibledit.desktop
sed -i.bak "s/Bible/${Bible}/g" bibledit.desktop
sed -i.bak "s/bibledit/${bibledit}/g" bibledit.desktop
# Change the name of the file also.
mv "bibledit.desktop" "${bibledit}.desktop"
# Change the builder that uses this desktop file.
sed -i.bak "s/bibledit.desktop/$bibledit.desktop/g" Makefile.am
# Remove backup file(s).
rm *.bak


# Remove the binary "bibledit" itself.
# The obfuscated binary will have a different name.
rm -f bibledit


# Remove the internationalization file.
# It is not needed.
rm -f locale/bibledit.pot


echo Renaming the bibles folder where to store the Bibles to ${bible}s
mv bibles ${bible}s
# Update the references to this folder in the code.
sed -i.bak "s/\"bibles\"/\"${bible}s\"/g" database/bibles.cpp setup/logic.cpp
# Remove backup file(s).
rm database/*.bak
rm setup/*.bak


echo Renaming the databases/config/bible folder where to store the Bibles configuration data
mv databases/config/bible databases/config/${bible}
# Update the references to this folder in the code.
sed -i.bak "s/\"bible\"/\"${bible}\"/g" database/config/bible.cpp
# Remove backup file(s).
rm database/config/*.bak


echo Removing the man file as not needed in this situation
rm -f man/bibledit.1
sed -i.bak "/man_MANS/g" Makefile.am
# Remove backup file(s).
rm *.bak


# Change any files with the fragment "Bible" in them to "Scripture".
# At the time of writing this script, there was only one file.
# The wildcard ; matches any directory.
# The wildcard * is the well-known one.
# The #1 matches the first wildcard, #2 the second wildcard, and so on.
# mmv ";*Bible*" "#1#2$Bible#3"


# Deal with "freebible.html" and where it is called.
# mmv ";*freebible*" "#1#2free$bible#3"
# find . -name "*.html" -print0 | xargs -0 sed -i '' -e "s/freebible/free$bible/g"


# Deal with "bibledit.xpm" and where it is called.
# mmv ";*bibledit.xpm*" "#1#2$bibledit.xpm#3"
# sed -i '' "s/bibledit.xpm/$bibledit.xpm/g" Makefile.am executable/bibledit.cpp


# Deal with "bibledit.png" and where it is called.
# mmv ";*bibledit.png*" "#1#2$bibledit.png#3"
# sed -i '' "s/bibledit.png/$bibledit.png/g" Makefile.am setup/index.html


echo Ready preparing $Bibledit

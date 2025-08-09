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


# Exit script on error.
set -e


cd "$(dirname "$0")"
cd "$(dirname "$PWD")"
SRC=$PWD
echo "Source directory " $SRC
TMP=/tmp/bibledit-i18n
echo Working directory $TMP


echo Include the sid IP address
source ~/scr/sid-ip


echo Synchronizing source files to working directory.
mkdir -p $TMP
rsync -a --delete $SRC/ $TMP/
cd $TMP


echo Removing files not to be processed.
rm -rf autom4te.cache
rm -rf developer
rm -rf dtl
rm -rf flate
rm -rf jquery
rm -rf jsonxx
rm -rf man
rm -rf mbedtls*
rm -rf microtar
rm -rf mimetic098
rm -rf miniz
rm -rf nmt
rm -rf notifit
rm -rf parsewebdata
rm -rf pugixml
rm -rf quill
rm -rf rangy13
rm -rf slip
rm -rf stb
rm -rf sword/*
rm -rf tmp
rm -rf unittest*
rm -rf utf8*


echo Gathering all the html files for internationalization.
find . -iname "*.html" > i18n.html


echo Transfer translatable strings from the html files to a C++ file.
g++ -std=c++17 -I. -I.. i18n/i18n.cpp
./a.out
rm a.out


echo Cleaning up raw string literals.
# Removing C++11 raw string literals
# because xgettext would complain about unterminated string literals in, e.g.:
# string script = R"(
# <div id="defid" style="clear:both"></div>
# )";
#sed -i.bak '/R"(/,/)";/d' lexicon/logic.cpp
#sed -i.bak '/R"(/,/)";/d' bb/book.cpp
#sed -i.bak '/R"(/,/)";/d' menu/logic.cpp
#sed -i.bak '/R"(/,/)";/d' filter/string.cpp
#sed -i.bak '/R"(/,/)";/d' resource/logic.cpp
#sed -i.bak '/R"(/,/)";/d' resource/logic.cpp
#sed -i.bak '/R"(/,/)";/d' i18n/i18n.cpp


echo Create a temporal file containing all the files for internationalization.
find . -iname "*.cpp" -o -iname "books.h" > gettextfiles.txt


# Remove any previous bibledit.pot because it could have strings no longer in use.
rm -f /tmp/bibledit.pot


echo Extracting translatable strings and storing them in bibledit.pot
xgettext --files-from=gettextfiles.txt --default-domain=bibledit --force-po --copyright-holder="Teus Benschop" -o /tmp/bibledit.pot --from-code=UTF-8 --no-location --keyword=translate --language=C++


# The message ids in bibledit.pot are unique already.
# If this were not the case, then $ msguniq could help to make them unique.


echo Copying bibledit.pot into place.
cp /tmp/bibledit.pot $SRC/locale


# Fix bzr: warning: unsupported locale setting on macOS.
# export LC_ALL=C


echo Pull translations from launchpad.net.
#cd
#cd dev/launchpad/po

#rm -f .DS_Store
ssh $DEBIANSID 'cd launchpad/po && bzr pull lp:~teusbenschop/bibledit/translations'


echo Synchronize translations to Bibledit.
#cd
#cd dev/launchpad/po
scp "$DEBIANSID:launchpad/po/*.po" ~/dev/cloud/locale


echo Push new translatable messages to Launchpad.
#cd
#cd dev/launchpad/pot
scp /tmp/bibledit.pot $DEBIANSID:launchpad/pot
ssh $DEBIANSID "cd launchpad/pot && bzr add bibledit.pot"
ssh $DEBIANSID "cd launchpad/pot && bzr commit --message updated_bibledit.pot"
ssh $DEBIANSID "cd launchpad/pot && bzr push"


echo Clean up.
# Remove dates so they don't appear as daily changes.
sed -i.bak '/POT-Creation-Date/d' ~/dev/cloud/locale/*.po ~/dev/cloud/locale/bibledit.pot
sed -i.bak '/X-Launchpad-Export-Date/d' ~/dev/cloud/locale/*.po ~/dev/cloud/locale/bibledit.pot
rm ~/dev/cloud/locale/*.bak
# Remove temporal .pot.
# rm /tmp/bibledit.pot


echo Remember to bump the version number so it reloads the localization database.

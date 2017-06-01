#!/bin/bash

# Copyright (©) 2003-2017 Teus Benschop.

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


# Remove unwanted files.
find . -name ".DS_Store" -delete

# Create file with the directories and files to install in the package data directory.
# Remove the first bit of it.
# Do case folding to get consistent sorting results across Debian and macOS.
find . | cut -c 2- | sort --ignore-case --ignore-leading-blanks --dictionary-order > pkgdata/files.txt
if [ $? -ne 0 ]; then exit 1; fi

# Remove blank lines.
sed -i.bak '/^$/d' pkgdata/files.txt

# Do not install source files.
sed -i.bak '/\.cpp$/d' pkgdata/files.txt
sed -i.bak '/\.c$/d' pkgdata/files.txt
sed -i.bak '/\.h$/d' pkgdata/files.txt
sed -i.bak '/\.hpp$/d' pkgdata/files.txt

# No git repository data.
sed -i.bak '/\.git/d' pkgdata/files.txt

# No build artifacts.
sed -i.bak '/\.deps/d' pkgdata/files.txt
sed -i.bak '/\.dirstamp/d' pkgdata/files.txt
sed -i.bak '/\.o$/d' pkgdata/files.txt
sed -i.bak '/\.a$/d' pkgdata/files.txt
sed -i.bak '/\.tar\.gz$/d' pkgdata/files.txt
sed -i.bak '/autom4te/d' pkgdata/files.txt
sed -i.bak '/~$/d' pkgdata/files.txt
sed -i.bak '/\.o-/d' pkgdata/files.txt
sed -i.bak '/bibledit-cloud/d' pkgdata/files.txt
sed -i.bak '/^\/bibledit$/d' pkgdata/files.txt

# Do not install license files.
# This fixes the lintian warning:
# W: bibledit: extra-license-file usr/share/bibledit-cloud/COPYING
# What happens is that running ./reconfigure creates COPYING.
# That causes the lintian warning.
# So even if present, it should not be installed.
sed -i.bak '/COPYING/d' pkgdata/files.txt

# No Xcode project data.
sed -i.bak '/xcodeproj/d' pkgdata/files.txt

# No Debian packaging information.
sed -i.bak '/debian/d' pkgdata/files.txt

# Clean backup file
rm pkgdata/files.txt.bak

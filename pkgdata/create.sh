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

find . -name ".DS_Store" -delete
find . | cut -c 2- > pkgdata/files.txt
sed -i.bak '/^$/d' pkgdata/files.txt
sed -i.bak '/\.cpp$/d' pkgdata/files.txt
sed -i.bak '/\.c$/d' pkgdata/files.txt
sed -i.bak '/\.h$/d' pkgdata/files.txt
sed -i.bak '/\.hpp$/d' pkgdata/files.txt
sed -i.bak '/\.o$/d' pkgdata/files.txt
sed -i.bak '/\.git/d' pkgdata/files.txt
sed -i.bak '/\.deps/d' pkgdata/files.txt
sed -i.bak '/\.dirstamp/d' pkgdata/files.txt
sed -i.bak '/COPYING/d' pkgdata/files.txt
sed -i.bak '/autom4te/d' pkgdata/files.txt
sed -i.bak '/xcodeproj/d' pkgdata/files.txt
sed -i.bak '/~$/d' pkgdata/files.txt
rm pkgdata/files.txt.bak

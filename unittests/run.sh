#!/bin/bash

# Copyright (©) 2003-2023 Teus Benschop.

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


cd "$(dirname "$0")"
cd "$(dirname "$PWD")"
SRC=$PWD
echo "Source directory " $SRC

#cd unittests
#if [ $? -ne 0 ]; then exit; fi


g++ -std=c++17 -I. -I.. `pkg-config --cflags --libs gtest`  unittests/gunittest.cpp
if [ $? -ne 0 ]; then exit; fi
./a.out "$@"
rm a.out


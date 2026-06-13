#!/bin/bash

# Copyright (©) 2003-2026 Teus Benschop.

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

PROJECT_SOURCE_DIR=$1
CMAKE_INSTALL_PREFIX=$2
PKG_DATA_DIR=$3
COMBINED_INSTALL_DIR=$DESTDIR/$CMAKE_INSTALL_PREFIX/$PKG_DATA_DIR
echo -- Files source directory: "$PROJECT_SOURCE_DIR"
echo -- Destination dir: "$DESTDIR"
echo -- CMake install prefix: "$CMAKE_INSTALL_PREFIX"
echo -- Package data directory: "$PKG_DATA_DIR"
echo -- Combined install directory: "$COMBINED_INSTALL_DIR"

echo -- Installing shared package data files to "$COMBINED_INSTALL_DIR"

FILE=$PROJECT_SOURCE_DIR/pkgdata/files.txt
LINES=$(cat "$FILE")

COUNT=0

for ITEM in $LINES;
do
if [ -d "$PROJECT_SOURCE_DIR/$ITEM" ]
then
mkdir -p "$COMBINED_INSTALL_DIR"/"$ITEM"
else
echo copy "$PROJECT_SOURCE_DIR"/"$ITEM" to "$COMBINED_INSTALL_DIR"/"$ITEM"
cp "$PROJECT_SOURCE_DIR"/"$ITEM" "$COMBINED_INSTALL_DIR"/"$ITEM"
COUNT=$((COUNT + 1))
fi
done

echo -- Installed $COUNT shared package data files

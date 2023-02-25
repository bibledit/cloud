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


SRCDIR=$1
DESTDIR=$2
PKGDATADIR=$3

FILE=$SRCDIR/pkgdata/files.txt
LINES=`cat $FILE`

for ITEM in $LINES;
do
if [ -d "$SRCDIR/$ITEM" ]
then
mkdir -p $DESTDIR$PKGDATADIR$ITEM
else
echo cp $ITEM $DESTDIR$PKGDATADIR$ITEM
cp $SRCDIR/$ITEM $DESTDIR$PKGDATADIR$ITEM
fi
done

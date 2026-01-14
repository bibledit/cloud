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


# Checks for certain words whether they occur in the filenames.

# Do not look in the source code because it is to be removed after build.
find . -name "*bible*" ! -name "*.cpp" ! -name "*.h" ! -name "*.Po" ! -name "*.o" ! -name "*.xccheckout" ! -name "*.xcscheme"
find . -name "*Bible*"

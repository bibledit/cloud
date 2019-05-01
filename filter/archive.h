/*
Copyright (©) 2003-2019 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef INCLUDED_FILTER_ARCHIVE_H
#define INCLUDED_FILTER_ARCHIVE_H


#include <config/libraries.h>


string filter_archive_zip_folder (string folder);
string filter_archive_zip_folder_shell_internal (string folder);
string filter_archive_zip_folder_miniz_internal (string folder);
string filter_archive_unzip (string file);
string filter_archive_unzip_shell_internal (string file);
string filter_archive_unzip_miniz_internal (string zipfile);
string filter_archive_tar_gzip_file (string filename);
string filter_archive_tar_gzip_folder (string folder);
string filter_archive_untar_gzip (string file);
string filter_archive_uncompress (string file);
int filter_archive_is_archive (string file);
string filter_archive_microtar_pack (string tarball, string directory, vector <string> files);
string filter_archive_microtar_unpack (string tarball, string directory);


#endif

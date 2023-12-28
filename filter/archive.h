/*
Copyright (©) 2003-2024 Teus Benschop.

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


#pragma once

#include <config/libraries.h>

std::string filter_archive_zip_folder (std::string folder);
std::string filter_archive_zip_folder_shell_internal (std::string folder);
std::string filter_archive_zip_folder_miniz_internal (std::string folder);
std::string filter_archive_unzip (std::string file);
std::string filter_archive_unzip_shell_internal (std::string file);
std::string filter_archive_unzip_miniz_internal (std::string zipfile);
std::string filter_archive_tar_gzip_file (std::string filename);
std::string filter_archive_tar_gzip_folder (std::string folder);
std::string filter_archive_untar_gzip (std::string file);
std::string filter_archive_uncompress (std::string file);
int filter_archive_is_archive (std::string file);
std::string filter_archive_microtar_pack (std::string tarball, std::string directory, std::vector <std::string> files);
std::string filter_archive_microtar_unpack (std::string tarball, std::string directory);

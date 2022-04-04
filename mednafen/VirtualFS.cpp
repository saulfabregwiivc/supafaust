/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* VirtualFS.cpp:
**  Copyright (C) 2011-2018 Mednafen Team
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <mednafen/mednafen.h>
#include <mednafen/VirtualFS.h>

namespace Mednafen
{

VirtualFS::VirtualFS(char preferred_path_separator_, const std::string& allowed_path_separators_)
	: preferred_path_separator(preferred_path_separator_), allowed_path_separators(allowed_path_separators_)
{

}

VirtualFS::~VirtualFS()
{


}

void VirtualFS::get_file_path_components(const std::string &file_path, std::string* dir_path_out, std::string* file_base_out, std::string *file_ext_out)
{
 const size_t final_ds = file_path.find_last_of(allowed_path_separators); // in file_path
 std::string file_name;
 size_t fn_final_dot;		// in local var file_name
 // Temporary output:
 std::string dir_path, file_base, file_ext;

 if(final_ds == std::string::npos)
 {
  dir_path = ".";
  file_name = file_path;
 }
 else
 {
  dir_path = file_path.substr(0, final_ds);
  file_name = file_path.substr(final_ds + 1);
 }

 fn_final_dot = file_name.find_last_of('.');

 if(fn_final_dot != std::string::npos)
 {
  file_base = file_name.substr(0, fn_final_dot);
  file_ext = file_name.substr(fn_final_dot);
 }
 else
 {
  file_base = file_name;
  file_ext = "";
 }

 // Write outputs at end, in case file_path references the same std::string as pointed to
 // by one of the outputs.
 if(dir_path_out)
  *dir_path_out = dir_path;

 if(file_base_out)
  *file_base_out = file_base;

 if(file_ext_out)
  *file_ext_out = file_ext;
}

}


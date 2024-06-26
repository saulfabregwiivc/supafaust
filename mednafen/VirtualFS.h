/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* VirtualFS.h:
**  Copyright (C) 2010-2018 Mednafen Team
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

#ifndef __MDFN_VIRTUALFS_H
#define __MDFN_VIRTUALFS_H

#include "Stream.h"
#include <functional>

#include <mednafen/string/string.h>

namespace Mednafen
{

class VirtualFS
{
 public:

 VirtualFS(char preferred_path_separator_, const std::string& allowed_path_separators_);
 virtual ~VirtualFS();

 enum : uint32
 {
  MODE_READ = 0x0D46323C,

  // Custom modes for derived classes after this.
  MODE__CUSTOM_BEGIN = 0xF0000000,
 };

 protected:

 // Probably not necessary, but virtual functions make me a little uneasy. ;)
 enum class CanaryType : uint64
 {
  open = 0xA8D4C7D433EC0BC9ULL
 };

 public:
 // If throw_on_noent is true, will always return a non-null pointer or throw.
 // Otherwise, will return nullptr if the file doesn't exist/wasn't found.
 virtual Stream* open(const std::string& path, const uint32 mode, const bool throw_on_noent = true, const CanaryType canary = CanaryType::open) = 0;

 struct FileInfo
 {
  INLINE FileInfo() : size(0), mtime_us(0), is_regular(false), is_directory(false) { }

  uint64 size;			// In bytes.
  int64 mtime_us;		// Last modification time, in microseconds(since the "Epoch").
  bool is_regular : 1;		// Is regular file.
  bool is_directory : 1;	// Is directory.
 };

 virtual void readdirentries(const std::string& path, std::function<bool(const std::string&)> callb) = 0;

 virtual std::string get_human_path(const std::string& path) = 0;
 virtual void get_file_path_components(const std::string& file_path, std::string* dir_path_out, std::string* file_base_out = nullptr, std::string *file_ext_out = nullptr);
 //
 //
 //
 INLINE char get_preferred_path_separator(void) { return preferred_path_separator; }

 // 'ext' must have a leading dot(.)
 INLINE bool test_ext(const std::string& path, const char* ext) const
 {
  size_t ext_len = strlen(ext);

  return (path.size() >= ext_len) && !MDFN_memazicmp(path.c_str() + (path.size() - ext_len), ext, ext_len);
 }

 protected:
 
 const char preferred_path_separator;
 const std::string allowed_path_separators;
};

}
#endif

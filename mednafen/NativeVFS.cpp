/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* NativeVFS.cpp:
**  Copyright (C) 2018-2019 Mednafen Team
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
#include <mednafen/NativeVFS.h>
#include <mednafen/FileStream.h>

#ifdef WIN32
 #include <mednafen/win32-common.h>
#else
 #include <unistd.h>
 #include <dirent.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

namespace Mednafen
{

NativeVFS::NativeVFS() : VirtualFS(MDFN_PS, (PSS_STYLE == 2) ? "\\/" : PSS)
{


}

NativeVFS::~NativeVFS()
{


}

Stream* NativeVFS::open(const std::string& path, const uint32 mode, const bool throw_on_noent, const CanaryType canary)
{
 if(canary != CanaryType::open)
  _exit(-1);

 try
 {
  return new FileStream(path, mode);
 }
 catch(MDFN_Error& e)
 {
  if(e.GetErrno() != ENOENT || throw_on_noent)
   throw;

  return nullptr;
 }
}

void NativeVFS::readdirentries(const std::string& path, std::function<bool(const std::string&)> callb)
{
 if(path.find('\0') != std::string::npos)
  throw MDFN_Error(EINVAL, _("Error reading directory entries from \"%s\": %s"), path.c_str(), _("Null character in path."));
 //
 //
#ifdef WIN32
 //
 // TODO: drive-relative?  probably would need to change how we represent and compose paths in Mednafen...
 //
 HANDLE dp = nullptr;
 bool invalid_utf8;
 std::u16string u16path = UTF8_to_UTF16(path + preferred_path_separator + '*', &invalid_utf8, true);
 WIN32_FIND_DATAW ded;

 if(invalid_utf8)
  throw MDFN_Error(EINVAL, _("Error reading directory entries from \"%s\": %s"), path.c_str(), _("Invalid UTF-8"));

 try
 {
  if(!(dp = FindFirstFileW((const wchar_t*)u16path.c_str(), &ded)))
  {
   const uint32 ec = GetLastError();

   throw MDFN_Error(0, _("Error reading directory entries from \"%s\": %s"), path.c_str(), Win32Common::ErrCodeToString(ec).c_str());
  }

  for(;;)
  {
   //printf("%s\n", UTF16_to_UTF8((const char16_t*)ded.cFileName, nullptr, true).c_str());
   if(!callb(UTF16_to_UTF8((const char16_t*)ded.cFileName, nullptr, true)))
    break;
   //
   if(!FindNextFileW(dp, &ded))
   {
    const uint32 ec = GetLastError();

    if(ec == ERROR_NO_MORE_FILES)
     break;
    else
     throw MDFN_Error(0, _("Error reading directory entries from \"%s\": %s"), path.c_str(), Win32Common::ErrCodeToString(ec).c_str());
   }
  }
  //
  FindClose(dp);
  dp = nullptr;
 }
 catch(...)
 {
  if(dp)
  {
   FindClose(dp);
   dp = nullptr;
  }
  throw;
 }
#else
 DIR* dp = nullptr;
 std::string fname;

 fname.reserve(512);

 try
 {
  if(!(dp = opendir(path.c_str())))
  {
   ErrnoHolder ene(errno);

   throw MDFN_Error(ene.Errno(), _("Error reading directory entries from \"%s\": %s"), path.c_str(), ene.StrError());
  }
  //
  for(;;)
  {
   struct dirent* de;

   errno = 0;
   if(!(de = readdir(dp)))
   {
    if(errno)
    {
     ErrnoHolder ene(errno);

     throw MDFN_Error(ene.Errno(), _("Error reading directory entries from \"%s\": %s"), path.c_str(), ene.StrError());     
    }    
    break;
   }
   //
   fname.clear();
   fname += de->d_name;
   //
   if(!callb(fname))
    break;
  }
  //
  closedir(dp);
  dp = nullptr;
 }
 catch(...)
 {
  if(dp)
  {
   closedir(dp);
   dp = nullptr;
  }
  throw;
 }
#endif
}


}

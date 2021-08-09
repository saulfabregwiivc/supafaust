/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* FileStream.h:
**  Copyright (C) 2010-2016 Mednafen Team
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

#ifndef __MDFN_FILESTREAM_H
#define __MDFN_FILESTREAM_H

#include "Stream.h"
#include "VirtualFS.h"

namespace Mednafen
{

class FileStream : public Stream
{
 public:

 // Convenience function so we don't need so many try { } catch { } for ENOENT
 static INLINE FileStream* open(const std::string& path, const uint32 mode)
 {
   return new FileStream(path, mode);
 }

 enum
 {
  MODE_READ = VirtualFS::MODE_READ,
 };

 FileStream(const std::string& path, const uint32 mode);
 virtual ~FileStream();

 virtual uint64 attributes(void);

 virtual uint8 *map(void);
 virtual uint64 map_size(void);
 virtual void unmap(void);

 virtual uint64 read(void *data, uint64 count, bool error_on_eos = true);
 virtual void write(const void *data, uint64 count);
 virtual void seek(int64 offset, int whence);
 virtual uint64 tell(void);
 virtual uint64 size(void);
 virtual void flush(void);
 virtual void close(void);

 virtual int get_line(std::string &str);

 INLINE int get_char(void)
 {
  int ret;

  if(MDFN_UNLIKELY(prev_was_write == 1))
   seek(0, SEEK_CUR);

  errno = 0;
  ret = fgetc(fp);

  if(MDFN_UNLIKELY(errno != 0))
  {
   ErrnoHolder ene(errno);
   throw(MDFN_Error(ene.Errno(), _("Error reading from opened file \"%s\": %s"), path_save.c_str(), ene.StrError()));
  }
  return(ret);
 }

 private:
 FileStream & operator=(const FileStream &);    // Assignment operator
 FileStream(const FileStream &);		// Copy constructor
 //FileStream(FileStream &);                // Copy constructor

 FILE *fp;
 std::string path_save;
 const uint32 OpenedMode;

 void* mapping;
 uint64 mapping_size;

 int prev_was_write;	// -1 for no state, 0 for last op was read
};

}
#endif

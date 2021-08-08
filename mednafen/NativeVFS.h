/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* NativeVFS.h:
**  Copyright (C) 2018 Mednafen Team
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

#ifndef __MDFN_NATIVEVFS_H
#define __MDFN_NATIVEVFS_H

namespace Mednafen
{

class NativeVFS final : public VirtualFS
{
 public:

 NativeVFS();
 virtual ~NativeVFS() override;

 virtual Stream* open(const std::string& path, const uint32 mode, const bool throw_on_noent = true, const CanaryType canary = CanaryType::open) override;
 virtual void readdirentries(const std::string& path, std::function<bool(const std::string&)> callb) override;

};

}
#endif

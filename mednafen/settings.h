/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* settings.h:
**  Copyright (C) 2005-2021 Mednafen Team
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

#ifndef __MDFN_SETTINGS_H
#define __MDFN_SETTINGS_H

#include "settings-common.h"

namespace Mednafen
{
class SettingsManager
{
 public:

 SettingsManager() MDFN_COLD;
 ~SettingsManager() MDFN_COLD;

 void Add(const MDFNSetting&);
 void Merge(const MDFNSetting *);

 void Finalize(void);

 void Kill();	// Free any resources acquired.
 int64 GetI(const char *name);
 uint64 GetUI(const char *name);
 double GetF(const char *name);
 bool GetB(const char *name);
 std::string GetS(const char *name);

 std::vector<uint64> GetMultiUI(const char *name);
 std::vector<int64> GetMultiI(const char *name);

 bool Set(const char *name, const char *value, bool NetplayOverride = false);
 bool SetB(const char *name, bool value);
 bool SetUI(const char *name, uint64 value);
 bool SetI(const char *name, int64 value);

 const std::vector<MDFNCS>* GetSettings(void);

 private:
 //void ValidateSetting(const char *value, const MDFNSetting *setting)
 MDFNCS *FindSetting(const char* name, bool dont_freak_out_on_fail = false);
 INLINE void MergeSettingSub(const MDFNSetting& setting);

 std::vector<MDFNCS> CurrentSettings;

 bool SettingsFinalized = false;

 std::vector<char*> UnknownSettings;
};

}
#endif

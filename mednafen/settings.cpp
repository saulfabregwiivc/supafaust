/******************************************************************************/
/* Mednafen - Multi-system Emulator                                           */
/******************************************************************************/
/* settings.cpp:
**  Copyright (C) 2005-2018 Mednafen Team
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

/*
 TODO: Setting changed callback on override setting loading/clearing.
*/

#include "mednafen.h"
#include <locale.h>
#include <map>
#include "settings.h"
#include "string/string.h"

namespace Mednafen
{

static bool SettingsFinalized = false;

static std::vector<MDFNCS> CurrentSettings;

static MDFNCS *FindSetting(const char *name, bool dont_freak_out_on_fail = false);


static bool TranslateSettingValueUI(const char *value, unsigned long long &tlated_value)
{
 char *endptr = NULL;

 if(value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
  tlated_value = strtoull(value + 2, &endptr, 16);
 else
  tlated_value = strtoull(value, &endptr, 10);

 if(!endptr || *endptr != 0)
 {
  return(false);
 }
 return(true);
}

static bool TranslateSettingValueI(const char *value, long long &tlated_value)
{
 char *endptr = NULL;

 if(value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
  tlated_value = strtoll(value + 2, &endptr, 16);
 else
  tlated_value = strtoll(value, &endptr, 10);

 if(!endptr || *endptr != 0)
 {
  return(false);
 }
 return(true);
}

//
// This function is a ticking time bomb of (semi-non-reentrant) wrong, but it's OUR ticking time bomb.
//
// Note to self: test it with something like: LANG="fr_FR.UTF-8" ./mednafen
//
static bool MR_StringToDouble(const char* string_value, double* dvalue) NO_INLINE;	// noinline for *potential* x87 FPU extra precision weirdness in regards to optimizations.
static bool MR_StringToDouble(const char* string_value, double* dvalue)
{
 static char MR_Radix = 0;
 const unsigned slen = strlen(string_value);
 char cpi_array[256 + 1];
 std::unique_ptr<char[]> cpi_heap;
 char* cpi = cpi_array;
 char* endptr = NULL;

 if(slen > 256)
 {
  cpi_heap.reset(new char[slen + 1]);
  cpi = cpi_heap.get();
 }

 if(!MR_Radix)
 {
  char buf[64]; // Use extra-large buffer since we're using sprintf() instead of snprintf() for portability reasons. //4];
  // Use libc snprintf() and not snprintf() here for out little abomination.
  //snprintf(buf, 4, "%.1f", (double)1);
  sprintf(buf, "%.1f", (double)1);
  if(buf[0] == '1' && buf[2] == '0' && buf[3] == 0)
  {
   MR_Radix = buf[1];
  }
  else
  {
   lconv* l = localeconv();
   assert(l != NULL);
   MR_Radix = *(l->decimal_point);
  }
 }

 for(unsigned i = 0; i < slen; i++)
 {
  char c = string_value[i];

  if(c == '.' || c == ',')
   c = MR_Radix;

  cpi[i] = c;
 }
 cpi[slen] = 0;

 *dvalue = strtod(cpi, &endptr);

 if(endptr == NULL || *endptr != 0)
  return(false);

 return(true);
}

static void ValidateSetting(const char *value, const MDFNSetting *setting)
{
 MDFNSettingType base_type = setting->type;

 if(base_type == MDFNST_UINT)
 {
  unsigned long long ullvalue;

  if(!TranslateSettingValueUI(value, ullvalue))
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\", is not set to a valid unsigned integer."), setting->name, value);
  }
  if(setting->minimum)
  {
   unsigned long long minimum;

   TranslateSettingValueUI(setting->minimum, minimum);
   if(ullvalue < minimum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
   }
  }
  if(setting->maximum)
  {
   unsigned long long maximum;

   TranslateSettingValueUI(setting->maximum, maximum);
   if(ullvalue > maximum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
   }
  }
 }
 else if(base_type == MDFNST_INT)
 {
  long long llvalue;

  if(!TranslateSettingValueI(value, llvalue))
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\", is not set to a valid signed integer."), setting->name, value);
  }
  if(setting->minimum)
  {
   long long minimum;

   TranslateSettingValueI(setting->minimum, minimum);
   if(llvalue < minimum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
   }
  }
  if(setting->maximum)
  {
   long long maximum;

   TranslateSettingValueI(setting->maximum, maximum);
   if(llvalue > maximum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
   }
  }
 }
 else if(base_type == MDFNST_FLOAT)
 {
  double dvalue;

  if(!MR_StringToDouble(value, &dvalue))
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\", is not set to a floating-point(real) number."), setting->name, value);
  }
  if(setting->minimum)
  {
   double minimum;

   if(MDFN_UNLIKELY(!MR_StringToDouble(setting->minimum, &minimum)))
    throw MDFN_Error(0, _("Minimum value, \"%f\", for setting \"%s\" is not set to a floating-point(real) number."), minimum, setting->name);

   if(MDFN_UNLIKELY(dvalue < minimum))
    throw MDFN_Error(0, _("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
  }
  if(setting->maximum)
  {
   double maximum;

   if(MDFN_UNLIKELY(!MR_StringToDouble(setting->maximum, &maximum)))
    throw MDFN_Error(0, _("Maximum value, \"%f\", for setting \"%s\" is not set to a floating-point(real) number."), maximum, setting->name);

   if(MDFN_UNLIKELY(dvalue > maximum))
    throw MDFN_Error(0, _("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
  }
 }
 else if(base_type == MDFNST_BOOL)
 {
  if(strlen(value) != 1 || (value[0] != '0' && value[0] != '1'))
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\",  is not a valid boolean value."), setting->name, value);
  }
 }
 else if(base_type == MDFNST_ENUM)
 {
  const MDFNSetting_EnumList *enum_list = setting->enum_list;
  std::string valid_string_list;

  assert(enum_list);

  while(enum_list->string)
  {
   if(!MDFN_strazicmp(value, enum_list->string))
    break;

   if(enum_list->description)	// Don't list out undocumented and deprecated values.
    valid_string_list = valid_string_list + (enum_list == setting->enum_list ? "" : " ") + std::string(enum_list->string);

   enum_list++;
  }
 }
 else if(base_type == MDFNST_MULTI_ENUM)
 {
  std::vector<std::string> mel = MDFN_strsplit(value);

  assert(setting->enum_list);

  for(auto& mee : mel)
  {
   bool found = false;
   const MDFNSetting_EnumList* enum_list = setting->enum_list;

   MDFN_trim(&mee);

   while(enum_list->string)
   {
    if(!MDFN_strazicmp(mee.c_str(), enum_list->string))
    {
     found = true;
     break;
    }
    enum_list++;
   }

   if(!found)
   {
    std::string valid_string_list;

    enum_list = setting->enum_list;
    while(enum_list->string)
    {
     if(enum_list->description)	// Don't list out undocumented and deprecated values.
      valid_string_list = valid_string_list + (enum_list == setting->enum_list ? "" : " ") + std::string(enum_list->string);

     enum_list++;
    }
    throw MDFN_Error(0, _("Setting \"%s\", value \"%s\" component \"%s\", is not a recognized string.  Recognized strings: %s"), setting->name, value, mee.c_str(), valid_string_list.c_str());
   }
  }
 }

 if(setting->validate_func && !setting->validate_func(setting->name, value))
 {
  if(base_type == MDFNST_STRING)
   throw MDFN_Error(0, _("Setting \"%s\" is not set to a valid string: \"%s\""), setting->name, value);
  else
   throw MDFN_Error(0, _("Setting \"%s\" is not set to a valid unsigned integer: \"%s\""), setting->name, value);
 }
}

static uint32 MakeNameHash(const char *name)
{
#if 0
 return crc32(0, (const Bytef *)name, strlen(name));
#else
 uint32 ret = 0;

 for(size_t i = 0; name[i]; i++)
  ret = (ret * 1103515245) + 12345 + (uint8)name[i];

 //printf("%s %08x\n", name, ret);

 return ret;
#endif
}

static INLINE void MergeSettingSub(const MDFNSetting *setting)
{
 MDFNCS TempSetting;

 assert(setting->name);
 assert(setting->default_value);

 TempSetting.name = strdup(setting->name);
 TempSetting.value = strdup(setting->default_value);
 TempSetting.name_hash = MakeNameHash(setting->name);
 TempSetting.desc = setting;
 TempSetting.ChangeNotification = setting->ChangeNotification;

 CurrentSettings.push_back(TempSetting);
}

void MDFN_MergeSettings(const MDFNSetting *setting)
{
 while(setting->name != NULL)
 {
  MergeSettingSub(setting);
  setting++;
 }
}

void MDFN_MergeSettings(const std::vector<MDFNSetting> &setting)
{
 assert(!SettingsFinalized);

 for(unsigned int x = 0; x < setting.size(); x++)
  MergeSettingSub(&setting[x]);
}

static bool CSHashSortFunc(const MDFNCS& a, const MDFNCS& b)
{
 return a.name_hash < b.name_hash;
}

static bool CSHashBoundFunc(const MDFNCS& a, const uint32 b)
{
 return a.name_hash < b;
}

void MDFN_FinalizeSettings(void)
{
 std::sort(CurrentSettings.begin(), CurrentSettings.end(), CSHashSortFunc);

 //
 // Ensure no duplicates.
 //
 for(size_t i = 0; i < CurrentSettings.size(); i++)
 {
  for(size_t j = i + 1; j < CurrentSettings.size() && CurrentSettings[j].name_hash == CurrentSettings[i].name_hash; j++)
  {
   if(!strcmp(CurrentSettings[i].name, CurrentSettings[j].name))
   {
    printf("Duplicate setting name %s\n", CurrentSettings[j].name);
    abort();
   }
  }
 }

 SettingsFinalized = true;
/*
 for(size_t i = 0; i < CurrentSettings.size(); i++)
 {
  assert(CurrentSettings[i].desc->type == MDFNST_ALIAS || !strcmp(FindSetting(CurrentSettings[i].name)->name, CurrentSettings[i].name));
 }
*/
}

void MDFN_KillSettings(void)
{
 for(auto& sit : CurrentSettings)
 {
  if(sit.desc->type == MDFNST_ALIAS)
   continue;

  free(sit.name);
  free(sit.value);
 }

 CurrentSettings.clear();	// Call after the list is all handled
 SettingsFinalized = false;
}

static MDFNCS* FindSetting(const char* name, bool dont_freak_out_on_fail)
{
 assert(SettingsFinalized);
 //printf("Find: %s\n", name);
 const uint32 name_hash = MakeNameHash(name);
 std::vector<MDFNCS>::iterator it;

 it = std::lower_bound(CurrentSettings.begin(), CurrentSettings.end(), name_hash, CSHashBoundFunc);

 while(it != CurrentSettings.end() && it->name_hash == name_hash)
 {
  if(!strcmp(it->name, name))
  {
   if(it->desc->type == MDFNST_ALIAS)
    return FindSetting(it->value, dont_freak_out_on_fail);

   return &*it;
  }
  //printf("OHNOS: %s(%08x) %s(%08x)\n", name, name_hash, it->name, it->name_hash);
  it++;
 }

 if(!dont_freak_out_on_fail)
 {
  printf("\n\nINCONCEIVABLE!  Setting not found: %s\n\n", name);
  exit(1);
 }

 return NULL;
}

static const char *GetSetting(const MDFNCS *setting)
{
 return setting->value;
}

static int GetEnum(const MDFNCS *setting, const char *value)
{
 const MDFNSetting_EnumList *enum_list = setting->desc->enum_list;
 int ret = 0;

 assert(enum_list);

 while(enum_list->string)
 {
  if(!MDFN_strazicmp(value, enum_list->string))
  {
   ret = enum_list->number;
   break;
  }
  enum_list++;
 }

 return(ret);
}

template<typename T>
static std::vector<T> GetMultiEnum(const MDFNCS* setting, const char* value)
{
 std::vector<T> ret;
 std::vector<std::string> mel = MDFN_strsplit(value);

 assert(setting->desc->enum_list);

 for(auto& mee : mel)
 {
  const MDFNSetting_EnumList *enum_list = setting->desc->enum_list;

  MDFN_trim(&mee);

  while(enum_list->string)
  {
   if(!MDFN_strazicmp(mee.c_str(), enum_list->string))
   {
    ret.push_back(enum_list->number);
    break;
   }
   enum_list++;
  }
 }

 return ret;
}


uint64 MDFN_GetSettingUI(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(setting);

 if(setting->desc->type == MDFNST_ENUM)
  return(GetEnum(setting, value));
 else
 {
  unsigned long long ret;
  TranslateSettingValueUI(value, ret);
  return(ret);
 }
}

int64 MDFN_GetSettingI(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(FindSetting(name));


 if(setting->desc->type == MDFNST_ENUM)
  return(GetEnum(setting, value));
 else
 {
  long long ret;
  TranslateSettingValueI(value, ret);
  return(ret);
 }
}

std::vector<uint64> MDFN_GetSettingMultiUI(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(setting);

 if(setting->desc->type == MDFNST_MULTI_ENUM)
  return GetMultiEnum<uint64>(setting, value);
 else
  abort();
}

std::vector<int64> MDFN_GetSettingMultiI(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(setting);

 if(setting->desc->type == MDFNST_MULTI_ENUM)
  return GetMultiEnum<int64>(setting, value);
 else
  abort();
}


double MDFN_GetSettingF(const char *name)
{
 double ret;

 MR_StringToDouble(GetSetting(FindSetting(name)), &ret);

 return ret;
}

bool MDFN_GetSettingB(const char *name)
{
 return((bool)MDFN_GetSettingUI(name));
}

std::string MDFN_GetSettingS(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(setting);

 // Even if we're getting the string value of an enum instead of the associated numeric value, we still need
 // to make sure it's a valid enum
 // (actually, not really, since it's handled in other places where the setting is actually set)
 //if(setting->desc->type == MDFNST_ENUM)
 // GetEnum(setting, value);

 return(std::string(value));
}

void MDFNI_SetSetting(const char *name, const char *value)
{
 MDFNCS *zesetting = FindSetting(name, true);

 if(zesetting)
 {
  ValidateSetting(value, zesetting->desc);

  if(zesetting->value)
   free(zesetting->value);
  zesetting->value = strdup(value);

  // TODO, always call driver notification function, regardless of whether a game is loaded.
  if(zesetting->ChangeNotification)
  {
   if(MDFNGameInfo)
    zesetting->ChangeNotification(name);
  }
 }
 else
  throw MDFN_Error(0, _("Unknown setting \"%s\""), name);
}

#if 0
// TODO after a game is loaded, but should we?
void MDFN_CallSettingsNotification(void)
{
 for(unsigned int x = 0; x < CurrentSettings.size(); x++)
 {
  if(CurrentSettings[x].ChangeNotification)
  {
   // TODO, always call driver notification function, regardless of whether a game is loaded.
   if(MDFNGameInfo)
    CurrentSettings[x].ChangeNotification(CurrentSettings[x].name);
  }
 }
}
#endif

}

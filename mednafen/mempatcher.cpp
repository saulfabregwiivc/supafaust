/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mednafen.h"

#include <mednafen/string/string.h>
#include "mempatcher.h"

namespace Mednafen
{

static std::vector<MemoryPatch> Cheats;

MemoryPatch::MemoryPatch() : addr(0), value(0), compare(0), 
			     mltpl_count(1), mltpl_addr_inc(0), mltpl_val_inc(0), copy_src_addr(0), copy_src_addr_inc(0),
			     length(0), bigendian(false), icount(0), type(0)
{

}

MemoryPatch::~MemoryPatch()
{

}

static INLINE uint8 ReadU8(uint32 addr)
{
 //addr %= (uint64)PageSize * NumPages;
 return MDFNGameInfo->CheatInfo.MemRead(addr);
}

static INLINE void WriteU8(uint32 addr, const uint8 val)
{
 //addr %= (uint64)PageSize * NumPages;
 MDFNGameInfo->CheatInfo.MemWrite(addr, val);
}

void MDFNI_AddCheat(const MemoryPatch& patch)
{
 if(patch.type == 'S' || patch.type == 'C')
 {
  if(MDFNGameInfo->CheatInfo.InstallReadPatch)
  {
   for(unsigned int x = 0; x < patch.length; x++)
   {
    uint32 address;
    uint8 value;
    int compare;
    unsigned int shiftie;

    if(patch.bigendian)
     shiftie = (patch.length - 1 - x) * 8;
    else
     shiftie = x * 8;
    
    address = patch.addr + x;
    value = (patch.value >> shiftie) & 0xFF;
    if(patch.type == 'C')
     compare = (patch.compare >> shiftie) & 0xFF;
    else
     compare = -1;

    MDFNGameInfo->CheatInfo.InstallReadPatch(address, value, compare);
   }
  }
 }
 else
  Cheats.push_back(patch);
}

void MDFNI_DelCheats(void)
{
 Cheats.clear();
 if(MDFNGameInfo->CheatInfo.RemoveReadPatches)
  MDFNGameInfo->CheatInfo.RemoveReadPatches();
}

/*
 Condition format(ws = white space):
 
  <variable size><ws><endian><ws><address><ws><operation><ws><value>
	  [,second condition...etc.]

  Value should be unsigned integer, hex(with a 0x prefix) or
  base-10.  

  Operations:
   >=
   <=
   >
   <
   ==
   !=
   &	// Result of AND between two values is nonzero
   !&   // Result of AND between two values is zero
   ^    // same, XOR
   !^
   |	// same, OR
   !|

  Full example:

  2 L 0xADDE == 0xDEAD, 1 L 0xC000 == 0xA0

*/

static bool TestConditions(const char *string)
{
 char address[64];
 char operation[64];
 char value[64];
 char endian;
 unsigned int bytelen;
 bool passed = 1;

 while(sscanf(string, "%u %c %63s %63s %63s", &bytelen, &endian, address, operation, value) == 5 && passed)
 {
  uint32 v_address;
  uint64 v_value;
  uint64 value_at_address;

  if(address[0] == '0' && address[1] == 'x')
   v_address = strtoul(address + 2, NULL, 16);
  else
   v_address = strtoul(address, NULL, 10);

  if(value[0] == '0' && value[1] == 'x')
   v_value = strtoull(value + 2, NULL, 16);
  else
   v_value = strtoull(value, NULL, 0);

  value_at_address = 0;
  for(unsigned int x = 0; x < bytelen; x++)
  {
   unsigned int shiftie;

   if(endian == 'B')
    shiftie = (bytelen - 1 - x) * 8;
   else
    shiftie = x * 8;

   value_at_address |= (uint64)ReadU8(v_address + x) << shiftie;
  }

  //printf("A: %08x, V: %08llx, VA: %08llx, OP: %s\n", v_address, v_value, value_at_address, operation);
  if(!strcmp(operation, ">="))
  {
   if(!(value_at_address >= v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "<="))
  {
   if(!(value_at_address <= v_value))
    passed = 0;
  }
  else if(!strcmp(operation, ">"))
  {
   if(!(value_at_address > v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "<"))
  {
   if(!(value_at_address < v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "==")) 
  {
   if(!(value_at_address == v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!="))
  {
   if(!(value_at_address != v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "&"))
  {
   if(!(value_at_address & v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!&"))
  {
   if(value_at_address & v_value)
    passed = 0;
  }
  else if(!strcmp(operation, "^"))
  {
   if(!(value_at_address ^ v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!^"))
  {
   if(value_at_address ^ v_value)
    passed = 0;
  }
  else if(!strcmp(operation, "|"))
  {
   if(!(value_at_address | v_value))
    passed = 0;
  }
  else if(!strcmp(operation, "!|"))
  {
   if(value_at_address | v_value)
    passed = 0;
  }
  else
   puts("Invalid operation");
  string = strchr(string, ',');
  if(string == NULL)
   break;
  else
   string++;
  //printf("Foo: %s\n", string);
 }

 return passed;
}

void MDFNMP_ApplyPeriodicCheats(void)
{
 //TestConditions("2 L 0x1F00F5 == 0xDEAD");
 //if(TestConditions("1 L 0x1F0058 > 0")) //, 1 L 0xC000 == 0x01"));
 for(auto const& ch : Cheats)
 {
  if(ch.type == 'R' || ch.type == 'A' || ch.type == 'T')
  {
   if(ch.conditions.size() == 0 || TestConditions(ch.conditions.c_str()))
   {
    uint32 mltpl_count = ch.mltpl_count;
    uint32 mltpl_addr = ch.addr;
    uint64 mltpl_val = ch.value;
    uint32 copy_src_addr = ch.copy_src_addr;

    while(mltpl_count--)
    {
     uint8 carry = 0;

     for(unsigned int x = 0; x < ch.length; x++)
     {
      const uint32 tmpaddr = ch.bigendian ? (mltpl_addr + ch.length - 1 - x) : (mltpl_addr + x);
      const uint8 tmpval = mltpl_val >> (x * 8);

      if(ch.type == 'A')
      {
       const unsigned t = ReadU8(tmpaddr) + tmpval + carry;

       carry = t >> 8;

       WriteU8(tmpaddr, t);
      }
      else if(ch.type == 'T')
      {
       const uint8 cv = ReadU8(ch.bigendian ? (copy_src_addr + ch.length - 1 - x) : (copy_src_addr + x));

       WriteU8(tmpaddr, cv);
      }
      else
       WriteU8(tmpaddr, tmpval);
     }
     mltpl_addr += ch.mltpl_addr_inc;
     mltpl_val += ch.mltpl_val_inc;
     copy_src_addr += ch.copy_src_addr_inc;
    }
   } // end if(ch.conditions.size() == 0 || TestConditions(ch.conditions.c_str()))
  }
 }
}



}

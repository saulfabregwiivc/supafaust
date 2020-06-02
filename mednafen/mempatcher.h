#ifndef __MDFN_MEMPATCHER_H
#define __MDFN_MEMPATCHER_H

namespace Mednafen
{

struct MemoryPatch
{
 MemoryPatch();
 ~MemoryPatch();

 std::string conditions;

 uint32 addr;
 uint64 value;
 uint64 compare;

 uint32 mltpl_count;
 uint32 mltpl_addr_inc;
 uint64 mltpl_val_inc;

 uint32 copy_src_addr;
 uint32 copy_src_addr_inc;

 unsigned length;
 bool bigendian;
 unsigned icount;

 char type; /* 'R' for replace, 'S' for substitute(GG), 'C' for substitute with compare */
	    /* 'T' for copy/transfer data, 'A' for add(variant of type R) */
};

INLINE void MDFNMP_Init(uint32 ps, uint32 numpages) { }
INLINE void MDFNMP_AddRAM(uint32 size, uint32 address, uint8 *RAM, bool use_in_search = true) { }
INLINE void MDFNMP_RegSearchable(uint32 addr, uint32 size) { }
INLINE void MDFNMP_Kill(void) { }

void MDFNMP_ApplyPeriodicCheats(void);

}
#endif

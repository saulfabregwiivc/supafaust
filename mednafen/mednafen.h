#ifndef __MDFN_MEDNAFEN_H
#define __MDFN_MEDNAFEN_H

#include "types.h"
#include "memory.h"
#include "video.h"
#include "state.h"
#include "git.h"
#include "settings.h"
#include "NativeVFS.h"
#include "MemoryStream.h"
#include "string/string.h"
#include "mempatcher.h"

namespace Mednafen
{

extern NativeVFS NVFS;
MDFN_HIDE extern MDFNGI *MDFNGameInfo;

enum MDFN_NoticeType : uint8
{
 // Terse status message for a user-initiated action during runtime(state loaded, screenshot saved, etc.).
 MDFN_NOTICE_STATUS = 0,

 // Something went slightly wrong, but we can mostly recover from it, but the user should still know because
 // it may cause behavior to differ from desired.
 MDFN_NOTICE_WARNING,

 // Something went horribly wrong(user-triggered action, or otherwise); generally prefer throwing MDFN_Error()
 // over sending this, where possible/applicable.
 MDFN_NOTICE_ERROR
};

void MDFN_Notify(MDFN_NoticeType t, const char* format, ...) MDFN_FORMATSTR(gnu_printf, 2, 3);

// Verbose status and informational messages, primarily during startup and exit.
void MDFN_printf(const char *format, ...) MDFN_FORMATSTR(gnu_printf, 1, 2);

void MDFN_StateAction(StateMem *sm, const unsigned load, const bool data_only);

MDFN_HIDE extern std::vector<MDFNGI *>MDFNSystems;

/* Indent stdout newlines +- "indent" amount */
void MDFN_indent(int indent);
struct MDFN_AutoIndent
{
 INLINE MDFN_AutoIndent() : indented(0) { }
 INLINE MDFN_AutoIndent(int amount) : indented(amount) { MDFN_indent(indented); }
 INLINE ~MDFN_AutoIndent() { MDFN_indent(-indented); }

 //INLINE void indent(int indoot) { indented += indoot; MDFN_indent(indoot); }
 private:
 int indented;
};
void MDFN_printf(const char *format, ...) MDFN_FORMATSTR(gnu_printf, 1, 2);

#define MDFNI_printf MDFN_printf

// MDFN_NOTICE_ERROR may block(e.g. for user confirmation), other notice types should be as non-blocking as possible.
void MDFND_OutputNotice(MDFN_NoticeType t, const char* s);

// Output from MDFN_printf(); fairly verbose informational messages.
void MDFND_OutputInfo(const char* s);

// Synchronize virtual time to actual time using members of espec:
//
//  MasterCycles and MasterCyclesALMS (coupled with MasterClock of MDFNGI)
//   and/or
//  SoundBuf, SoundBufSize, and SoundBufSizeALMS
//
// ...and after synchronization, update the data pointed to by the pointers passed to MDFNI_SetInput().
// DO NOT CALL MDFN_* or MDFNI_* functions from within MDFND_MidSync().
// Calling MDFN_printf(), MDFN_DispMessage(),and MDFND_PrintError() are ok, though.
//
// If you do not understand how to implement this function, you can leave it empty at first, but know that doing so
// will subtly break at least one PC Engine game(Takeda Shingen), and raise input latency on some other PC Engine games.
enum : unsigned
{
 MIDSYNC_FLAG_NONE		= 0,
 MIDSYNC_FLAG_UPDATE_INPUT	= 1U << 0,
 MIDSYNC_FLAG_SYNC_TIME		= 1U << 1,
};
void MDFN_MidSync(EmulateSpecStruct *espec, const unsigned flags = MIDSYNC_FLAG_UPDATE_INPUT | MIDSYNC_FLAG_SYNC_TIME);
void MDFND_MidSync(const EmulateSpecStruct *espec, const unsigned flags);

void MDFNI_Reset(void);
void MDFNI_Power(void);

//
MDFNGI* MDFNI_LoadGame(GameFile* gf) MDFN_COLD;

//
void MDFNI_Initialize(void) MDFN_COLD;

/* Emulates a frame. */
void MDFNI_Emulate(EmulateSpecStruct *espec);

/* Closes currently loaded game */
void MDFNI_CloseGame(void) MDFN_COLD;

/* Deallocates all allocated memory.  Call after MDFNI_Emulate() returns. */
void MDFNI_Kill(void) MDFN_COLD;

void MDFNI_SetLayerEnableMask(uint64 mask);

uint8* MDFNI_SetInput(const uint32 port, const uint32 type);

void MDFNI_SaveState(Stream* s);
void MDFNI_LoadState(Stream* s);

void MDFNI_AddCheat(const MemoryPatch& patch);
void MDFNI_DelCheats(void);

}

#endif

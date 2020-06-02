#define MDFN_DISABLE_PICPIE_ERRWARN 1
#define MDFN_SNES_FAUST_SUPAFAUST 1
#define MDFN_SNES_FAUST_SPC700_IPL_HLE 1
#define MDFN_SNES_FAUST_SKETCHYSPC700OPT 1
//#define MDFN_SNES_FAUST_SKETCHYPPUOPT 1
#define HAVE_SEM_TIMEDWAIT 1
#define PTHREAD_AFFINITY_NP cpu_set_t

#define LSB_FIRST 1
// #define MSB_FIRST 1

#define MEDNAFEN_VERSION_NUMERIC 0x00102400

#define WANT_SNES_FAUST_EMU 1

#if defined(_MSC_VER) || defined(_WIN32)
 #define WIN32 1
 #define PSS_STYLE 2
 #define PSS "\\"
 #define MDFN_PS '\\'
#else
 #define PSS_STYLE 1
 #define PSS "/"
 #define MDFN_PS '/'
 #define HAVE_MKDIR 1
#endif


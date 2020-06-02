
#define trio_vsnprintf vsnprintf
#define trio_sprintf sprintf
#define trio_snprintf snprintf
#define trio_vasprintf vasprintf

static inline char* trio_vaprintf(const char* format, va_list ap)
{
 char* ret = nullptr;

 if(vasprintf(&ret, format, ap) == -1)
 {
  //errno = ENOMEM;
  return nullptr;
 }

 return ret;
}

#ifndef HAVE_SNPRINTF
#include <stdarg.h>
extern int snprintf(char *buf, int len, const char *str, ...);
#endif

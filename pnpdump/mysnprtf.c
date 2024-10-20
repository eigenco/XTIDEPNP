#define HAVE_SNPRINTF

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_SNPRINTF
#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_SFN
#include <isapnp/callback.h>
#else
#include <isapnp/callbacks.h>
#endif
#include <isapnp/mysnprtf.h>

/* My djgpp libc doesn't have this, so here's a rough hack */
int
snprintf(char *buf, int len, const char *str, ...)
{
	int actlen;
	va_list ap;
	va_start(ap, str);
	actlen = vsprintf(buf, str, ap);
	if(actlen >= len)
	{
		st_fatal_error_callback(ISAPNP_E_SNPRINTF_BUF_OVERRUN);
		/* is not expected to return */
		exit(1);
	}
	va_end(ap);
	return actlen;
}
#endif


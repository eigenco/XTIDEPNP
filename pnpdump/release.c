#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <isapnp/release.h>

#undef RELEASE_AXP
#undef RELEASE_REALTIME
#undef RELEASE_NEEDSCH
#undef RELEASE_NEEDNANO
#undef RELEASE_NEEDABORT
#undef RELEASE_DEBUG
#undef RELEASE_TAGDEBUG
#undef RELEASE_VALIDATE
#undef RELEASE_GONEFILE

#ifdef _AXP_
#define RELEASE_AXP " -D_AXP_"
#else
#define RELEASE_AXP ""
#endif /* !_AXP_ */

#ifdef REALTIME
#define RELEASE_REALTIME " -DREALTIME"
#else
#define RELEASE_REALTIME ""
#endif /* !REALTIME */

#ifdef ENABLE_PCI
#define RELEASE_ENABLE_PCI " -DENABLE_PCI"
#else
#define RELEASE_ENABLE_PCI ""
#endif /* !ENABLE_PCI */

#ifdef HAVE_PROC
#define RELEASE_HAVE_PROC " -DHAVE_PROC"
#else
#define RELEASE_HAVE_PROC ""
#endif /* !HAVE_PROC */

#ifdef HAVE_SCHED_SETSCHEDULER
#define RELEASE_NEEDSCH " -DHAVE_SCHED_SETSCHEDULER"
#else
#define RELEASE_NEEDSCH ""
#endif /* !HAVE_SCHED_SETSCHEDULER */

#ifdef HAVE_NANOSLEEP
#define RELEASE_NEEDNANO " -DHAVE_NANOSLEEP"
#else
#define RELEASE_NEEDNANO ""
#endif /* !HAVE_NANOSLEEP */

#ifdef ABORT_ONRESERR
#define RELEASE_NEEDABORT " -DABORT_ONRESERR"
#else
#define RELEASE_NEEDABORT ""
#endif /* !ABORT_ONRESERR */

#ifdef DEBUG
#define RELEASE_DEBUG " -DDEBUG"
#else
#define RELEASE_DEBUG ""
#endif /* !DEBUG */

#ifdef TAG_DEBUG
#define RELEASE_TAGDEBUG " -DTAG_DEBUG"
#else
#define RELEASE_TAGDEBUG ""
#endif /* !TAG_DEBUG */

#ifdef WANT_TO_VALIDATE
#define RELEASE_VALIDATE " -DWANT_TO_VALIDATE"
#else
#define RELEASE_VALIDATE ""
#endif /* !WANT_TO_VALIDATE */

#ifdef GONEFILE
#define RELEASE_GONEFILE " -DGONEFILE=\"" GONEFILE "\""
#else
#define RELEASE_GONEFILE ""
#endif /* !GONEFILE */

char libtoolsver[1] = "-";
char libcompilerflags[] = RELEASE_AXP RELEASE_REALTIME 
                     RELEASE_HAVE_PROC RELEASE_ENABLE_PCI 
                     RELEASE_NEEDSCH RELEASE_NEEDNANO RELEASE_NEEDABORT 
                     RELEASE_DEBUG RELEASE_TAGDEBUG RELEASE_VALIDATE 
                     RELEASE_GONEFILE;


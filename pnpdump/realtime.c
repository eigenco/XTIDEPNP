/*****************************************************************************
**
** realtime.c
**
** Realtime support code for pnpdump and for code, which uses the pnpdump
** features library (libisapnp.a/libisapnp.so).
**
** The code in this file was extracted from isapnptools-1.19/pnpdump.c
** and is subject to the same copyright and licensing restrictions of
** the above package.  In particular, in spite of any comments below,
** this file is released under the terms of GPL rather than LGPL!
**
** Copyright (C) P.J.H.Fox (fox@roestock.demon.co.uk)
** Modifications by Omer Zak (omerz@actcom.co.il)
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the 
** Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
** Boston, MA  02111-1307  USA.
**
******************************************************************************
**
** Bug reports and fixes - to  P.J.H.Fox (fox@roestock.demon.co.uk)
** Note:  by sending unsolicited commercial/political/religious
**        E-mail messages (known also as "spam") to any E-mail address
**        mentioned in this file, you irrevocably agree to pay the
**        receipient US$500.- (plus any legal expenses incurred while
**        trying to collect the amount due) per unsolicited
**        commercial/political/religious E-mail message - for
**        the service of receiving your E-mail message.
**
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static char st_realtime_version[] __attribute__((unused)) = "$Id: realtime.c,v 0.4 2001/04/30 21:50:29 fox Exp $";

#ifdef REALTIME

#include <isapnp/release.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <isapnp/realtime.h>
#include <isapnp/errenum.h>
#ifdef HAVE_SFN
#include <isapnp/callback.h>
#else
#include <isapnp/callbacks.h>
#endif

static long program_timeout = 5L;

/*
 * Code to set scheduler to Realtime, Round-Robin, so usleeps right etc
 */
#if !defined(HAVE_SCHED_SETSCHEDULER) || !defined(HAVE_NANOSLEEP)
#include <linux/unistd.h>
#endif /* __GLIBC__ */

#ifdef HAVE_SCHED_SETSCHEDULER
#include <sched.h>
#else
#include <linux/sched.h>
_syscall3(int, sched_setscheduler, pid_t, pid, int, policy, struct sched_param *, sched_p)
#endif

#ifdef HAVE_NANOSLEEP
#include <time.h>
#else
#include <sys/time.h>
_syscall2(int, nanosleep, struct timespec *, rqtp, struct timespec *, rmtp)
#endif

#include <signal.h>

/****************************************************************************/

int realtimeok = 0;

/****************************************************************************/

static RETSIGTYPE
abandon(int signo)
{
  st_fatal_error_callback(ISAPNP_E_TIME_EXPIRED);
  /* Should not return */
  exit(1);
}

void
normal_sched(void)
{
  pid_t mypid = getpid();
  struct sched_param sched_p = {0};
  if (sched_setscheduler(mypid, SCHED_OTHER, &sched_p) < 0) {
    /* Should never happen */
    st_non_fatal_error_callback(errno,ISAPNP_E_REALTIME_NOT_CLEARED);
    signal(SIGALRM, abandon);
    alarm(program_timeout);
  }
  else {
    /* Done it, so can kill the timeout */
    alarm(0L);
  }
}

static RETSIGTYPE
realtime_expired(int signo)
{
  pid_t mypid = getpid();
  struct sched_param sched_p = {0};
  if (sched_setscheduler(mypid, SCHED_OTHER, &sched_p) < 0) {
    /* Should never happen */
    st_non_fatal_error_callback(errno,ISAPNP_E_REALTIME_NOT_CLEARED2);
    signal(SIGALRM, abandon);
    alarm(program_timeout);
  }
  else {
    st_non_fatal_error_callback(0,ISAPNP_E_REALTIME_EXCEEDED);
    /* fflush(stdout); */
  }
}

/* Enter realtime mode.
** The arguments are:
** in_realtime_timeout - is a timeout in seconds.
**            If the realtime mode lasts this number of seconds, it is
**            timed out, to prevent the application from getting stuck.
*/
void
setroundrobin(long in_realtime_timeout)
{
  pid_t mypid = getpid();
  struct sched_param sched_p = {50};

  /* Ensure that the error reporting callbacks have already been
  ** initialized.
  */
  if (NULL == st_fatal_error_callback) {
    fprintf(stderr,
	    "Callbacks were not initialized - aborting application\n");
    exit(1);
  }

  /* Use the syscall, as my library not up todate */
  if (sched_setscheduler(mypid, SCHED_RR, &sched_p) < 0) {
    st_non_fatal_error_callback(errno,ISAPNP_E_REALTIME_NOT_SET);
  }
  else {
    realtimeok = 1;
    signal(SIGALRM, realtime_expired);
    alarm(in_realtime_timeout);
  }
}

/*
 * Use nanosleep for realtime delays
 */
void
delaynus(long del)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = del * 1000;
  /*
   * Need to handle case where binary for later kernel run on an earlier
   * one, which doesn't support nanosleep (emergency backup spare !)
   */
  if (realtimeok) {
    if (nanosleep(&t, (struct timespec *) 0) < 0) {
      st_non_fatal_error_callback(errno,ISAPNP_E_REALTIME_NANOSLEEP_FAILED);
      realtimeok = 0;
    }
  }
  else {
    usleep(del);
  }
}



#endif /* REALTIME */
/* End of realtime.c */

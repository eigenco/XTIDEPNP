/*****************************************************************************
**
** realtime.h
** $Header: /free/fox/mystuff/isapnptools-1.26/include/isapnp/RCS/realtime.h,v 0.1 2000/07/25 19:43:34 fox Exp $
**
** Realtime support code for pnpdump and code which uses the pnpdump
** features library (libisapnp.a/libisapnp.so).
**
** Copyright (C) 1999  Omer Zak (omerz@actcom.co.il)
**
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

#ifdef REALTIME

#ifndef _REALTIME_H_
#define _REALTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* Enter realtime mode.
** The arguments are:
** in_realtime_timeout - is a timeout in seconds.
**            If the realtime mode lasts this number of seconds, it is
**            timed out, to prevent the application from getting stuck.
*/
void setroundrobin(long in_realtime_timeout);

/* Leave realtime mode. */
void normal_sched(void);

/* Use nanosleep for realtime delays */
void delaynus(long del);

/****************************************************************************/

#ifdef __cplusplus
}; /* end of extern "C" */
#endif

#endif /* _REALTIME_H_ */
/* End of realtime.h */

#endif /* REALTIME */

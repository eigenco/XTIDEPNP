/*****************************************************************************
**
** iopl.c
**
** Acquire/Relinquish I/O port access privileges.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static char rcsid[] __attribute__((unused)) = "$Id: iopl.c,v 0.4 2001/04/30 21:52:40 fox Exp $";

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <isapnp/iopl.h>
#include <isapnp/pnp.h>

/* Acquire I/O port privileges needed for ISA PnP configuration.
** The return value is 0 on success, or the value of errno on failure.
*/
int
acquire_pnp_io_privileges(void)
{
  int ret = 0;
#ifdef HAVE_IOPL
  /*
   * Have to get unrestricted access to io ports, as WRITE_DATA port > 0x3ff
   */
  ret = iopl(3);
#else
#ifdef HAVE_IOPERM
  /* ALPHA only has ioperm, apparently, so cover all with one permission */
  ret = ioperm(MIN_READ_ADDR, WRITEDATA_ADDR - MIN_READ_ADDR + 1, 1);
#endif /* HAVE_IOPERM */
#endif /* !HAVE_IOPL */

  if (ret < 0) {
    return(errno);
  }
  else {
    return(0);
  }
}

/* Relinquish I/O port privileges needed for ISA PnP configuration.
** The return value is 0 on success, or the value of errno on failure.
*/
int
relinquish_pnp_io_privileges(void)
{
  int ret = 0;
#ifdef HAVE_IOPL
  ret = iopl(0);
#else
#ifdef HAVE_IOPERM
  ret = ioperm(MIN_READ_ADDR, WRITEDATA_ADDR - MIN_READ_ADDR + 1, 0);
#endif /* HAVE_IOPERM */
#endif /* !HAVE_IOPL */

  if (ret < 0) {
    return(errno);
  }
  else {
    return(0);
  }
}

/****************************************************************************/

/* End of iopl.c */

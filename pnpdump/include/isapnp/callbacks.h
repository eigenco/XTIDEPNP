/*****************************************************************************
**
** callbacks.h
** $Header: /free/fox/mystuff/isapnptools-1.26/include/isapnp/RCS/callbacks.h,v 0.3 2001/04/30 22:06:59 fox Exp $
**
** Callbacks used by all libisapnp.a modules for requesting the caller to
** abort the application, to report error messages, and to report progress
** messages.
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

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

/* The following declares the typedefs of the callback procedures needed
** by realtime.c procedures.
** It also declares the progress_report_buf[] buffer.
*/
#ifdef HAVE_SFN
#include <isapnp/pnp_acce.h>
#else
#include <isapnp/pnp-access.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
** Fatal errors - the callback is expected to abort the process.            **
** This callback must be provided by the caller.                            **
** It is anticipated that callers, which don't wish to abort the process    **
** altogether, will use setjmp/longjmp to exit the fatal errors callback.   **
**                                                                          **
** The argument is the error code (see errcodes.h include file).            **
** This typedef was removed from pnp-access.h, because we don't want        **
** ugly aborts in library procedures.                                       **
*****************************************************************************/
typedef void fatal_error(int in_isapnp_error);


/****************************************************************************/
/* Function pointers to be used by code which needs to invoke the callbacks */

extern fatal_error     *st_fatal_error_callback;
extern non_fatal_error *st_non_fatal_error_callback;
extern progress_report *st_progress_report_callback;

/*****************************************************************************
**     Auxiliary memory buffer for building progress report messages        **
**                                                                          **
** The declaration of progress_report_buf[] was moved to pnp-access.h.      **
*****************************************************************************/

/****************************************************************************/
/* Initialize callbacks.
** The arguments are:
** in_fatal_error_callback - callback for reporting fatal errors.
**            The callback is not expected to return.
**            Must not be set to NULL.
** in_non_fatal_error_callback - callback for reporting non-fatal errors.
**            The callback is expected to return.
**            May be set to NULL if the application wants to ignore
**            non-fatal errors.
** in_progress_report_callback - callback for issuing progress reports.
**            The callback is expected to return.
**            May be set to NULL if the application wants to ignore
**            progress reports.
*/
void callbacks_init(fatal_error *in_fatal_error_callback,
		    non_fatal_error *in_non_fatal_error_callback,
		    progress_report *in_progress_report_callback);

/****************************************************************************/
/*                                                                          */
/* Standard callback functions in the library and error messages            */
/*                                                                          */
/****************************************************************************/

extern char *st_error_messages[];

extern fatal_error     normal_fatal_error_callback;

extern non_fatal_error null_non_fatal_error_callback;
extern non_fatal_error normal_non_fatal_error_callback;

extern progress_report null_progress_report_callback;

/****************************************************************************/

#ifdef __cplusplus
}; /* end of extern "C" */
#endif

#endif /* _CALLBACKS_H_ */
/* End of callbacks.h */

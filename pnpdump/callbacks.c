#include <stdlib.h>
/*****************************************************************************
**
** callbacks.c
**
** Callbacks used by all libisapnp.a modules for requesting the caller to
** abort the application, to report error messages, and to report progress
** messages.
**
** Copyright (C) 1999  Omer Zak (omerz@actcom.co.il)
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

static char st_callbacks_version[] __attribute__((unused)) = "$Id: callbacks.c,v 0.4 2001/04/30 21:50:29 fox Exp $";

#include <isapnp/release.h>
#ifdef HAVE_SFN
#include <isapnp/callback.h>
#else
#include <isapnp/callbacks.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>

/****************************************************************************/

/* Forward declarations */
non_fatal_error null_non_fatal_error_callback;
progress_report null_progress_report_callback;

fatal_error *st_fatal_error_callback = NULL;
non_fatal_error *st_non_fatal_error_callback = &null_non_fatal_error_callback;
progress_report *st_progress_report_callback = &null_progress_report_callback;

/****************************************************************************/
/*     Auxiliary memory buffer for building progress report messages        */

char progress_report_buf[PROGRESS_REPORT_BUFFER_SIZE+3];

/****************************************************************************/

/* Null non-fatal error callback - is used by default if the caller set
** the non-fatal errors callback to NULL.
*/
void
null_non_fatal_error_callback(int in_errno, int in_isapnp_error)
{
}

/* Null progress report callback - is used by default if the caller set
** the progress report callback to NULL.
*/
void
null_progress_report_callback(const char *in_msg)
{
}

/****************************************************************************/

/****************************************************************************/
/* Callbacks needed by the libisapnp procedures                             */
/****************************************************************************/

char *st_error_messages[] = {
#define ISAPNP_ERR_CODE(code,msg) msg,
#include <isapnp/errcodes.h>
};

/* Forward declarations - this way I am making sure that the typedefs and the
** actual declarations (below) match each other.
*/
fatal_error     normal_fatal_error_callback;
non_fatal_error normal_non_fatal_error_callback;

/* The callbacks themselves */
void
normal_fatal_error_callback(int in_isapnp_error)
{
  if (in_isapnp_error >= ISAPNP_E_LAST_ERROR_CODE) {
    fprintf(stderr, "Unknown fatal error - error code is %d.\n",
	    in_isapnp_error);
  }
  else {
    fprintf(stderr,"%s\n",st_error_messages[in_isapnp_error]);
  }
  exit(1);
}

void
normal_non_fatal_error_callback(int in_errno, int in_isapnp_error)
{
  if (in_isapnp_error >= ISAPNP_E_LAST_ERROR_CODE) {
    fprintf(stderr, "Unknown fatal error - error code is %d: %s\n",
	    in_isapnp_error, strerror(in_errno));
  }
  else if (ISAPNP_E_PRINT_PROGRESS_REPORT_BUF == in_isapnp_error) {
    fprintf(stderr, progress_report_buf);
  }
  else {
    if (0 == in_errno) {
      fprintf(stderr,"%s\n", st_error_messages[in_isapnp_error]);
    }
    else {
		perror(st_error_messages[in_isapnp_error]);
    }
  }
}

/****************************************************************************/

/* Initialize callbacks */

void callbacks_init(fatal_error *in_fatal_error_callback,
		    non_fatal_error *in_non_fatal_error_callback,
		    progress_report *in_progress_report_callback)
{
  if (NULL == in_fatal_error_callback) {
    fprintf(stderr,
	    "Fatal error callback was not set - aborting application\n");
    exit(1);
  }
  st_fatal_error_callback = in_fatal_error_callback;
  if (NULL != in_non_fatal_error_callback) {
    st_non_fatal_error_callback = in_non_fatal_error_callback;
  }
  if (NULL != in_progress_report_callback) {
    st_progress_report_callback = in_progress_report_callback;
  }
}

/****************************************************************************/


/* End of callbacks.c */

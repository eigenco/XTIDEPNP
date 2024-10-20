/*****************************************************************************
**
** pnp-access.c
**
**
** Contains the interrogate_isapnp() procedure, which encapsulates
** everything having to do with creation of the ISA PnP resources
** list.
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

static char st_pnp_access_version[] __attribute__((unused)) = "$Id: pnp-access.c,v 0.3 2001/04/30 21:50:29 fox Exp $";

#include <stdio.h>            /* only for NULL declaration */
#include <setjmp.h>
#include <isapnp/release.h>
#ifdef HAVE_SFN
#include <isapnp/callback.h>
#include <isapnp/pnp_acce.h>
#include <isapnp/res_acce.h>
#else
#include <isapnp/callbacks.h>
#include <isapnp/pnp-access.h>
#include <isapnp/res-access.h>
#endif
#include <isapnp/iopl.h>
#ifdef REALTIME
#include <isapnp/realtime.h>
#endif
#include <isapnp/errenum.h>

/****************************************************************************/
/* Fatal error callback - is not visible to the user */
fatal_error     pnpdump_fatal_error_callback;

jmp_buf st_fatal_error_jmp_buf;  /* Used to handle fatal errors */
int st_fatal_isapnp_error_code;  /* set by the callback for
				 ** interrogate_isapnp()
				 */

void
pnpdump_fatal_error_callback(int in_isapnp_error)
{
  longjmp(st_fatal_error_jmp_buf,st_fatal_isapnp_error_code);

  /*
!!!!!!!! PREVIOUS CODE - MOVE TO DEMO2 !!!!!!!!!!!!
  if (in_isapnp_error >= ISAPNP_E_LAST_ERROR_CODE) {
    fprintf(stderr, "Unknown fatal error - error code is %d.\n",
	    in_isapnp_error);
  }
  else {
    fprintf(stderr,"%s\n",st_error_messages[in_isapnp_error]);
  }
  exit(1);
!!!!!!!!!!!!! End of previous code !!!!!!!!!
  */
}

/*****************************************************************************
** Interrogate the ISA PnP cards and optionally allocate resources.
** The arguments are:
** in_args_p - points at a structure containing all input arguments and
**             settings of the command line flags (if any).
** out_results_p - points at a structure containing all results.
**
** The return value of the procedure is 0 for normal return.  In this
** case, *out_results_p contains valid results.
** If anything was wrong, a nonzero value is returned.
*/

int
interrogate_isapnp(const interrogate_args *in_args_p,
		   interrogate_results *out_results_p)
{
  int l_card_index;
  int l_ret;

  /**************************************************************************/
  /* First thing to do - initialize callbacks */
  l_ret = setjmp(st_fatal_error_jmp_buf);
  if (l_ret != 0) {
    /* Returned here from a longjmp() call */
    return(st_fatal_isapnp_error_code);
  }
  callbacks_init(pnpdump_fatal_error_callback,
		 in_args_p->m_non_fatal_error_callback_p,
		 in_args_p->m_progress_report_callback_p);

  /**************************************************************************/

  if (acquire_pnp_io_privileges() != 0) {
    return(ISAPNP_E_PNPACC_NO_PERMISSION);
  }

  /**************************************************************************/

  /* Have to do this anyway to check readport for clashes */
  alloc_system_resources();
#ifdef REALTIME
  setroundrobin(in_args_p->m_realtime_timeout);
#endif /* REALTIME */

  /**************************************************************************/

  /* Now get board info */
  /* It is to be noted that the following code is kludge, and it should be
  ** cleaned up in a future version of the libisapnp library.
  */
  /* Arguments for initiate() */
  do_fullreset = in_args_p->m_reset_flag;
  do_ignorecsum = in_args_p->m_ignore_csum_flag;
  debug_isolate = in_args_p->m_debug_flag;
  boards_found = (in_args_p->m_numcards >= 0) ? in_args_p->m_numcards : 0;
  read_port = (in_args_p->m_readport >= 0) ? in_args_p->m_readport : 0;
  l_ret = initiate();

  if ((-1) == l_ret) {
    /* No ISA PnP boards were found. */
    out_results_p->m_numcards_found = 0;
    out_results_p->m_readport_found = read_port;  /* Meaningless */
    out_results_p->m_resource_p = NULL;
    out_results_p->m_resource_count = 0;
  }
  else {
    /* Retrieve actual No. of boards found and actual read_port used. */
    out_results_p->m_numcards_found = boards_found;
    out_results_p->m_readport_found = read_port;

    /* Actually retrieve board information */
    out_results_p->m_resource_p = NULL;
    out_results_p->m_resource_count = 0;
    for (l_card_index = 1;
	 l_card_index <= out_results_p->m_numcards_found;
	 l_card_index++) {
      read_board(l_card_index,
		 &out_results_p->m_resource_p,
		 &out_results_p->m_resource_count);
      /* Here one could call dumpregs() (or equivalent) if one wants
      ** the detailed information from the card's registers.
      */
    }
  }

  /* Finished getting board information. */
#ifdef REALTIME
  /* No longer need real-time scheduling */
  normal_sched();
#endif /* REALTIME */
  /* Release resources */
  if (relinquish_pnp_io_privileges() != 0) {
    return(ISAPNP_E_PNPACC_NO_PERMS_RELEASE);
  }

  return(0);
}

/****************************************************************************/
/* End of pnp-access.c */

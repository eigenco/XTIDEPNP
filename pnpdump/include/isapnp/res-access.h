/*****************************************************************************
**
** res-access.h
** $Header: /free/fox/mystuff/isapnptools-1.26/include/isapnp/RCS/res-access.h,v 0.2 2001/04/30 22:06:59 fox Exp $
**
** Resource access support code for pnpdump and for code, which uses
** the pnpdump features library (libisapnp.a/libisapnp.so).
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

#ifndef _RES_ACCESS_H_
#define _RES_ACCESS_H_

#include <isapnp/resource.h>
#ifdef HAVE_SFN
#include <isapnp/pnp_acce.h>
#else
#include <isapnp/pnp-access.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/

extern int boards_found;
extern int do_fullreset; /* a command line option */
extern int do_ignorecsum; /* a command line option */
extern int debug_isolate; /* a command line option */


#define NUM_CARDS 128
#define IDENT_LEN 9
#define TMP_LEN 16

extern unsigned char serial_identifier[NUM_CARDS + 1][IDENT_LEN];
extern unsigned char csumdiff[NUM_CARDS + 1];

extern int csum;
extern int read_port;
#define ASSERT(condition)								   /* as nothing */


/****************************************************************************/

typedef void resource_inspector
     (FILE * output_file, struct resource * res, int selected);
/* !!! Change the names to resource_handler and resource_handler_p */

/* The following declaration was moved to pnp-access.h
** typedef void (*resource_handler)
**     (FILE * output_file, struct resource * res, int selected);
*/

int statuswait(void);
int initiate(void);
  /* Return value is -1 if no board was found.
  ** Otherwise, the return value is 0.
  */

/* The following declaration was moved to pnp-access.h
** void for_each_resource(struct resource *res,
**		       int num_resources,
**		       resource_handler handler,
**		       FILE * output_file,
**		       int selected);
*/

void read_board_resources(struct resource *first_element,
			  int dependent_clause,
			  struct resource **result_array_ptr,
			  int *count);

void read_board(int csn, struct resource **res_list, int *list_len);

void unread_resource_data(char prev);

/****************************************************************************/

#ifdef __cplusplus
}; /* end of extern "C" */
#endif

#endif /* _RES_ACCESS_H_ */
/* End of res-access.h */

/*****************************************************************************
**
** pnp-access.h
** $Header: /free/fox/mystuff/isapnptools-1.26/include/isapnp/RCS/pnp-access.h,v 0.1 2000/04/19 21:43:46 fox Exp $
**
** pnp-access.h declares and documents all interface know-how needed
** by users of the library libisapnp.a.
** The isapnptools package contains the files demo.c and demo2.c, which
** demonstrate the points made in this file.
**
******************************************************************************
**
** pnp-access.c Contains the interrogate_isapnp() procedure, which
** encapsulates everything having to do with creation of the ISA
** PnP resources list.
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

#ifndef _PNP_ACCESS_H_
#define _PNP_ACCESS_H_

/* INFORMATION ONLY
** The following header file declares the version and a string
** literal quoting the compile-time flags used for building this
** package.
** The information in this header file is not needed for normal
** use of the libisapnp.a library.
*/
#include <isapnp/release.h>

/* INFORMATION ONLY - relevant for your resource inspection code.
** The following header file declares the following:
** struct resource - describes the layout of a single PnP resource
**                   record.  Consult the header file and the file
**                   demo2.c to understand the meanings of the various
**                   struct resource's fields.
** struct alternative - one of the fields of struct resource.
**
** See below for discussion of the alloc_resources(), which is
** declared, too, in resource.h.
*/
#include <isapnp/resource.h>

/* INFORMATION ONLY - relevant for your resource inspection code.
** The following header file declares the various resource tags used
** in case statements in code for building and interpreting the
** struct resource records.
** Look for "Short  Tags" and "Long  Tags".
*/
#include <isapnp/pnp.h>

/*****************************************************************************
** Typedefs of callback procedures needed by realtime.c procedures          **
** You must define those procedures in your code.  In the sample code, they **
** exist in demo.c.                                                         **
******************************************************************************
** The sample callbacks provided in demo.c declare an array of error        **
** messages as follows:                                                     **
++ char *st_error_messages[] = {                                            ++
++ #define ISAPNP_ERR_CODE(code,msg) msg,                                   ++
++ #include <isapnp/errcodes.h>                                             ++
++ };                                                                       ++
**                                                                          **
** In addition to the above, enumerations for the error messages need to be **
** declared, and this is accomplished by the following include file.        **
*****************************************************************************/
#include <isapnp/errenum.h>

/* Non-fatal errors - the callback is expected to somehow report
** the error and return back.
** This callback is optional.
** There are three possible cases of error codes:
** 1. in_isapnp_error is equal to ISAPNP_E_PRINT_PROGRESS_REPORT_BUF:
**    This is not really an error.  This code is used when it is desired
**    to output progress reports in a different way (typically to
**    stderr) than those outputted by the progress_report callback
**    (see below), which is typically to write to stdout.
** 2. in_errno is equal to 0:
**    This is an libisapnp-reported error, which corresponds to no
**    system error (in_errno is always set to system-provided errno).
** 3. in_errno is not equal to 0:
**    This is an error, which was caused by a system error.
**    libisapnp augments the system error message by additional
**    information.
*/
typedef void non_fatal_error(int in_errno, int in_isapnp_error);

/* Progress reports - the callback is typically used to dump PnP
** information to stdout.
** This callback is optional, and is expected not to be used in
** applications which don't create isapnp.conf-formatted files.
*/
typedef void progress_report(const char *in_msg);

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/

/* The following buffer can be used by callback procedures to compose their
** messages.
*/
#define PROGRESS_REPORT_BUFFER_SIZE 1023
extern char progress_report_buf[PROGRESS_REPORT_BUFFER_SIZE+3];


/*****************************************************************************
** In your program, declare:                                                **
++  interrogate_args l_interrog_args;                                       ++
++  interrogate_results l_interrog_results;                                 ++
**                                                                          **
** Then initialize l_interrog_args according to your needs.                 **
** Sample initialization code follows:                                      **
++  l_interrog_args.m_non_fatal_error_callback_p = NULL;                    ++
++  l_interrog_args.m_progress_report_callback_p = NULL;                    ++
++  l_interrog_args.m_numcards = -1;  // Default = autodetect **            ++
++  l_interrog_args.m_readport = -1;  // Default = autodetect **            ++
++  l_interrog_args.m_reset_flag = 0;                                       ++
++  l_interrog_args.m_ignore_csum_flag = 0;                                 ++
++#ifdef REALTIME                                                           ++
++  l_interrog_args.m_realtime_timeout = 5L;                                ++
++#endif                                                                    ++
++  l_interrog_args.m_debug_flag = 0;                                       ++
*****************************************************************************/

/* Input arguments to interrogate_isapnp() */
typedef struct interrogate_args {
  /* Callbacks */
  non_fatal_error *m_non_fatal_error_callback_p;
  progress_report *m_progress_report_callback_p;

  /* Options */
  int m_numcards;          /* "numcards" -1 if this is to be
			      automatically determined */
  int m_readport;          /* "readport" -1 if this is to be
			      automatically determined */
  int m_reset_flag;        /* "reset" */
  int m_ignore_csum_flag;  /* "ignorecsum" */
#ifdef REALTIME
  long m_realtime_timeout; /* "max-realtime" */
#endif
  int m_debug_flag;        /* "debug" */
} interrogate_args;

/* Outputs of interrogate_isapnp() */
typedef struct interrogate_results {
  struct resource *m_resource_p;
  int m_resource_count;
  int m_numcards_found;
  int m_readport_found;
} interrogate_results;

/****************************************************************************/

/*****************************************************************************
** After having initialized the l_interrog_args structure, you may call     **
** interrogate_isapnp() as follows:                                         **
++  l_ret = interrogate_isapnp(&l_interrog_args, &l_interrog_results);      ++
++  if (0 != l_ret) {                                                       ++
++    // Insert your error handling code, if so desired. **                 ++
++    printf("!!! Error while trying to interrogate the"                    ++
++           " ISA PnP cards !!!\n");                                       ++
++    exit(1);                                                              ++
++  }                                                                       ++
******************************************************************************
** Interrogate the ISA PnP cards and optionally allocate resources.
** The arguments are:
** in_args_p - points at a structure containing all input arguments and
**             settings of the command line flags (if any).
** out_results_p - points at a structure containing all results.
**
** The return value of the procedure is 0 for normal return.  In this
** case, *out_results_p contains valid results.
** If anything was wrong, a nonzero value is returned.
*****************************************************************************/

int
interrogate_isapnp(const interrogate_args *in_args_p,
		   interrogate_results *out_results_p);

/*****************************************************************************
** After getting the results from interrogating the ISA PnP cards, you may  **
** optionally call alloc_resources() to select optimal configuration for    **
** the cards.                                                               **
** This procedure is declared in resource.h (see above).                    **
** The calling sequence is as follows:                                      **
++  l_alloc_result = alloc_resources(l_interrog_results.m_resource_p,       ++
++				     l_interrog_results.m_resource_count,   ++
++				     NULL, 0);                              ++
**                                                                          **
** You probably want to go through the resource records' array pointed at   **
** by l_interrog_results.m_resource_p.  The procedure for_each_resource()   **
** provides this service.                                                   **
** You must declare your inspect_resource() procedure with the appropriate  **
** prototype.                                                               **
** demo2.c provides also sample prepare_to_inspect() and                    **
** finish_inspection() to be called before&after going through the whole    **
** resource records' array.                                                 **
**                                                                          **
** Assuming the following prototype declarations:                           **
++ void prepare_to_inspect(void);                                           ++
++ resource_inspector inspect_resource;                                     ++
++ void finish_inspection(void);                                            ++
**                                                                          **
** The calling sequence is as follows:                                      **
++  prepare_to_inspect();                                                   ++
++  for_each_resource(l_interrog_results.m_resource_p,                      ++
++		      l_interrog_results.m_resource_count,                  ++
++		      inspect_resource, stdout, l_alloc_result);            ++
++  finish_inspection();                                                    ++
** where 'stdout' is a FILE* variable which is passed directly to your      **
** inspect_resource() procedure and can be ignored there.                   **
** l_alloc_result is set to either 0 or 1, depending upon the results of    **
** the alloc_resources() call you made earlier.  If you didn't call         **
** alloc_resources(), l_alloc_result should be set to 0.                    **
*****************************************************************************/

#include <stdio.h>        /* to declare the FILE data type used below */

typedef void (*resource_handler)
     (FILE * output_file, struct resource * res, int selected);


void for_each_resource(struct resource *res,
		       int num_resources,
		       resource_handler handler,
		       FILE * output_file,
		       int selected);

/*****************************************************************************
** After getting the results from interrogating the ISA PnP cards and/or    **
** allocating resources, you may retrieve additional information by calling **
** any of the following procedures.                                         **
*****************************************************************************/

/* The arguments are pointers to variables, which will receive bit masks
** indicating which IRQ/DMA lines are free.
** In each bit mask, set bit ith indicates that line ith is free.
**
** Note:  in PCs, there are 16 IRQ lines and 8 DMA lines.
** IRQ2 is always considered to be in use.
**
** NULL may be supplied as an argument, if the corresponding value is of no
** interest to the caller.
**
** The return code is 0 if the procedure call was successful.
** Otherwise, a nonzero error code is returned.
*/
int get_free_irqs_dmas(long *out_irq_p, long *out_dma_p);

/*****************************************************************************
** The following procedures allow you to select a card and retrieve a lot   **
** of information about it.                                                 **
*****************************************************************************/

/* Retrieve the total number of ISA PnP cards found.
** The first argument points at the information returned
** from interrogate_isapnp().
** The result is deposited at the address pointed at by the second argument.
** The return code is used to indicate success (0) or failure (nonzero).
*/
int get_number_of_cards(const interrogate_results *in_results_p,
			int *out_number_of_cards);

/* Retrieve the device ID string for a card.
**
** The first argument points at the information returned
** from interrogate_isapnp().
**
** The second argument is the ID number of the card whose ID string
** is desired.  The ID number is in range between 0 and N-1 where N
** is the total number of cards whose resources are described by the
** first argument.
**
** The memory area, at which the third argument points, must be at
** least 5 bytes long.
**
** The return value indicates success (0) or failure (nonzero).
*/
int retrieve_device_ID(const interrogate_results *in_results_p,
		       int in_card_ID,
		       char *out_ID_str);

/****************************************************************************/

/* Do pnpdump */
int pnpdump_main(int argc, char **argv);

/* Do isapnp */
int isapnp_main(int argc, char **argv);

#ifdef __cplusplus
}; /* end of extern "C" */
#endif

#endif /* _PNP_ACCESS_H_ */
/* End of pnp-access.h */

/*****************************************************************************
**
** errcodes.h
** $Header: /free/fox/mystuff/isapnptools-1.26/include/isapnp/RCS/errcodes.h,v 0.3 2000/07/25 19:38:14 fox Exp $
**
** Declaration of error codes used by the pnpdump related code which is
** put in libisapnp.a.
**
** Before including this file, you must #define ISAPNP_ERR_CODE(code,msg).
** Typical use is to include this file in an header file which declares
** error codes, and in a .c file, which declares an array of message
** strings.
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

/* This header file may be included several times. */
/* #ifndef _ERRCODES_H_ */
/* #define _ERRCODES_H_ */

#ifndef ISAPNP_ERR_CODE
#error "You must have defined ISAPNP_ERR_CODE before including this file"
#endif /* ISAPNP_ERR_CODE */

/****************************************************************************/

/* Special error codes, which should be handled specifically by callbacks. */

ISAPNP_ERR_CODE(ISAPNP_E_PRINT_PROGRESS_REPORT_BUF,"The error message is in progress_report_buf - display/print its contents")

/* realtime package's error codes */

ISAPNP_ERR_CODE(ISAPNP_E_TIME_EXPIRED,"Time expired - aborting program")
ISAPNP_ERR_CODE(ISAPNP_E_REALTIME_NOT_CLEARED,"Couldn't clear real-time scheduling")
ISAPNP_ERR_CODE(ISAPNP_E_REALTIME_NOT_CLEARED2,"Couldn't clear real-time scheduling, may continue to use all CPU for a while")
ISAPNP_ERR_CODE(ISAPNP_E_REALTIME_NOT_SET,"Couldn't set real-time scheduling, may be a bit slow")
ISAPNP_ERR_CODE(ISAPNP_E_REALTIME_EXCEEDED,"REALTIME operation timeout exceeded - Switching to normal scheduling")
     /* If the application is writing to stdout, it should do fflush(stdout)
     ** in the callback when this error is seen. */
ISAPNP_ERR_CODE(ISAPNP_E_REALTIME_NANOSLEEP_FAILED,"nanosleep failed")

/* Resource access package's error codes */

ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_TIMEOUT_READ,"Timeout attempting to read resource data - is READPORT correct ?\n")
ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_TOO_MANY_BOARDS,"Too many boards found, recompile with NUM_CARDS bigger\n")
ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_MALLOC_FAILURE,"read_one_resource: memory allocation failure.\n")
ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_MALLOC_FAILURE2,"read_board_resources: malloc of result_array failed.\n")
ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_REALLOC_FAILURE,"read_board_resources: realloc of result_array failed.\n")
ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_REALLOC_FAILURE2,"read_board_resources: memory allocation for alternatives failed.\n")
ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_REALLOC_FAILURE3,"read_board: failed to allocate memory for res_array.\n")
ISAPNP_ERR_CODE(ISAPNP_E_ACCESS_MALLOC_FAILURE3,"read_board: failed to allocate memory for board identifier.\n")

/* Resource management package's error codes */

ISAPNP_ERR_CODE(ISAPNP_E_RES_MALLOC_FAILURE,"alloc_in_range_list: malloc failure.\n")
ISAPNP_ERR_CODE(ISAPNP_E_RES_CONFLICT_NOT_CHECKED,"Neither /proc/bus/pci/devices nor /proc/pci found, so PCI resource conflict not checked\n")

/* interrogate_isapnp() error codes */

ISAPNP_ERR_CODE(ISAPNP_E_PNPACC_NO_PERMISSION,"Unable to get io permission for WRITE_DATA")
ISAPNP_ERR_CODE(ISAPNP_E_PNPACC_NO_PERMS_RELEASE,"Unable to release io permission for WRITE_DATA")
/* ISAPNP_ERR_CODE(ISAPNP_W_NO_CARDS_FOUND,"No cards were found (this may not be an error)") */

/* cardinfo.c error codes */
ISAPNP_ERR_CODE(ISAPNP_E_CARDINFO_BAD_IRQ,"Bad IRQ in range_list structure")
ISAPNP_ERR_CODE(ISAPNP_E_CARDINFO_BAD_DMA,"Bad DMA in range_list structure")
ISAPNP_ERR_CODE(ISAPNP_E_CARDINFO_NULL_RESO,"NULL pointer to ISA PnP resources information")
ISAPNP_ERR_CODE(ISAPNP_E_CARDINFO_NULL_RESULT_PTR,"NULL pointer to memory area, which is supposed to receive the result")
ISAPNP_ERR_CODE(ISAPNP_E_CARDINFO_CARD_ID_OUT_OF_RANGE,"Given card number is out of range")
ISAPNP_ERR_CODE(ISAPNP_E_CARDINFO_INTERNAL_ERROR,"Internal software error in module cardinfo.c")

#ifndef HAVE_SNPRINTF
/* DOS snprintf() detected buffer overrun */
ISAPNP_ERR_CODE(ISAPNP_E_SNPRINTF_BUF_OVERRUN,"snprintf() buffer overflowed - aborting program.\n")
#endif

     /*
ISAPNP_ERR_CODE(,)
     */

ISAPNP_ERR_CODE(ISAPNP_E_LAST_ERROR_CODE,"Unknown error code")
/****************************************************************************/



/****************************************************************************/

/* Undefine ISAPNP_ERR_CODE() to ensure that the user has defined it
** appropriately before next inclusion of errcodes.h.
*/
#undef ISAPNP_ERR_CODE

/* #endif ** _ERRCODES_H_ */
/* End of errcodes.h */

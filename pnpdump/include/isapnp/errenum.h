/*****************************************************************************
**
** errenum.h
** $Header: /free/fox/mystuff/isapnptools-1.26/include/isapnp/RCS/errenum.h,v 0.1 2000/04/19 21:51:03 fox Exp $
**
** Declaration of enumeration of error codes used by the pnpdump related
** code which is put in libisapnp.a.
**
** This header file includes errcodes.h after defining the macro
** ISAPNP_ERR_CODE(code,msg) so that it'll participate in an enum
** declaration.
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

#ifndef _ERRENUM_H_
#define _ERRENUM_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/

#define ISAPNP_ERR_CODE(code,msg) code,

enum {
#include <isapnp/errcodes.h>
};

/****************************************************************************/

#ifdef __cplusplus
}; /* end of extern "C" */
#endif

#endif /* _ERRENUM_H_ */
/* End of errenum.h */

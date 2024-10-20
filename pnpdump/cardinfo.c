/*****************************************************************************
**
** cardinfo.c
**
** Interrogate various internal structures in order to provide user-requested
** information about available ISA PnP cards.
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

static char rcsid[] __attribute__((unused)) = "$Id: cardinfo.c,v 0.3 2001/04/30 21:50:29 fox Exp $";

#ifdef HAVE_SFN
#include <isapnp/res_acce.h>
#include <isapnp/pnp_acce.h>
#else
#include <isapnp/res-access.h>
/* res-access.h is needed only for serial_identifier[]. */
#include <isapnp/pnp-access.h>
#endif
#include <isapnp/errenum.h>

/*
 * Define this to check board ident
 */
/* #define WANT_TO_VALIDATE */

/* #define DEBUG */
#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */
#include <string.h>

/****************************************************************************/

#define IRQ_LINES 16
#define DMA_LINES 8

/****************************************************************************/

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
int
get_free_irqs_dmas(long *out_irq_p, long *out_dma_p)
{
  if (NULL != out_irq_p) {
    struct range_list *l_range_p = tag_range_lists[tag_to_index(IRQ_TAG)];
    long l_result = 0;
    for (; l_range_p != NULL; l_range_p = l_range_p->next) {
      unsigned long l_irq;

#ifdef DEBUG
      printf("cardinfo.c: IRQ range: start=%ld end=%ld source=%s\n",
	     l_range_p->start, l_range_p->end, l_range_p->source);
#endif /* DEBUG */

      for (l_irq = l_range_p->start; l_irq < l_range_p->end; l_irq++) {
	if (l_irq >= IRQ_LINES) return(ISAPNP_E_CARDINFO_BAD_IRQ);
	l_result |= 1 << l_irq;
      }
    }

    /* Mark IRQ0,IRQ2 as always in use */
    l_result |= 1 << 0;
    l_result |= 1 << 2;

#ifdef DEBUG
    printf("cardinfo.c: DMA l_result=0x%02x\n", (int)l_result);
#endif /* DEBUG */

    /* Now l_result indicates the IRQ lines in use.
    ** We want to output indication of free IRQ lines.
    */
    *out_irq_p = l_result ^ ((1 << IRQ_LINES) - 1);
  }
  if (NULL != out_dma_p) {
    struct range_list *l_range_p = tag_range_lists[tag_to_index(DMA_TAG)];
    long l_result = 0;
    for (; l_range_p != NULL; l_range_p = l_range_p->next) {
      unsigned long l_dma;

#ifdef DEBUG
      printf("cardinfo.c: DMA range: start=%ld end=%ld source=%s\n",
	     l_range_p->start, l_range_p->end, l_range_p->source);
#endif /* DEBUG */

      for (l_dma = l_range_p->start; l_dma < l_range_p->end; l_dma++) {
	if (l_dma >= DMA_LINES) return(ISAPNP_E_CARDINFO_BAD_DMA);
	l_result |= 1 << l_dma;
      }
    }

#ifdef DEBUG
    printf("cardinfo.c: DMA l_result=0x%02x\n", (int)l_result);
#endif /* DEBUG */

    /* Now l_result indicates the DMA lines in use.
    ** We want to output indication of free DMA lines.
    */
    *out_dma_p = l_result ^ ((1 << DMA_LINES) - 1);
  }
  return(0);
}

/****************************************************************************/

/* Retrieve the total number of ISA PnP cards found.
** The first argument points at the information returned
** from interrogate_isapnp().
** The result is deposited at the address pointed at by the second argument.
** The return code is used to indicate success (0) or failure (nonzero).
*/
int
get_number_of_cards(const interrogate_results *in_results_p,
		    int *out_number_of_cards)
{
  if (NULL == in_results_p) return(ISAPNP_E_CARDINFO_NULL_RESO);
  if (NULL == out_number_of_cards) return(ISAPNP_E_CARDINFO_NULL_RESULT_PTR);

  *out_number_of_cards = in_results_p->m_numcards_found;
  return(0);
}

/****************************************************************************/

/* Find which resources correspond to card whose ID is given.
** This procedure is internal to cardinfo.c, and is not expected
** to be directly used by users of the module.
*/
int
find_resources_for_card(const interrogate_results *in_results_p,
			int in_card_ID,
			unsigned int *out_first_resource_index_p,
			unsigned int *out_last_resource_index_p)
{
  unsigned int l_index;    /* Index of resource in resources array */
  int l_card;              /* Index of card to which the currently inspected
			   ** resource belongs
			   */
  if (NULL == in_results_p) return(ISAPNP_E_CARDINFO_NULL_RESO);
  if (NULL == out_first_resource_index_p) return(ISAPNP_E_CARDINFO_NULL_RESULT_PTR);
  if (NULL == out_last_resource_index_p) return(ISAPNP_E_CARDINFO_NULL_RESULT_PTR);
  if (in_card_ID >= in_results_p->m_numcards_found) {
    return(ISAPNP_E_CARDINFO_CARD_ID_OUT_OF_RANGE);
  }

  /* The following if statement is in lieu of an assert(). */
  if (NewBoard_PSEUDOTAG != in_results_p->m_resource_p[0].type) {

#ifdef DEBUG
    printf("*** Internal software error, resource[0] type is %d.\n",
	   (int) in_results_p->m_resource_p[0].type);
#endif /* DEBUG */

    return(ISAPNP_E_CARDINFO_INTERNAL_ERROR);
  }

  for (l_index = 0, l_card = -1;
       l_index < in_results_p->m_resource_count; l_index++) {

#ifdef DEBUG
    printf("find_resources_for_card: iteration %d (card %d) res.type 0x%02x\n",
	   (int) l_index, (int) l_card,
	   in_results_p->m_resource_p[l_index].type);
#endif /* DEBUG */

    if (NewBoard_PSEUDOTAG == in_results_p->m_resource_p[l_index].type) {
      /* End of resources corresponding to current l_card */
      if (l_card == in_card_ID) {
	if (0 == l_index) {

#ifdef DEBUG
	  printf("*** Internal software error, at card %d, l_index is zero.\n",
		 (int) l_card);
#endif /* DEBUG */

	  return(ISAPNP_E_CARDINFO_INTERNAL_ERROR);
	}
	*out_last_resource_index_p = l_index-1;
      }
      /* Next card */
      l_card++;
      /* Beginning of resources corresponding to new l_card */
      if (l_card == in_card_ID) {
	*out_first_resource_index_p = l_index;
      }
    }
  }
  /* If in_card_ID was the last card, we didn't update
  ** *out_last_resource_index_p in the loop, so we have to do this now.
  */
  if (in_card_ID == in_results_p->m_numcards_found-1) {
    *out_last_resource_index_p = in_results_p->m_resource_count - 1;
  }
  return(0);
}

/****************************************************************************/
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
int
retrieve_device_ID(const interrogate_results *in_results_p,
		   int in_card_ID,
		   char *out_ID_str)
{
  int l_ret;     /* Return code */
  unsigned int l_first_resource_index;
  unsigned int l_last_resource_index;

  /* Validity checks */
  if (NULL == in_results_p) return(ISAPNP_E_CARDINFO_NULL_RESO);
  if (in_card_ID >= in_results_p->m_numcards_found) {
    return(ISAPNP_E_CARDINFO_CARD_ID_OUT_OF_RANGE);
  }
  if (NULL == out_ID_str) return(ISAPNP_E_CARDINFO_NULL_RESULT_PTR);

  /* Retrieve range of resources associated with the desired card */
  l_ret = find_resources_for_card(in_results_p, in_card_ID,
				  &l_first_resource_index,
				  &l_last_resource_index);
  if (0 != l_ret) return(l_ret);

  /* A validity check */
  if (NewBoard_PSEUDOTAG
      != in_results_p->m_resource_p[l_first_resource_index].type) {

#ifdef DEBUG
    printf("*** Internal software error, 1st resource type is %d\n"
	   "*** 1st resource index is %d, last resource index is %d\n",
	   (int) in_results_p->m_resource_p[l_first_resource_index].type,
	   (int) l_first_resource_index, (int) l_last_resource_index);
#endif /* DEBUG */

    return(ISAPNP_E_CARDINFO_INTERNAL_ERROR);
  }

  /* Retrieve the ID string itself */
  memcpy(out_ID_str,     /* dest */
	 in_results_p->m_resource_p[l_first_resource_index].data, /* src */
	 4);             /* length */
  out_ID_str[4] = '\0';

#ifdef WANT_TO_VALIDATE
  /* Compare it with another place, to validate.  This is not
  ** necessary, and should be removed in a future version.
  */
  l_ret = memcmp(out_ID_str,
		 serial_identifier[in_results_p->m_resource_p[l_first_resource_index].start],
		 4);
  if (0 != l_ret) {

#ifdef DEBUG
    printf("*** Internal software error, card IDs disagree:\n"
	   "*** serial_identifier says 0x%02x %02x %02x %02x,"
	   " 1st resource says 0x%02x %02x %02x %02x\n",
serial_identifier[in_results_p->m_resource_p[l_first_resource_index].start][0],
serial_identifier[in_results_p->m_resource_p[l_first_resource_index].start][1],
serial_identifier[in_results_p->m_resource_p[l_first_resource_index].start][2],
serial_identifier[in_results_p->m_resource_p[l_first_resource_index].start][3],
	   out_ID_str[0],out_ID_str[1],out_ID_str[2],out_ID_str[3]);
#endif /* DEBUG */

    return(ISAPNP_E_CARDINFO_INTERNAL_ERROR);
  }
#endif /* WANT_TO_VALIDATE */

  return(0);
}

/****************************************************************************/
/* End of cardinfo.c */

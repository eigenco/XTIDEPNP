/*****************************************************************************
**
** res-access.c
**
** Resource access support code for pnpdump and for code, which uses
** the pnpdump features library (libisapnp.a/libisapnp.so).
**
** The code in this file was extracted from isapnptools-1.19/pnpdump.c
** and is subject to the same copyright and licensing restrictions of
** the above package.  In particular, in spite of any comments below,
** this file is released under the terms of GPL rather than LGPL!
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

static char st_res_access_version[] __attribute__((unused)) = "$Id: res-access.c,v 0.6 2001/04/30 21:50:29 fox Exp $";

/*
 * Define this to enable tag debugging code
 */
/* #undef TAG_DEBUG */

#include <isapnp/release.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <isapnp/pnp.h>
#include <isapnp/realtime.h>
#ifdef HAVE_SFN
#include <isapnp/res_acce.h>
#include <isapnp/callback.h>
#else
#include <isapnp/res-access.h>
#include <isapnp/callbacks.h>
#endif
#include <isapnp/errenum.h>
#include <isapnp/iopl.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define HAVE_SNPRINTF

#ifndef HAVE_SNPRINTF
#include <isapnp/mysnprtf.h>
#endif

/****************************************************************************/

int do_fullreset = 0;  /* a command line option */
int do_ignorecsum = 0; /* a command line option */
int debug_isolate = 0; /* a command line option */
int boards_found = 0; /* optional command line option */
int read_port = START_READ_ADDR; /* optional command line option */

unsigned char serial_identifier[NUM_CARDS + 1][IDENT_LEN];
unsigned char csumdiff[NUM_CARDS + 1];

int backspaced = 0;
char backspaced_char;
int csum = 0;

unsigned char st_tmp[TMP_LEN];

/****************************************************************************/

#include <dos.h>
#define HAVE_DELAY

#ifdef HAVE_USLEEP
#define HAVE_USLEEPANY
#else
#ifdef HAVE_DELAY
unsigned 
usleep(unsigned wait)
{
	return (delay((wait + 990) / 1000), 0);
}
#define HAVE_USLEEPANY
#else
#ifdef HAVE__SLEEP2
unsigned 
usleep(unsigned wait)
{
	return (_sleep2((wait + 990) / 1000));
}
#define HAVE_USLEEPANY
#endif /* HAVE__SLEEP2 */
#endif /* !HAVE_DELAY */
#endif /* !HAVE_USLEEP */

#ifndef REALTIME
#define delaynus(x) usleep(x)
#endif

#define ASSERT(condition)	/* as nothing */

/****************************************************************************/
/* Forward declarations of functions which are used only in this module */
int do_isolate(void);
void read_idents(void);

/****************************************************************************/

/*
 * Wait for resource data byte
 *
 * Return true if error occurred (timeout)
 */
int
statuswait(void)
{
#ifdef REALTIME
#define TIMEOUTLOOPS 100
	int to;												   /* For timeout */
	/*
	 * Try for up to 1ms
	 */
	for (to = 0; to < TIMEOUTLOOPS; to++)
	{
		if (STATUS & 1)
			break;
		delaynus(10L);
	}
	if (to >= TIMEOUTLOOPS)
	{
	  st_non_fatal_error_callback(0, ISAPNP_E_ACCESS_TIMEOUT_READ);
	  return (1);
	}
#else /* !REALTIME */
#ifdef HAVE_USLEEPANY
#define TIMEOUTLOOPS 2
	int to;												   /* For timeout */
	/*
	 * Try for up to 2ms
	 */
	for (to = 0; to < TIMEOUTLOOPS; to++)
	{
		usleep(1000);
		if (STATUS & 1)
			break;
	}
	if (to >= TIMEOUTLOOPS)
	{
	  st_non_fatal_error_callback(0, ISAPNP_E_ACCESS_TIMEOUT_READ);
	  return (1);
	}
#else /* !HAVE_USLEEPANY */
	/*
	 * Infinite loop potentially, but if we usleep, we may lose 10ms
	 */
	while (!(STATUS & 1))
		;
#endif /* !HAVE_USLEEPANY */
#endif /* !REALTIME */
	return (0);
}

int port;

void
sendkey(void)
{
	static char initdata[INIT_LENGTH] = INITDATA;
	int i;
	ADDRESS(0);
	ADDRESS(0);
	for (i = 0; i < INIT_LENGTH; i++)
	{
		ADDRESS(initdata[i]);
	}
}

/* Return value is -1 if no board was found.
** Otherwise, the return value is 0.
*/
int
initiate(void)
{
	int i;
	if (!boards_found)
	{
		/* All cards now isolated, read the first one */
		for (port = read_port; port <= MAX_READ_ADDR; port += READ_ADDR_STEP)
		{
			/* Check it doesn't clash with anything */
			if(!allocate_resource(IOport_TAG, port, 1, "readport"))
			{
				continue;
			}
			deallocate_resource(IOport_TAG, port, 1);
			/* Make sure all cards are in Wait for key */
			CONFIGCONTROL;
			WRITE_DATA(CONFIG_WAIT_FOR_KEY);
			/* Make them listen */
			delaynus(2000L);
			sendkey();
			/* Reset the cards */
			CONFIGCONTROL;
			WRITE_DATA(CONFIG_RESET_CSN);
			if(do_fullreset)
			{
				CONFIGCONTROL;
				WRITE_DATA(CONFIG_RESET);
				delaynus(2000L);
			}
			snprintf(progress_report_buf,
				 PROGRESS_REPORT_BUFFER_SIZE,
				 "# Trying port address %04x\n", port);
			st_progress_report_callback(progress_report_buf);
			if (do_isolate())
				break;
		}
		if (port > MAX_READ_ADDR)
		{
			st_progress_report_callback("# No boards found\n");
			return(-1);
		}
		snprintf(progress_report_buf,
			 PROGRESS_REPORT_BUFFER_SIZE,
			 "# Board %d has serial identifier", boards_found);
		st_progress_report_callback(progress_report_buf);
		for (i = IDENT_LEN; i--;) {
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   " %02x", serial_identifier[boards_found][i]);
		  st_progress_report_callback(progress_report_buf);
		}
		st_progress_report_callback("\n");
		while (do_isolate())
		{
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   "# Board %d has serial identifier", boards_found);
		  st_progress_report_callback(progress_report_buf);
		  for (i = IDENT_LEN; i--;) {
		    snprintf(progress_report_buf,
			     PROGRESS_REPORT_BUFFER_SIZE,
			     " %02x", serial_identifier[boards_found][i]);
		    st_progress_report_callback(progress_report_buf);
		  }
		  st_progress_report_callback("\n");
		}
		snprintf(progress_report_buf,
			 PROGRESS_REPORT_BUFFER_SIZE,
			 "\n# (DEBUG)\n(READPORT 0x%04x)\n", port);
		st_progress_report_callback(progress_report_buf);
		if(do_ignorecsum)
			st_progress_report_callback("(IGNORECRC)\n");
		if(do_fullreset)
			st_progress_report_callback("(ISOLATE CLEAR)\n");
		else
			st_progress_report_callback("(ISOLATE PRESERVE)\n");
		st_progress_report_callback("(IDENTIFY *)\n");
	}
	else
	{
		CONFIGCONTROL;
		WRITE_DATA(CONFIG_WAIT_FOR_KEY);
		sendkey();
		read_idents();
		snprintf(progress_report_buf,
			 PROGRESS_REPORT_BUFFER_SIZE,
			 "\n# (DEBUG)\n(READPORT 0x%04x)\n(CSN %d)\n(IDENTIFY *)\n",
			 read_port, boards_found);
		st_progress_report_callback(progress_report_buf);
	}
	snprintf(progress_report_buf,
		 PROGRESS_REPORT_BUFFER_SIZE,
		 "(VERBOSITY 2)\n(CONFLICT (IO FATAL)(IRQ FATAL)(DMA FATAL)(MEM FATAL)) # or WARNING\n\n");
	st_progress_report_callback(progress_report_buf);

	return(0);  /* Normal return */
}

void
read_idents(void)
{
	int csn = 0;
	int i;
	for (csn = 1; csn <= boards_found; csn++)
	{
		Wake(csn);
		for (i = 0; i < IDENT_LEN; i++)
		{
			if (statuswait())
				return;
			serial_identifier[csn][i] = RESOURCEDATA;
		}
		snprintf(progress_report_buf,
			 PROGRESS_REPORT_BUFFER_SIZE,
			 "# Board %d has serial identifier", csn);
		st_progress_report_callback(progress_report_buf);
		for (i = IDENT_LEN; i--;) {
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   " %02x", serial_identifier[csn][i]);
		  st_progress_report_callback(progress_report_buf);
		}
		st_progress_report_callback("\n");
		if (serial_identifier[csn][0] == 0xff)
			boards_found = csn - 1;
	}
}

int
do_isolate(void)
{
	unsigned char c1, c2;
	int i;
	int index;
	int newbit;
	int goodaddress = 0;
	if (boards_found >= NUM_CARDS)
	{
	  st_non_fatal_error_callback(0, ISAPNP_E_ACCESS_TOO_MANY_BOARDS);
	  return 0;
	}
	csum = 0x6a;
	/* Assume we will find one */
	boards_found++;
	Wake(0);
	SetRdPort(port);
	delaynus(1000L);
	SERIALISOLATION;
	delaynus(2000L);
	for (index = 0; index < IDENT_LEN - 1; index++)
	{
		for (i = 0; i < 8; i++)
		{
			newbit = 0x00;
			delaynus(250L);
			c1 = READ_DATA;
			delaynus(250L);
			c2 = READ_DATA;
			if (c1 == 0x55)
			{
				if (c2 == 0xAA)
				{
					goodaddress = 1;
					newbit = 0x80;
				}
				else
				{
					goodaddress = 0; 
				}
			}
			if(debug_isolate) {
			  snprintf(progress_report_buf,
				   PROGRESS_REPORT_BUFFER_SIZE,
				   "\n# %02x %02x - bit %02x, goodaddress %d",c1,c2, newbit, goodaddress);
			  st_progress_report_callback(progress_report_buf);
			}
			serial_identifier[boards_found][index] >>= 1;
			serial_identifier[boards_found][index] |= newbit;
			/* Update checksum */
			if (((csum >> 1) ^ csum) & 1)
				newbit ^= 0x80;
			csum >>= 1;
			csum |= newbit;
		}
		if(debug_isolate) {
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   " *** %02x \n", serial_identifier[boards_found][index]);
		  st_progress_report_callback(progress_report_buf);
		}
	}
	if(debug_isolate) {
	  snprintf(progress_report_buf,
		   PROGRESS_REPORT_BUFFER_SIZE,
		   "# computed csum is %02x", csum);
	  st_progress_report_callback(progress_report_buf);
	}
	for (i = 0; i < 8; i++)
	{
		newbit = 0x00;
		delaynus(250L);
		c1 = READ_DATA;
		delaynus(250L);
		c2 = READ_DATA;
		if (c1 == 0x55)
		{
			if (c2 == 0xAA)
			{
				goodaddress = 1;
				newbit = 0x80;
			}
		}
		if(debug_isolate) {
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   "\n# %02x %02x - bit %02x, goodaddress %d",c1,c2, newbit, goodaddress);
		  st_progress_report_callback(progress_report_buf);
		}
		serial_identifier[boards_found][index] >>= 1;
		serial_identifier[boards_found][index] |= newbit;
	}
	if(debug_isolate) {
	  snprintf(progress_report_buf,
		   PROGRESS_REPORT_BUFFER_SIZE,
		   " *** csum *** %02x\n", serial_identifier[boards_found][index]);
	  st_progress_report_callback(progress_report_buf);
	}
	if (goodaddress && (do_ignorecsum || (csum == serial_identifier[boards_found][index])))
	{
		CARDSELECTNUMBER;
		WRITE_DATA(boards_found);
		if((csumdiff[boards_found] = (csum - serial_identifier[boards_found][index]))) {
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   "# WARNING: serial identifier mismatch for board %d: expected 0x%02X, got 0x%02X\n", boards_found, serial_identifier[boards_found][index], csum);
		  st_progress_report_callback(progress_report_buf);
		}
		return (1);
	}
	/* We didn't find one */
	boards_found--;
	return (0);
}

void 
unread_resource_data(char prev)
{
	csum = (csum - prev) & 0xFF;
	backspaced = 1;
	backspaced_char = prev;
}

/* returns 0 on success, nonzero on failure. */

static inline int
read_resource_data(unsigned char *result)
{
	if (backspaced)
	{
		*result = backspaced_char;
		backspaced = 0;
		return 0;
	}
	if (statuswait())
		return 1;
	*result = RESOURCEDATA;
	csum = (csum + *result) & 0xFF;
	return 0;
}

static int
read_one_resource(struct resource *res)
{
	int i;
	if (read_resource_data(&res->tag) != 0)
		return 1;
	if (res->tag & 0x80)
	{
		/* Large item */
		res->type = res->tag;
		for (i = 1; i <= 2; i++)
		{
			if (read_resource_data(&st_tmp[i]) != 0)
				return 1;
		}
		res->len = (st_tmp[2] << 8) | st_tmp[1];
	}
	else
	{
		/* Small item */
		res->type = (res->tag >> 3) & 0x0f;
		res->len = res->tag & 7;
	}
	if (res->len < 1)
		res->data = NULL;
	else
	if ((res->data = malloc(res->len * sizeof(unsigned char))) == NULL)
	{
	  st_fatal_error_callback(ISAPNP_E_ACCESS_MALLOC_FAILURE);
	  /* fatal_error() is not expected to return. */
	  exit(1);
	}
	for (i = 0; i < res->len; i++)
	{
		if (read_resource_data(&res->data[i]) != 0)
			return 1;
	}

#ifdef TAG_DEBUG
	snprintf(progress_report_buf,
		 PROGRESS_REPORT_BUFFER_SIZE,
		 "# %s %02X Tag %02X[%02X] :",
		 (res->tag & 0x80) ? "L" : "S", res->tag, res->type, res->len);
	st_progress_report_callback(progress_report_buf);
	for (i = 0; i < res->len; i++)
	{
	  snprintf(progress_report_buf,
		   PROGRESS_REPORT_BUFFER_SIZE,
		   " %02x", res->data[i]);
	  st_progress_report_callback(progress_report_buf);
	}
	st_progress_report_callback("\n");
#endif
	return 0;
}

static void
fill_in_resource(struct resource *res)
{
	res->mask = ~0;
	res->start = 0;				/* Set some defaults */
	res->size = 1;
	res->step = 1;
	res->errflags = 0;
	switch (res->type)
	{
	case IRQ_TAG:
		res->end = 16;
		res->mask = res->data[0] | (res->data[1] << 8);
		if(!res->mask)
		{
			res->errflags |= RES_ERR_NO_IRQ;
			res->mask |= 1; /* IRQ0 ! */
		}
		if(res->mask & 4)
		{
			res->errflags |= RES_ERR_IRQ2;
			res->mask |= (1 << 9);
			res->mask &= ~4;
		}
		break;
	case DMA_TAG:
		res->end = 8;
		res->mask = res->data[0];
		break;
	case IOport_TAG:
		res->start = (res->data[2] << 8) | res->data[1];
		res->end = ((res->data[4] << 8) | res->data[3]) + 1;
		res->step = res->data[5];
		res->size = res->data[6];
		if(!res->step)
		{
			res->errflags |= RES_ERR_NO_STEP;
			res->step = 1;
		}
		break;
	case FixedIO_TAG:
		res->start = (res->data[1] << 8) | res->data[0];
		res->size = res->data[2];
		res->end = res->start + 1;
		break;
	case MemRange_TAG:
		res->start = (((long) res->data[2] << 8) | res->data[1]) * 256;
		res->end = (((long) res->data[4] << 8) | res->data[3]) * 256 + 1;
		res->step = (res->data[6] << 8) | res->data[5];
		res->size = (((long) res->data[8] << 8) | res->data[7]) * 256;
		break;
	case Mem32Range_TAG:
		res->start = res->end = 0;
		/* STUB */
		break;
	case FixedMem32Range_TAG:
		res->start = res->end = 0;
		/* STUB */
		break;
	default:
		break;
	}
	/* Set initial value. */
	for (res->value = res->start; res->value < res->end; res->value += res->step)
	{
		if ((1 << (res->value & 0x1F)) & res->mask)
			break;
	}
}

static void
read_alternative(struct alternative *alt, struct resource *start_element)
{
	ASSERT(start_element->type == StartDep_TAG);
	switch (start_element->len)
	{
	case 0:
		alt->priority = 1;
		break;					/* 1 == "acceptable" */
	case 1:
		alt->priority = start_element->data[0];
		break;
	default:
	  snprintf(progress_report_buf,
		   PROGRESS_REPORT_BUFFER_SIZE,
		   "read_alternative: Illegal StartDep_TAG length %d.\n", start_element->len);
	  st_non_fatal_error_callback(0, ISAPNP_E_PRINT_PROGRESS_REPORT_BUF);
	  /* The above will print to stderr without being fatal. */
	  break;
	}
	alt->resources = NULL;
	alt->len = 0;
	read_board_resources(start_element, 1, &alt->resources, &alt->len);
	*start_element = alt->resources[alt->len - 1];
	(alt->len)--;
}

void
read_board_resources(struct resource *first_element,
					 int dependent_clause,
					 struct resource **result_array_ptr,
					 int *count)
{
	struct resource *result_array;
	int elts;

	result_array = *result_array_ptr;
	elts = *count;
	if (!result_array)
	{
		result_array = malloc(sizeof(struct resource));
		*count = elts = 0;
	}
	if (!result_array)
	{
	  st_fatal_error_callback(ISAPNP_E_ACCESS_MALLOC_FAILURE2);
	  /* is not expected to return */
	  exit(1);
	}
	if (first_element != NULL)
	{
		elts++;
		if ((result_array =
			 realloc(result_array, elts * sizeof(struct resource))) == NULL)
		{
		  st_fatal_error_callback(ISAPNP_E_ACCESS_REALLOC_FAILURE);
		  /* is not expected to return */
		  exit(1);
		}
		result_array[elts - 1] = *first_element;
	}
	do
	{
		elts++;
		result_array = realloc(result_array, elts * sizeof(struct resource));
		if (!result_array)
		{
		  st_fatal_error_callback(ISAPNP_E_ACCESS_REALLOC_FAILURE);
		  /* is not expected to return */
		  exit(1);
		}
		memset(st_tmp, 0, sizeof(st_tmp));
		read_one_resource(&result_array[elts - 1]);
		if (result_array[elts - 1].type == StartDep_TAG && !dependent_clause)
		{
			int num_alts = 1;
			struct alternative *alts;
			alts = malloc(sizeof(struct alternative));
			/* !!! overlooked here memory allocation failure !!! */
			do
			{
				if (num_alts != 1)
				{
					alts = realloc(alts, num_alts * sizeof(struct alternative));
				}
				if (!alts)
				{
				  st_fatal_error_callback(ISAPNP_E_ACCESS_REALLOC_FAILURE2);
				  /* is not expected to return */
				  exit(1);
				}
				read_alternative(&alts[num_alts - 1], &result_array[elts - 1]);
				num_alts++;
			}
			while (result_array[elts - 1].type == StartDep_TAG);
			result_array[elts - 1].start = 0;
			result_array[elts - 1].end = num_alts - 1;
			result_array[elts - 1].mask = ~0;
			result_array[elts - 1].step = 1;
			result_array[elts - 1].alternatives = alts;
		}
		else
		{
			fill_in_resource(&result_array[elts - 1]);
		}
	}
	while (result_array[elts - 1].type != End_TAG &&
		   (!dependent_clause ||
			(result_array[elts - 1].type != EndDep_TAG &&
			 result_array[elts - 1].type != StartDep_TAG)));
	*count = elts;
	*result_array_ptr = result_array;
}

void 
for_each_resource(struct resource *res,
				  int num_resources,
				  resource_handler handler,
				  FILE * output_file,
				  int selected)
{
	int i;
	for (i = 0; i < num_resources; i++)
	{
		(*handler) (output_file, &res[i], selected);
	}
}

void
read_board(int csn, struct resource **res_list, int *list_len)
{
	int i;
	struct resource *res_array;
	int array_len;

	/* Ensure that callbacks have been initialized */
	if (NULL == st_fatal_error_callback) {
	  fprintf(stderr,
		  "Callbacks were not initialized - aborting application\n");
	  exit(1);
	}

	res_array = *res_list;
	array_len = *list_len;
	if (res_array == NULL || array_len == 0)
	{
		res_array = malloc(sizeof(struct resource));
		array_len = 1;
	}
	else
	{
		array_len++;
		res_array = realloc(res_array,
							array_len * sizeof(struct resource));
	}
	if (!res_array)
	{
	  st_fatal_error_callback(ISAPNP_E_ACCESS_REALLOC_FAILURE3);
	  /* is not expected to return */
	  exit(1);
	}

	res_array[array_len - 1].type = NewBoard_PSEUDOTAG;
	res_array[array_len - 1].start = csn;
	res_array[array_len - 1].end = csn + 1;
	if ((res_array[array_len - 1].data = malloc(IDENT_LEN)) == NULL)
	{
	  st_fatal_error_callback(ISAPNP_E_ACCESS_MALLOC_FAILURE3);
	  /* is not expected to return */
	  exit(1);
	}

	Wake(csn);
	/*
	 * Check for broken cards that don't reset their resource pointer
	 * properly
	 */
	/* Get the first byte */
	if (read_resource_data(&res_array[array_len - 1].data[0]) != 0)
		return;
	/* Just check the first byte, if these match, or the serial identifier had a checksum error, we assume it's ok */
	if((!csumdiff[csn])&&(res_array[array_len - 1].data[0] != serial_identifier[csn][0]))
	{
		/*
		 * Assume the card is broken and this is the start of the resource
		 * data.
		 */
		unread_resource_data(res_array[array_len - 1].data[0]);
		res_array[array_len - 1].len = 1;
		goto broken;			/* Wouldn't need this if people read the spec */
	}
	res_array[array_len - 1].len = IDENT_LEN;
	/*
	 * Read resource data past serial identifier - As it should be spec
	 * section 4.5 para 2
	 */
	for (i = 1; i < IDENT_LEN; i++)
	{
		if (read_resource_data(&res_array[array_len - 1].data[i]) != 0)
			return;
	}
	if(csumdiff[csn])
	{
		/* Original serial identifier had checksum error, so make the serial
		 * identifier what we read from the resource data, as that's what
		 * isapnp will see from an (identify *), assuming checksum ok
		 */
		int data;
		int newbit;
		int j;
		csum = 0x6a;
		for (i = 0; i < IDENT_LEN - 1; i++)
		{
			data = res_array[array_len - 1].data[i];
			for(j = 0; j < 8; j++)
			{
				newbit = (data & 1) << 7;
				data >>= 1;
				if(((csum >> 1) ^ csum) & 1)
					newbit ^= 0x80;
				csum >>= 1;
				csum |= newbit;
			}
		}
		if(csum == res_array[array_len - 1].data[IDENT_LEN - 1])
		{
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   "# WARNING: Making Board %d serial identifier the resource data version\n", csn);
		  st_progress_report_callback(progress_report_buf);
		  for (i = 0; i < IDENT_LEN; i++) {
		    serial_identifier[csn][i] = res_array[array_len - 1].data[i];
		  }
		}
		else {
		  snprintf(progress_report_buf,
			   PROGRESS_REPORT_BUFFER_SIZE,
			   "# WARNING: Board %d resource data identifier has checksum error too\n", csn);
		  st_progress_report_callback(progress_report_buf);
		}
	}
	/* Now for the actual resource data */
	csum = 0;
  broken:
	*res_list = res_array;
	*list_len = array_len;
	read_board_resources(NULL, 0, res_list, list_len);
}

/****************************************************************************/
/* End of res-access.c */

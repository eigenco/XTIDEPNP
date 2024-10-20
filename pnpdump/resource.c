/* Copyright 1998 Yggdrasil Computing, Inc.
   Written by Adam J. Richter

   This file may be copied according to the terms and conditions of the
   GNU General Public License as published by the Free Software
   Foundation.

   other patches applied from Gilles Frattini <gilles@cyberdeck.net>

   isapnp.gone handling added to alloc_system_resources() by P.Fox

   alloc_in_range_list() reordered to handle source name by P.Fox
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static char Id[] __attribute__((unused)) = "$Id: resource.c,v 0.10 2001/04/30 21:50:29 fox Exp $";

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef ENABLE_PCI
#include <linux/pci.h>
#endif

#include <isapnp/resource.h>
#include <isapnp/pnp.h>
#ifdef HAVE_SFN
#include <isapnp/callback.h>
#else
#include <isapnp/callbacks.h>
#endif
#include <isapnp/errenum.h>

#define HAVE_SNPRINTF
#ifndef HAVE_SNPRINTF
#include <isapnp/mysnprtf.h>
#endif

#ifndef GONEFILE
#define GONEFILE "/etc/isapnp.gone" /* File with unuseable resources */
#endif

#define ASSERT(x)	/* as nothing for now */

char * conflict_source = 0; /* Resource which clashed last */

/* struct range_list declaration was moved to resource.h */

int
_print_list(struct range_list * rl) {
	if (rl==NULL) return 0;

	snprintf(progress_report_buf,
		 PROGRESS_REPORT_BUFFER_SIZE,
		 "# reserved %ld-%ld (%s)\n",rl->start,rl->end,rl->source);
	st_progress_report_callback(progress_report_buf);

	return _print_list(rl->next);
}

static int
alloc_in_range_list (unsigned long start, unsigned long end, struct range_list **list, char *source)
{

  	struct range_list *new;
  	struct range_list *prev; /* Remember last entry in the list */
  	struct range_list *curr = *list; /* get a **copy** of the list pointer! */

  	if (start == end) {
    	return 1;	/* adding empty range. */
  	}

  	/* Ensure that callbacks have been initialized */
  	if (NULL == st_fatal_error_callback) {
    	fprintf(stderr, "Callbacks were not initialized - aborting application\n");
    	exit(1);
  	}

    ASSERT(start < end);
	/* Run up the list until the new start is at the list entry end, or before the list entry start */
	prev = NULL;
    while (curr && (start > curr->end)) {
		prev = curr;
        curr = curr->next;
    }

	/* We either ran off the end or we found an entry that should follow the new one */
    ASSERT(curr == NULL || start <= curr->end);

	if(curr)
	{
		/* Test and deal with overlaps first */
		if(((start >= curr->start) && (start < curr->end)) /* IO port is inside existing range */
		   ||((start == curr->end) && curr->next && (curr->next->start < end)) /* IO port overlaps following entry */
		   ||((start < curr->start) && (end > curr->start))) /* IO port overlaps the current entry */
		{
			if((start == curr->end) && curr->next && (curr->next->start < end)) /* Repeat following entry test Tsk! */
				conflict_source = curr->next->source; /* And select the right resource */
			else
				conflict_source = curr->source; /* Otherwise we have the failing entry */
			return 0;
		}

		/* Ok, the new one fits in */
		ASSERT(end >= curr->start); /* bad assertion */
		ASSERT(start <= curr->end); /* bad assertion */
		/* Test for contiguous regions */
		/* First new end matches source start ==> gap before new start */
		/* If two adjacent entries have the same source, they can be coalesced */
		if ((end == curr->start) && (source == curr->source)) {
			/* We are contiguous with next element, and have the same source */
			curr->start = start;
			return 1;
		}

		/* If new start matches end, need to test the new end against the next region start */
		if (start == curr->end) /* contiguous */
		{
			if (curr->next && (curr->next->start == end)) /* and contiguous again */
			{
				/* we exactly bridge to subsequent element. */
				if(source == curr->source) /* and sources match */
				{
					if(source == curr->next->source) /* and the next one too! */
					{
						/* If both sources are the same, can merge */
						struct range_list *obselete = curr->next;
						curr->end = obselete->end;
						curr->next = obselete->next;
						free(obselete);
						return 1;
					}

					/* next element is contiguous with us and the subsequent element has a different source. */
					curr->end = end;
					return 1;

				}

				if(source == curr->next->source)
				{
					/* next element is contiguous with us and the current element has a different source. */
					curr->next->start = start;
					return 1;
				}

				/* Both have different sources, so drop out to put in new element */
			}
			else if(source == curr->source)
			{
				/* next element is contiguous with us and we do not reach subsequent element. */
				curr->end = end;
				return 1;
			}

			prev = curr;
			curr = curr->next;
		}

	}

	/* There is a gap between us and next element, or no next element, or the sources are different. */

	new = malloc(sizeof(struct range_list));
	if (!new) {
	  st_fatal_error_callback(ISAPNP_E_RES_MALLOC_FAILURE);
	  /* is not expected to return */
	  exit(1);
	}

	new->start 	= start;
	new->end 	= end;
	new->source = source;

	/* If we have a predecessor, tag us on */

	if(prev)
		prev->next = new;
	else
		*list = new;

	/* And our successor is what it is */

	new->next = curr;

	return 1;

}

static void
dealloc_in_range_list (unsigned long start, unsigned long end,
					   struct range_list **list) {
    if (start == end) return;	/* deleting empty range. */
    ASSERT(start < end);
    while (*list && (start >= (*list)->end)) {
        list = &(*list)->next;
    }
    while (*list && end > (*list)->start) {
		/* we overlap with this element */
        ASSERT(start < (*list)->end);
		if((*list)->end > end) {
			(*list)->start = end;
			return;
		} else {
			struct range_list *tmp = *list;
			*list = (*list)->next;
			free(tmp);
		}
    }
}

struct range_list *tag_range_lists[] = {NULL, NULL, NULL, NULL};

int
tag_to_index (unsigned char tag) {
    switch (tag) {
	case IRQ_TAG: return 0;
	case DMA_TAG: return 1;
	case IOport_TAG:
	case FixedIO_TAG:
	    return 2;
	case MemRange_TAG:
	case Mem32Range_TAG:
	case FixedMem32Range_TAG:
	    return 3;
	default:		
	    return -1;		/* Unrecognized tag */
    }
}

int print_irq() {
	st_progress_report_callback("# IRQ\n");
	return _print_list(tag_range_lists[0]);
}

int
tag_allocable (unsigned char tag) {
    return tag_to_index(tag) != -1;
}

int
allocate_resource(char tag, unsigned long start, unsigned long len, char *source) {
    return alloc_in_range_list(start, start + len,
							   &tag_range_lists[tag_to_index(tag)], source);
}

void
deallocate_resource(char tag, unsigned long start, unsigned long len) {
    dealloc_in_range_list(start, start + len,
						  &tag_range_lists[tag_to_index(tag)]);
}

#define INBUFSIZ 256 /* Be reasonable */

#ifdef HAVE_PROC
#ifdef ENABLE_PCI
int allocate_pci_resources_old( void )
{
	char buf[80];
	int next=0;
	int val;
	
   FILE *fp = fopen( "/proc/pci", "rt" );
   if( !fp )
   {
      return 0;
   }

   while (fscanf(fp,"%s ",buf)==1) {
	   if (next==IRQ_TAG) {
		   if (sscanf(buf,"%d",&val)==1)  
			   allocate_resource( IRQ_TAG, val, 1, "pci" );
		   next=0;
	   }
	   if (strncmp(buf,"IRQ",3)==0) next=IRQ_TAG;
   }
   fclose(fp);
   return 1;
}

void allocate_pci_resources( void )
{
   char *line = 0;
   int lineMax = 0;

   FILE *fp = fopen( "/proc/bus/pci/devices", "rt" );
   if( !fp )
   {
	  if(!allocate_pci_resources_old())
	  {
	    st_non_fatal_error_callback(0, ISAPNP_E_RES_CONFLICT_NOT_CHECKED);
	  }
      return;
   }
   while( getdelim( &line, &lineMax, '\n', fp ) > 0 )
   {
      int i;
      unsigned long val;
      char *seg, *end;

      if( !strtok( line, " \t\n" ) )
         continue;

      strtok( 0, " \t\n" );
      seg = strtok( 0, " \t\n" );
      if( !seg )
         continue;

      val = strtoul( seg, &end, 16 );
      if( *end == 0 )
         allocate_resource( IRQ_TAG, val, 1, "pci" );

      for( i = 0; i < 7; i++ )
      {
         seg = strtok( 0, " \t\n" );
         if( !seg )
            break;
         val = strtoul( seg, &end, 16 );
         if( *end )
            continue;

         if( (val & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY )
         {
            val = val & PCI_BASE_ADDRESS_MEM_MASK;
            if( val )
               allocate_resource( MemRange_TAG, val, 1, "pci" );
         }else{
            val = val & PCI_BASE_ADDRESS_IO_MASK;
            if( val )
               allocate_resource( IOport_TAG, val, 1, "pci" );
         }
      }
   }

   if( line )
      free( line );
   fclose( fp );
   return;
}
#endif /* ENABLE_PCI */
#endif /* HAVE_PROC */

void
alloc_system_resources(void) {
    FILE *input;
	static char inbuf[INBUFSIZ];
    int interrupt_num, dma, io_start, mem_start;

#ifdef HAVE_PROC
	int io_end;
    /* Avoid allocating DMA channels used by other devices in /proc. */
    if ((input = fopen("/proc/interrupts", "r")) != NULL) {
      fscanf(input, "%*[^\n]\n"); /* skip first line */
      while (fscanf (input, "%d%*[^\n]\n", &interrupt_num) == 1) {
#if 0
		  snprintf(progress_report_buf,
				   PROGRESS_REPORT_BUFFER_SIZE,
				   "IRQ %d in use\n", interrupt_num);
		  st_non_fatal_error_callback(0, ISAPNP_E_PRINT_PROGRESS_REPORT_BUF);
#endif
		  (void) allocate_resource(IRQ_TAG, interrupt_num, 1, "/proc/interrupts");
      }
      fclose(input);
    }
    if ((input = fopen("/proc/dma", "r")) != NULL) {
        while (fscanf (input, "%d%*[^\n]\n", &dma) == 1) {
			(void) allocate_resource(DMA_TAG, dma, 1, "/proc/dma");
		}
        fclose(input);
    }
    if ((input = fopen("/proc/ioports", "r")) != NULL) {
        while (fscanf (input, "%x-%x%*[^\n]\n", &io_start, &io_end) == 2) {
			/* /proc/ioports addresses are inclusive, hence + 1 below */
			(void) allocate_resource(IOport_TAG, io_start, io_end - io_start + 1, "/proc/ioports");
		}
        fclose(input);
    }

#ifdef ENABLE_PCI
    allocate_pci_resources();
#endif /* ENABLE_PCI */
#endif /* HAVE_PROC */

	/* Now read in manual stuff from isapnp.gone file */
    if ((input = fopen(GONEFILE, "r")) != NULL)
	{
		int line = 0;
		/*
		 * File format - 
		 *
		 * Leading whitespace removed, then..
		 * Blank lines ignored
		 * # in column 1 is comment
		 * 
		 * One entry per line, after entry is rubbish and ignored
		 *
		 * IO base [, size] - default size 8
		 * IRQ n
		 * DMA n
		 * MEM base, size
		 *
		 * Numbers may be decimal or hex (preceded by 0x)
		 *
		 * Anything else is rubbish and complained about noisily
		 */
#if 0
		printf("about to call snprintf: Reading %s\n", GONEFILE);
		snprintf(progress_report_buf,
				 PROGRESS_REPORT_BUFFER_SIZE,
				 "Reading %s\n", GONEFILE);
		st_non_fatal_error_callback(0, ISAPNP_E_PRINT_PROGRESS_REPORT_BUF);
#endif
        while (fgets(inbuf, INBUFSIZ, input))
		{
			unsigned long n;
			int size;
			char *inptr = inbuf;
			line++;
			while(isspace(*inptr))
				inptr++;
			if((*inptr == '#')||(*inptr == 0))
				continue;
			if(toupper(*inptr) == 'I')
			{
				/* IO or IRQ */
				inptr++;
				if((toupper(*inptr) == 'O')&&(isspace(inptr[1])))
				{
					/* IO */
					int size = 8;
					inptr += 2;
					n = strtoul(inptr, &inptr, 0);
					io_start = (int)n;
					while(isspace(*inptr))
						inptr++;
					/* Check for optional size */
					if(*inptr == ',')
					{
						inptr++;
						n = strtoul(inptr, &inptr, 0);
						if(!n)
							goto garbage;
						size = n;
					}
					(void) allocate_resource(IOport_TAG, io_start, size, GONEFILE);
					continue;
				}
				if((toupper(*inptr) == 'R')&&(toupper(inptr[1]) == 'Q')&&(isspace(inptr[2])))
				{
					/* IRQ */
					inptr += 3;
					n = strtoul(inptr, &inptr, 0);
					interrupt_num = (int)n;
					(void) allocate_resource(IRQ_TAG, interrupt_num, 1, GONEFILE);
					continue;
				}
				goto garbage;
			}
			else if(toupper(*inptr) == 'D')
			{
				/* DMA */
				inptr++;
				if((toupper(*inptr) == 'M')&&(toupper(inptr[1]) == 'A')&&(isspace(inptr[2])))
				{
					inptr += 3;
					n = strtoul(inptr, &inptr, 0);
					dma = (int)n;
					(void) allocate_resource(DMA_TAG, dma, 1, GONEFILE);
					continue;
				}
				goto garbage;
			}
			else if(toupper(*inptr) == 'M')
			{
				/* MEM */
				inptr++;
				if((toupper(*inptr) == 'E')&&(toupper(inptr[1]) == 'M')&&(isspace(inptr[2])))
				{
					/* MEM */
					inptr += 3;
					n = strtoul(inptr, &inptr, 0);
					mem_start = (int)n;
					while(isspace(*inptr))
						inptr++;
					/* Check for optional size */
					if(*inptr != ',')
						goto garbage;
					inptr++;
					n = strtoul(inptr, &inptr, 0);
					if(!n)
						goto garbage;
					size = n;
					(void) allocate_resource(MemRange_TAG, mem_start, size, GONEFILE);
					continue;
				}
				goto garbage;
			}
			else
			{
			garbage:
			  snprintf(progress_report_buf,
				   PROGRESS_REPORT_BUFFER_SIZE,
				   "Garbage in %s file on line %d: %s", GONEFILE, line, inbuf);
			  st_non_fatal_error_callback(0, ISAPNP_E_PRINT_PROGRESS_REPORT_BUF);
			}
		}
        fclose(input);
    }
}

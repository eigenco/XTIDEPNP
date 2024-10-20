/* Copyright 1998 Yggdrasil Computing, Inc.
   Written by Adam J. Richter

   This file may be copied according to the terms and conditions of the
   GNU General Public License as published by the Free Software
   Foundation.

   $Id: resource.h,v 0.3 1999/12/02 22:42:38 fox Exp $

   Added errflags and associated #defines 15-Oct-99 - P.Fox.
*/
#ifndef RESOURCE_H
#define RESOURCE_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/

#define DEPENDENCY_PSEUDOTAG	-2

struct resource {
    unsigned int type;		/* IRQ_TAG, DMA_TAG, IOport_TAG, StartDep_TAG,
				   MemRange_TAG, Mem32Range_TAG,
				   FixedMem32Range_TAG */
				/* If this is a parent node of a number
				   of dependent resources, the type of
				   this node is EndDep_TAG.  The
				   StartDep_TAG's are the first resource
				   of each child alternative
                                   (i.e., alterantives[i].resources[0]).*/
    unsigned char tag;		/* The actual tag from which type and possibly
				   len were derived.  Used for debugging. */
    int len;
    unsigned char *data;

    unsigned long value;
    unsigned long start, end, step, size; /* IO or memory  */
    /* For other tags: start = 0, end = num_alternatives-1, step = 1, size = 1 */
    unsigned long mask;		    /* DMA or IRQ, ~0 for all others */
    unsigned int errflags;      /* Flag errors found for later display */
    struct alternative *alternatives; /* StartDep_TAG */
};

/* Bit mask of resource data errors */
#define RES_ERR_NO_IRQ  (1)
#define RES_ERR_IRQ2    (2)
#define RES_ERR_NO_STEP (4)

struct alternative {
    int priority;
    int len;
    struct resource *resources;
};


extern int
allocate_resource(char tag, unsigned long start, unsigned long len, char *source);
				/* Returns 1 on successful reservation. */

void
deallocate_resource(char tag, unsigned long start, unsigned long len);

int
alloc_resources ( struct resource *res, int count,
		  struct resource *parent_res, int parent_count);

int tag_allocable (unsigned char tag);

void alloc_system_resources(void);

extern char * conflict_source;

/*****************************************************************************
** Declarations and procedures to enable access to tag_range_lists[] from   **
** other modules                                                            **
*****************************************************************************/

struct range_list {
  unsigned long start, end;	/* end = start+len */
  struct range_list *next;
  char *source; /* Name of file where resource allocated, the pointer
		** must be the same for the same source and resource type
		*/
};

extern struct range_list *tag_range_lists[];

/* Convert resource tags (such as IRQ_TAG and DMA_TAG) into index
** to the tag_range_lists array.
*/

int
tag_to_index (unsigned char tag);

/****************************************************************************/
#ifdef __cplusplus
}; /* end of extern "C" */
#endif

#endif /* RESOURCE_H */
/* End of resource.h */

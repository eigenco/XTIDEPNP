#define PACKAGE 0
#define VERSION 0
/*****************************************************************************
**
** pnpdump_main.c
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
/* If not in library, use our own version */
#include "getopt.h"
#endif

#include <stdlib.h>
#include <string.h>		/* For strncpy */

/*
 * Define this to enable additional output (show all valid address ranges)
 */
/* #undef DUMPADDR */

/*
 * Define this to just dumpdata
 */
/* #undef DUMPDATA */

#ifndef __DJGPP__
#define SCRIPT_OUTPUT
#define FILE_OUTPUT
#endif

#undef RELEASE_DUMPADDR
#ifdef DUMPADDR
#define RELEASE_DUMPADDR " -DDUMPADDR"
#else
#define RELEASE_DUMPADDR ""
#endif /* !DUMPADDR */

#include <isapnp/iopl.h>
#include <isapnp/pnp.h>
#include <isapnp/resource.h>
#include <isapnp/errenum.h>
#ifdef HAVE_SFN
#include <isapnp/callback.h>
#include <isapnp/res_acce.h>
#else
#include <isapnp/callbacks.h>
#include <isapnp/res-access.h>
#endif
#include <isapnp/release.h>

#ifdef REALTIME
#include <isapnp/realtime.h>
static long l_realtime_timeout = 5L;
#endif

static char rcsid[] = "$Id: pnpdump_main.c,v 1.27 2001/04/30 21:54:53 fox Exp $";

static void print_resource(FILE * output_file,
			   struct resource *res,
			   int selected);


#ifdef SCRIPT_OUTPUT

/* Script stuff */

static int do_script = 0;
static char *output_script = NULL;

static void print_script_header(FILE * script_file);

static void print_script(FILE * output_file,
						 struct resource *res,
						 int selected);

#endif /* SCRIPT_OUTPUT */

#ifdef FILE_OUTPUT

/* Output file stuff */

static int do_output_file = 0;
static char *o_file_name = NULL;

#endif /* FILE_OUTPUT */

static FILE* o_file = NULL;

/* Global state variables used for printing resource information. */
static unsigned long serialno = 0;
static char devid[8];									   /* Copy as devidstr
														    * returns pointer to
														    * static buffer subject
														    * to change */
static int nestdepth;
static int curid;
static int curdma, depdma;
static int curio, depio;
static int curint, depint;
static int curmem, depmem;
static int starteddeps;
static char *indep = "";

/* This shouldn't be required 8-Oct-1998 */
static void
prepare_to_print(void)
{
	nestdepth = 0;
	curid = 0;
	curdma = 0;
	depdma = 0;
	curio = 0;
	depio = 0;
	curint = 0;
	depint = 0;
	curmem = 0;
	depmem = 0;
	starteddeps = 0;
	indep = "";
}

static unsigned char tmp[TMP_LEN];
static char *devstring = 0;

static int showmasks = 0;
static int do_autoconfig = 0;

#ifdef DUMPDATA
static void dumpdata(int);
#endif

static void usage(char *program_name);

static int do_dumpregs = 0;
static void dumpregs(int);

static char *devidstr(unsigned char, unsigned char, unsigned char, unsigned char);

/* Non-standard callback */

static void
pnpdump_progress_report_callback(const char *in_msg)
{
  /* Prints the progress report to stdout.
  ** In pnpdump.c it is used to actually write the isapnp.conf
  ** configuration file.
  */
  fprintf(o_file, "%s", in_msg);
}


static void
banner(FILE *fp, char *prefix)
{
	fprintf(fp, 
			"%s%s\n"
			"%sRelease %s\n"
			"%s\n"
			"%sThis is free software, see the sources for details.\n"
			"%sThis software has NO WARRANTY, use at your OWN RISK\n"
			"%s\n"
			"%sFor details of the output file format, see isapnp.conf(5)\n"
			"%s\n"
			"%sFor latest information and FAQ on isapnp and pnpdump see:\n"
			"%shttp://www.roestock.demon.co.uk/isapnptools/\n"
			"%s\n"
			"%sCompiler flags: %s%s\n"
			"%s\n",
			prefix, rcsid,
			prefix, libtoolsver,
			prefix, prefix, prefix, prefix, prefix, prefix, prefix, prefix, prefix,
		    prefix, libcompilerflags, RELEASE_DUMPADDR,
			prefix);
}

/*
 * Single argument is initial readport
 * Two arguments is no-of-boards and the readport to use
 */

int
pnpdump_main(int argc, char **argv)
{
	int i;
	char *program = argv[0];
	struct resource *resources;
	int resource_count;
	int alloc_result;
	int l_ret;

	/* Process command line options */
	const struct option longopts[] =
	{
		{"masks", no_argument, NULL, 'm'},
		{"config", no_argument, NULL, 'c'},
		{"help", no_argument, NULL, 'h'},
#ifdef SCRIPT_OUTPUT
		{"script", optional_argument, NULL, 's'},
#endif
		{"reset", no_argument, NULL, 'r'},
		{"dumpregs", no_argument, NULL, 'd'},
		{"debug-isolate", no_argument, NULL, 'D'},
		{"ignorecsum", no_argument, NULL, 'i'},
#ifdef FILE_OUTPUT
		{"outputfile", required_argument, NULL, 'o'},
#endif
#ifdef REALTIME
		{"max-realtime", required_argument, NULL, 't'},
#endif
		{"version", no_argument, NULL, 'v'},
		{0, 0, 0, 0},
	};
	int opt;

	while ((opt = getopt_long(argc, argv, "cms:rdit:o:vDh", longopts, NULL)) != EOF)
	{
		switch (opt)
		{
		case 'c':
			do_autoconfig = 1;
			break;
		case 'm':
			showmasks = 1;
			break;
		case 'r':
			do_fullreset = 1;
			break;
#ifdef SCRIPT_OUTPUT
		case 's':
			do_script = 1;
			output_script = optarg;
			break;
#else
		case 's':
			fprintf(stderr, "Output scripts support not compiled in - option ignored\n");
			break;
#endif
		case 'd':
			do_dumpregs = 1;
			break;
		case 'D':
			debug_isolate = 1;
			break;
		case 'h':
			usage(program);
			break;
		case 'i':
			do_ignorecsum = 1;
			break;
#ifdef REALTIME
		case 't':
			l_realtime_timeout = atol(optarg);
			break;
#else
		case 't':
			fprintf(stderr, "Realtime support not compiled in - option ignored\n");
			break;
#endif
		case 'v':
			fprintf(stderr, "Version: pnpdump from %s-%s\n", PACKAGE, VERSION);
			exit(0);
			break;
#ifdef FILE_OUTPUT
		case 'o':
			do_output_file = 1;
			o_file_name = optarg;
			break;			
#else
		case 'o':
			fprintf(stderr, "Output file support not compiled in - option ignored\n");
			break;
#endif
		case '?':
#if 1
			fprintf(stderr, "unrecognized option.\n");
#endif
			usage(program);
			return 1;
		case ':':
#if 1
			fprintf(stderr, "missing parameter.\n");
#endif
			usage(program);
			return 1;
		default:
			fprintf(stderr,
					"?? getopt returned character code 0x%x ('%c').\n",
					opt, opt);
			return 1;
		}

	}

	argc -= optind - 1;
	argv += optind - 1;

	/* Initialize callbacks */
	callbacks_init(normal_fatal_error_callback,
				   normal_non_fatal_error_callback,
				   pnpdump_progress_report_callback);

	/* Process arguments */
	if ((argc < 1) || (argc > 3))
	{
		usage(program);
		return 1;
	}

	/* If a number of boards and read_port have been given, don't ISOLATE */
	if (argc == 3)
	{
		boards_found = atoi(argv[1]);	/* This stops ISOLATE */
		if ((boards_found < 0) || (boards_found >= NUM_CARDS))
		{
			fprintf(stderr, "Cannot handle %d boards, recompile with NUM_CARDS bigger\n", boards_found);
			exit(1);
		}
	}
	if (argc > 1)
	{
		/* Read decimal or hex number */
		read_port = (int) strtol(argv[argc - 1], (char **) NULL, 0);
		if ((read_port < MIN_READ_ADDR) || (read_port > MAX_READ_ADDR))
		{
			fprintf(stderr, "Port address %s (0x%04x) out of range 0x203..0x3ff\n", argv[argc - 1], read_port);
			exit(1);
		}
		read_port |= 3;
	}

#ifdef FILE_OUTPUT
	if (do_output_file)
	{
		if (o_file_name != NULL) {
			o_file = fopen(o_file_name, "w");
			if (!o_file)
			{
				perror(o_file_name);
				printf("Output of pnpdump will be streamed to screen.\n");
				o_file = stdout;
			}
		}
	}
#endif /* FILE_OUTPUT */

	if (!o_file)  
		o_file = stdout;
	
	if (acquire_pnp_io_privileges() != 0)
	{
		perror("Unable to get io permission for WRITE_DATA");
		exit(1);
	}
	banner(o_file, "# ");
	/* Have to do this anyway to check readport for clashes */
	alloc_system_resources();
#ifdef REALTIME
	setroundrobin(l_realtime_timeout);
#endif /* REALTIME */
	/* Now get board info */
	l_ret = initiate();
	if ((-1) == l_ret) {
	  exit(0);   /* This is what initiate() originally did when
		     ** it found no ISA PnP boards.
		     */
	}
	resources = NULL;
	resource_count = 0;
	for (i = 1; i <= boards_found; i++)
	{
#ifndef DUMPDATA
		read_board(i, &resources, &resource_count);
#else
		dumpdata(i);
#endif
		if(do_dumpregs)
			dumpregs(i);
	}
#ifdef REALTIME
	/* No longer need real-time scheduling */
	normal_sched();
#endif /* REALTIME */
	if (do_autoconfig)
	{
		alloc_result = alloc_resources(resources, resource_count, NULL, 0);
	}
	else
	{
		alloc_result = 0;
	}
	prepare_to_print();
	for_each_resource(resources, resource_count,
					  print_resource, o_file, alloc_result);
	/* Musn't forget this ! */
	fprintf(o_file, "# Returns all cards to the \"Wait for Key\" state\n");
	fprintf(o_file, "(WAITFORKEY)\n");
	/* Release resources */
	if (relinquish_pnp_io_privileges() != 0) {
	  perror("Unable to release io permission for WRITE_DATA");
	  exit(1);
	}


#ifdef SCRIPT_OUTPUT
	if (do_script)
	{
		FILE *script_file;

		if (output_script != NULL)
		{
			script_file = fopen(output_script, "w");
		}
		else
		{
			script_file = popen("sh", "w");
		}
		if (script_file == NULL)
		{
			perror(output_script ? output_script : "sh");
			exit(1);
		}
		if (alloc_result == 0)
		{
			fprintf(script_file, "#!/bin/sh\n" "isa-pnp-config-failed\n");
		}
		else
		{
			print_script_header(script_file);
			for_each_resource(resources, resource_count,
							  print_script, script_file, 1);
		}
		if (output_script)
		{
			fclose(script_file);
		}
		else
		{
			pclose(script_file);
		}
	}
#endif /* SCRIPT_OUTPUT */

	return (0);
}

static void 
usage(char *program_name)
{
	banner(stderr, "");
	fprintf(stderr,
			"Usage: %s [OPTIONS] [[devs] readport]\n\n"
#ifdef FILE_OUTPUT
			" -o o, --outputfile=o    stream pnpdump output to file o instead of stdout\n"
#endif
			"   -c, --config          uncomment settings to which the devices can be set\n"
			"   -d, --dumpregs        dump standard configuration registers for each board\n"
			"   -D, --debug-isolate   output loads of extra information on isolation process\n"
			"   -h, --help            show a help summary to stderr\n"
			"   -i, --ignorecsum      ignore checksum errors when choosing the readport address\n"
			"   -r, --reset           carry out a full configuration reset\n"
			"   -m, --masks           list acceptable interrupts and DMA channels as bitmasks\n"
#ifdef REALTIME
			" -t t, --max-realtime=t  maximum real-time priority execution is t seconds\n"
#endif
#ifdef SCRIPT_OUTPUT
			" -s scr, --script[=scr]  write a shell script to the specified file\n"
#endif
			"                         no script pipes output to a shell\n"
			"   -v, --version         print the isapnptools version number on stderr\n"
			"\n"
			"   devs      is the number of PnP cards that the BIOS has found\n"
			"   readport  is the address of the readport to use for Plug-And-Play access\n"
			, program_name);
	exit(0);
}

static char *
devidstr(unsigned char d1, unsigned char d2, unsigned char d3, unsigned char d4)
{
	static char resstr[] = "PNP0000";
	sprintf(resstr, "%c%c%c%x%x%x%x", 'A' + (d1 >> 2) - 1, 'A' + (((d1 & 3) << 3) | (d2 >> 5)) - 1,
			'A' + (d2 & 0x1f) - 1, d3 >> 4, d3 & 0x0f, d4 >> 4, d4 & 0x0f);
	return resstr;
}

static void
lengtherror(struct resource *res, char *msg)
{
	int tag = res->tag;
	int len = res->len;
	int i;
	fprintf(o_file, "# Bad tag length for %s in 0x%02x", msg, tag);
	if (tag & 0x80)
	{
		fprintf(o_file, " 0x%02x 0x%02x", len & 255, len >> 8);
	}
	for (i = 0; i < len; i++)
		fprintf(o_file, " 0x%02x", res->data[i]);
	fprintf(o_file, "\n");
}

#ifdef ABORT_ONRESERR
#define BREAKORCONTINUE   goto oncorruptresourcedata
#else
#define BREAKORCONTINUE   break
#endif

static void
showmask(unsigned long mask, FILE* output_file)
{
	int i;
	int firsttime = 1;

	if (showmasks)
	{
		fprintf(output_file, " mask 0x%lx", mask);
	}
	else
	{
		for (i = 0; mask; i++, mask >>= 1)
		{
			if (mask & 0x1)
			{
				if (!firsttime)
				{
					fprintf(output_file, mask == 1 ? " or " : ", ");
				}
				else
				{
					fprintf(output_file, " ");
					firsttime = 0;
				}
				fprintf(output_file, "%d", i);
			}
		}
	}
}


static int IORangeCheck = 0;


static void
print_resource(FILE * output_file, struct resource *res, int selected)
{
	static int logdevno = 0;
	int type = res->type;
	int len = res->len;
	int i;
	char *comment_if_not_selected = selected ? " " : "#";

	switch (type)
	{
	case NewBoard_PSEUDOTAG:
		{
			int csn = res->start;

			logdevno = 0;
			serialno = (unsigned long)
				serial_identifier[csn][4] +
				(serial_identifier[csn][5] << 8) +
				(serial_identifier[csn][6] << 16) +
				(serial_identifier[csn][7] << 24);
			strncpy(devid, devidstr(serial_identifier[csn][0],
									serial_identifier[csn][1],
									serial_identifier[csn][2],
									serial_identifier[csn][3]), 8);

			fprintf(output_file, "# Card %d: (serial identifier", csn);
			for (i = IDENT_LEN; i--;)
				fprintf(output_file, " %02x", serial_identifier[csn][i]);
			fprintf(output_file, ")\n");
			if (serialno == 0xffffffffUL)
			{
				fprintf(output_file, "# Vendor Id %s, No Serial Number (-1), checksum 0x%02X.\n", devid, serial_identifier[csn][8]);
			}
			else
			{
				fprintf(output_file, "# Vendor Id %s, Serial Number %lu, checksum 0x%02X.\n", devid, serialno, serial_identifier[csn][8]);
			}
			if (res->len != IDENT_LEN)
			{
				i = 0;
				fprintf(output_file, "# Ident byte %d, (%02x) differs from resource data (%02x)\n", i, serial_identifier[csn][i], res->data[0]);
				fprintf(output_file, "#Assuming the card is broken and this is the start of the resource data\n");
			}
			else
			{
				for (i = 1; i < IDENT_LEN; i++)
				{
					if ((res->data[i] != serial_identifier[csn][i]) && (i < (IDENT_LEN - 1)))
						fprintf(output_file, "# Ident byte %d, (%02x) differs from resource data (%02x)\n", i, serial_identifier[csn][i], res->data[i]);
				}
			}
		}

		break;
	case PnPVerNo_TAG:
		{
			if (len != 2)
			{
				lengtherror(res, "PnPVerNo_TAG");
				BREAKORCONTINUE;
			}
			fprintf(output_file, "# %sVersion %d.%d, Vendor version %x.%x\n", indep,
				   res->data[0] >> 4,
				   res->data[0] & 0x0f,
				   res->data[1] >> 4,
				   res->data[1] & 0x0f);
			break;
		}
	case LogDevId_TAG:
		{
			int reg;
			static char *regfns[8] =
			{"Device capable of taking part in boot process",
			 "Device supports I/O range check register",
			 "Device supports reserved register @ 0x32",
			 "Device supports reserved register @ 0x33",
			 "Device supports reserved register @ 0x34",
			 "Device supports reserved register @ 0x35",
			 "Device supports reserved register @ 0x36",
			 "Device supports reserved register @ 0x37"};
			if ((len < 5) || (len > 6))
			{
				lengtherror(res, "LogDevId_TAG");
				BREAKORCONTINUE;
			}
			indep = "";
			if (nestdepth)
			{
				/* If we have a device name, show it (the last description string before the END DF flag is it) */
				if (serialno == 0xffffffffUL)
					fprintf(output_file, " (NAME \"%s/-1[%d]", devid, logdevno-1);
				else
					fprintf(output_file, " (NAME \"%s/%lu[%d]", devid, serialno, logdevno-1);
				if(devstring)
					fprintf(output_file, "{%-20s}", devstring);
				fprintf(output_file, "\")\n");
				fprintf(output_file, "%s (ACT Y)\n", comment_if_not_selected);
				while (nestdepth)
				{
					fprintf(output_file, ")");
					nestdepth--;
				}
				fprintf(output_file, "\n");
			}
			fprintf(output_file, "#\n# %sLogical device id %s\n", indep, devidstr(res->data[0], res->data[1], res->data[2], res->data[3]));
			indep = "    ";
			IORangeCheck = 0;
			for (i = 0, reg = 1; reg < 256; i++, reg <<= 1)
			{
				if (res->data[4] & reg)
				{
					fprintf(output_file, "# %s%s\n", indep, regfns[i]);
					if(i == 1)
						IORangeCheck = 1;
				}
			}
			for (i = 0, reg = 1; reg < 256; i++, reg <<= 1)
			{
				if (res->data[5] & reg)
				{
					fprintf(output_file, "# %sDevice supports vendor reserved register @ 0x%02x\n", indep, 0x38 + i);
				}
			}
			fprintf(output_file, "#\n# Edit the entries below to uncomment out the configuration required.\n");
			fprintf(output_file, "# Note that only the first value of any range is given, this may be changed if required\n");
			fprintf(output_file, "# Don't forget to uncomment the activate (ACT Y) when happy\n\n");
			if (serialno == 0xffffffffUL)
				fprintf(output_file, "(CONFIGURE %s/-1 (LD %d\n", devid, logdevno++);
			else
				fprintf(output_file, "(CONFIGURE %s/%lu (LD %d\n", devid, serialno, logdevno++);
			nestdepth = 2;
			curdma = 0;
			depdma = 0;
			curio = 0;
			depio = 0;
			curint = 0;
			depint = 0;
			curmem = 0;
			depmem = 0;
			starteddeps = 0;
			break;
		}
	case CompatDevId_TAG:
		{
			if (len != 4)
			{
				lengtherror(res, "CompatDevId_TAG");
				BREAKORCONTINUE;
			}
			fprintf(output_file, "# %sCompatible device id %s\n", indep, devidstr(res->data[0], res->data[1], res->data[2], res->data[3]));
			break;
		}
	case IRQ_TAG:
		{
			int firstirq = 0;
			char *edge = "+E";
			if ((len < 2) || (len > 3))
			{
				lengtherror(res, "IRQ_TAG");
				BREAKORCONTINUE;
			}
			if ((len >= 2) && (res->data[0] || res->data[1]))
			{
				if(res->errflags & RES_ERR_NO_IRQ)
					fprintf(output_file, "# %s%s*** Bad resource data: No IRQ specified\n", indep, indep);
				if(res->errflags & RES_ERR_IRQ2)
					fprintf(output_file, "# %s%s*** Bad resource data (Clarifications 4.6.2): IRQ 2 invalid, changing to 9\n", indep, indep);
				fprintf(output_file, "# %sIRQ", indep);
				showmask(res->mask, output_file);
				fprintf(output_file, ".\n");
				firstirq = res->value;
				if (len == 3)
				{
					if (res->data[2] & 1)
					{
						fprintf(output_file, "# %s%sHigh true, edge sensitive interrupt\n", indep, indep);
						edge = "+E";
					}
					if (res->data[2] & 2)
					{
						fprintf(output_file, "# %s%sLow true, edge sensitive interrupt\n", indep, indep);
						edge = "-E";
					}
					if (res->data[2] & 4)
					{
						fprintf(output_file, "# %s%sHigh true, level sensitive interrupt\n", indep, indep);
						edge = "+L";
					}
					if (res->data[2] & 8)
					{
						fprintf(output_file, "# %s%sLow true, level sensitive interrupt\n", indep, indep);
						edge = "-L";
					}
				}
				else
				{
					fprintf(output_file, "# %s%sHigh true, edge sensitive interrupt (by default)\n", indep, indep);
				}
				fprintf(output_file, "%s (INT %d (IRQ %ld (MODE %s)))\n",
					   comment_if_not_selected, curint, res->value, edge);
				curint++;
				if (!starteddeps)
					depint = curint;
			}
			else
			{
				fprintf(output_file, "# %s*** ERROR *** No IRQ specified!\n", indep);
			}
			break;
		}
	case DMA_TAG:
		{
			int firstdma = 4;							   /* Ie, no DMA */
			if (len != 2)
			{
				lengtherror(res, "DMA_TAG");
				BREAKORCONTINUE;
			}
			if (res->mask)
			{
				fprintf(output_file, "# %s%sDMA channel",
					   indep, (curdma == 0) ? "First " : "Next ");
				firstdma = res->value;
				showmask(res->mask, output_file);
				fprintf(output_file, ".\n");
			}
			else
			{
				fprintf(output_file, "# %s*** ERROR *** No DMA channel specified!\n", indep);
			}
			if ((res->data[1] & 3) == 0)
				fprintf(output_file, "# %s%s8 bit DMA only\n", indep, indep);
			if ((res->data[1] & 3) == 1)
				fprintf(output_file, "# %s%s8 & 16 bit DMA\n", indep, indep);
			if ((res->data[1] & 3) == 2)
				fprintf(output_file, "# %s%s16 bit DMA only\n", indep, indep);
			fprintf(output_file, "# %s%sLogical device is%s a bus master\n", indep, indep, res->data[2] & 4 ? "" : " not");
			fprintf(output_file, "# %s%sDMA may%s execute in count by byte mode\n", indep, indep, res->data[1] & 8 ? "" : " not");
			fprintf(output_file, "# %s%sDMA may%s execute in count by word mode\n", indep, indep, res->data[1] & 0x10 ? "" : " not");
			if ((res->data[1] & 0x60) == 0x00)
				fprintf(output_file, "# %s%sDMA channel speed in compatible mode\n", indep, indep);
			if ((res->data[1] & 0x60) == 0x20)
				fprintf(output_file, "# %s%sDMA channel speed type A\n", indep, indep);
			if ((res->data[1] & 0x60) == 0x40)
				fprintf(output_file, "# %s%sDMA channel speed type B\n", indep, indep);
			if ((res->data[1] & 0x60) == 0x60)
				fprintf(output_file, "# %s%sDMA channel speed type F\n", indep, indep);
			fprintf(output_file, "%s (DMA %d (CHANNEL %d))\n", comment_if_not_selected, curdma, firstdma);
			curdma++;
			if (!starteddeps)
				depdma = curdma;
			break;
		}
	case StartDep_TAG:
		{
			if (len > 1)
			{
				lengtherror(res, "StartDep_TAG");
				BREAKORCONTINUE;
			}
			putc('\n', output_file);
			if (!starteddeps)
				fprintf(output_file, "# Multiple choice time, choose one only !\n\n");
			if (res->len == 0)
			{
				fprintf(output_file, "# %sStart dependent functions: priority acceptable\n", indep);
			}
			else
				switch (res->data[0])
				{
				case 0:
					fprintf(output_file, "# %sStart dependent functions: priority preferred\n", indep);
					break;
				case 1:
					fprintf(output_file, "# %sStart dependent functions: priority acceptable\n", indep);
					break;
				case 2:
					fprintf(output_file, "# %sStart dependent functions: priority functional\n", indep);
					break;
				default:
					fprintf(output_file, "# %sStart dependent functions: priority INVALID\n", indep);
					break;
				}
			indep = "      ";
			starteddeps = 1;
			curio = depio;
			curdma = depdma;
			curint = depint;
			curmem = depmem;
			break;
		}
	case EndDep_TAG:
		{
			int i;
			if (len > 0)
			{
				lengtherror(res, "EndDep_TAG");
				BREAKORCONTINUE;
			}
			for (i = 0; i < res->end; i++)
			{
				for_each_resource(res->alternatives[i].resources,
								  res->alternatives[i].len,
								  print_resource,
								  output_file,
								  selected && (res->value == i));
			}
			indep = "    ";
			fprintf(output_file, "\n# %sEnd dependent functions\n", indep);
			break;
		}
	case IOport_TAG:
		{
			if (len != 7)
			{
				lengtherror(res, "IOport_TAG");
				fprintf(output_file, "# Bad tag length in 0x%02x\n", res->tag);
				BREAKORCONTINUE;
			}
			fprintf(output_file, "# %sLogical device decodes %s IO address lines\n", indep, res->data[0] ? "16 bit" : "10 bit");
			fprintf(output_file, "# %s%sMinimum IO base address 0x%04lx\n", indep, indep, res->start);
			fprintf(output_file, "# %s%sMaximum IO base address 0x%04lx\n", indep, indep, res->end - 1);
			fprintf(output_file, "# %s%sIO base alignment %ld bytes\n", indep, indep, res->step);
			if(res->errflags & RES_ERR_NO_STEP)
				fprintf(output_file, "# %s%s*** Bad resource data: Base alignment 0 - changed to 1\n", indep, indep);
			fprintf(output_file, "# %s%sNumber of IO addresses required: %d\n", indep, indep, res->data[6]);
#ifdef DUMPADDR
			for (i = ((res->data[2] << 8) + res->data[1]); i <= ((res->data[4] << 8) + res->data[3]); i += res->data[5])
			{
				fprintf(output_file, "# %s%s%s0x%04x..0x%04x\n", indep, indep, indep, i, i + res->data[6] - 1);
			}
#endif /* DUMPADDR */
			fprintf(output_file, "%s (IO %d (SIZE %d) (BASE 0x%04lx)%s)\n",
				   comment_if_not_selected, curio, res->data[6], res->value, IORangeCheck ? " (CHECK)" : "");
			curio++;
			if (!starteddeps)
				depio = curio;
			break;
		}
	case FixedIO_TAG:
		{
			if (len != 3)
			{
				lengtherror(res, "FixedIO_TAG");
				fprintf(output_file, "# Bad tag length in 0x%02x\n", res->tag);
				BREAKORCONTINUE;
			}
			fprintf(output_file, "# %sFixed IO base address 0x%04lx\n", indep, res->start);
			fprintf(output_file, "# %s%sNumber of IO addresses required: %ld\n", indep, indep, res->size);
			fprintf(output_file, "%s (IO %d (SIZE %ld) (BASE 0x%04lx)%s)\n",
				   comment_if_not_selected, curio, res->size, res->start, IORangeCheck ? " (CHECK)" : "");
			curio++;
			if (!starteddeps)
				depio = curio;
			break;
		}
	case MemRange_TAG:
		{
			char width = 'w';

			if (len != 9)
			{
				lengtherror(res, "MemRange_TAG");
				fprintf(output_file, "# %sInvalid length for memory range tag 0x%02x\n", indep, res->tag);
				BREAKORCONTINUE;
			}
			fprintf(output_file, "# %sMemory is %s\n", indep, res->data[0] & 1 ? "writeable" : "non-writeable (ROM)");
			fprintf(output_file, "# %sMemory is %s\n", indep, res->data[0] & 2 ? "read cacheable, write-through" : "non-cacheable");
			fprintf(output_file, "# %sMemory decode supports %s\n", indep, res->data[0] & 4 ? "range length" : "high address");
			if ((res->data[0] & 0x18) == 0x00)
			{
				width = 'b';
				fprintf(output_file, "# %smemory is 8-bit only\n", indep);
			}
			if ((res->data[0] & 0x18) == 0x08)
				fprintf(output_file, "# %smemory is 16-bit only\n", indep);
			if ((res->data[0] & 0x18) == 0x10)
				fprintf(output_file, "# %smemory is 8-bit and 16-bit\n", indep);
			if (res->data[0] & 0x20)
				fprintf(output_file, "# %smemory is shadowable\n", indep);
			if (res->data[0] & 0x40)
				fprintf(output_file, "# %smemory is an expansion ROM\n", indep);
			fprintf(output_file, "# %sMinimum memory base address 0x%06lx\n", indep, res->start);
			fprintf(output_file, "# %sMaximum memory base address 0x%06lx\n", indep, res->end - 1);
			fprintf(output_file, "# %sRange base alignment mask 0xff%04lx bytes\n", indep, res->step);
			fprintf(output_file, "# %sRange length %ld bytes\n", indep, res->size);
#ifdef DUMPADDR
#if 0
			***untested ***
				for (i = ((res->data[2] << 16) + (res->data[1] << 8));
					 i <= ((res->data[4] << 16) + (res->data[3] << 8));
					 i += ((res->data[6] << 8) + res->data[5]))
			{
				fprintf(output_file, "# %s%s0x%06x..0x%06x\n", indep, indep,
					 i, i + (res->data[8] << 16) + (res->data[7] << 8) - 1);
			}
#endif
#endif /* DUMPADDR */
			fprintf(output_file, "# Choose UPPER = Range, or UPPER = Upper limit to suit hardware\n");
			fprintf(output_file, "%s (MEM %d (BASE 0x%06lx) (MODE %cu) (UPPER 0x%06lx))\n",
				   comment_if_not_selected,
				   curmem, res->start, width, res->start + res->size);
			fprintf(output_file, "# (MEM %d (BASE 0x%06lx) (MODE %cr) (UPPER 0x%06lx))\n", curmem, res->start, width, res->size);	/* XXX AJR */
			curmem++;
			if (!starteddeps)
				depmem = curmem;
			break;
		}
	case ANSIstr_TAG:
		{
			fprintf(output_file, "# %sANSI string -->", indep);
			/* Remember the device name */
			if(devstring)
				free(devstring);
			devstring = (char *)malloc(len + 1);
			if(!devstring)
			{
				fprintf(stderr, "Out of memory to store board device string\n");
				exit(1);
			}
			for (i = 0; i < len; i++)
				devstring[i] = res->data[i];
			devstring[i] = 0;
			fprintf(output_file, "%s<--\n", devstring);
			break;
		}
	case UNICODEstr_TAG:
		{
			fprintf(output_file, "# %sUNICODE string -->", indep);
			/* Remember the device name */
			if(devstring)
				free(devstring);
			devstring = (char *)malloc(len + 1);
			if(!devstring)
			{
				fprintf(stderr, "Out of memory to store board device string\n");
				exit(1);
			}
			for (i = 0; i < len; i++)
				putc(devstring[i] = res->data[i], output_file);
			devstring[i] = 0;
			fprintf(output_file, "<--\n");
			break;
		}
	case VendorShort_TAG:
	case VendorLong_TAG:
		{
			fprintf(output_file, "# %sVendor defined tag: ", indep);
			fprintf(output_file, " %02x", res->tag);
			for (i = 0; i < len; i++)
				fprintf(output_file, " %02x", res->data[i]);
			putc('\n', output_file);
			break;
		}
	case End_TAG:
		{
			char *indep = "";
			if (nestdepth)
			{
				/* If we have a device name, show it (the last description string before the END DF flag is it) */
				if (serialno == 0xffffffffUL)
					fprintf(output_file, " (NAME \"%s/-1[%d]", devid, logdevno-1);
				else
					fprintf(output_file, " (NAME \"%s/%lu[%d]", devid, serialno, logdevno-1);
				if(devstring)
					fprintf(output_file, "{%-20s}", devstring);
				fprintf(output_file, "\")\n");
				fprintf(output_file, "%s (ACT Y)\n", comment_if_not_selected);
				while (nestdepth)
				{
					fprintf(output_file, ")");
					nestdepth--;
				}
				fprintf(output_file, "\n");
			}
			/* Cancel any device names */
			if(devstring)
				free(devstring);
			devstring = 0;
			fprintf(output_file, "# %sEnd tag... Checksum 0x%02x (%s)\n\n", indep, csum, (csum % 256) ? "BAD" : "OK");
			break;
		}
	case Mem32Range_TAG:
	case FixedMem32Range_TAG:
		{
			if (len != 17)
			{
				lengtherror(res, "Mem32Range_TAG or FixedMem32Range_TAG");
				BREAKORCONTINUE;
			}
			fprintf(output_file, "# %s32-bit MemRange tag %02x ...\n", indep, type);
			break;
		}
	default:
		{
			fprintf(output_file, "# %sUnknown tag %02x ...\n", indep, type);
			BREAKORCONTINUE;
		}
	}
	return;
#ifdef ABORT_ONRESERR
  oncorruptresourcedata:
	fprintf(output_file, "# Resource data dump aborted\n");
	exit(1);
#endif
}

#ifdef SCRIPT_OUTPUT
static void
print_script_header(FILE * output_file)
{
	fprintf(output_file,
			"#!/bin/sh\n"
			"prefixes=\"/etc/pnp/config-scripts/isa /usr/share/pnp/config-scripts/isa\"\n"
			"\n"
			"function pnp_isa_configure {\n"
			"    local i id dir found_it\n"
			"    i=0\n"
			"    while [ \"${dev_ids[$i]}\" != \"\" ] ; do\n"
			"        id=\"${dev_ids[$i]}\"\n"
			"        for dir in $prefixes ; do\n"
			"	    if [ -r $dir/$id ] ; then\n"
			"	        source $dir/$id\n"
			"		return $?\n"
			"	    fi\n"
			"	done\n"
			"	i=`expr $i + 1`\n"
			"    done\n"
			"    # Failed to find anything\n"
			"    return 1\n"
			"}\n"
	);
}

static void
print_script(FILE * output_file, struct resource *res, int selected)
{
	switch (res->type)
	{
	case NewBoard_PSEUDOTAG:
		{
			int csn = res->start;

			serialno = (unsigned long)
				serial_identifier[csn][4] +
				(serial_identifier[csn][5] << 8) +
				(serial_identifier[csn][6] << 16) +
				(serial_identifier[csn][7] << 24);
			strncpy(devid, devidstr(serial_identifier[csn][0],
									serial_identifier[csn][1],
									serial_identifier[csn][2],
									serial_identifier[csn][3]), 8);
		}
		break;
	case PnPVerNo_TAG:
		break;
	case LogDevId_TAG:
		{
			if (nestdepth)
			{
				fprintf(output_file, "pnp_isa_configure\n\n");
				nestdepth = 0;
			}
			fprintf(output_file,
			  "unset irq irq_flags dma dma_flags io_start io_len io_flags\n"
					"unset mem_start mem_len mem_flags dev_ids\n"
					"board_id=%s\n"
					"dev_ids[0]=%s\n",
					devid,
					devidstr(res->data[0], res->data[1],
							 res->data[2], res->data[3]));
			nestdepth = 2;
			curdma = 0;
			depdma = 0;
			curio = 0;
			depio = 0;
			curint = 0;
			depint = 0;
			curmem = 0;
			depmem = 0;
			curid = 2;
			starteddeps = 0;
			break;
		}
	case CompatDevId_TAG:
		{
			fprintf(output_file,
					"dev_ids[%d]=%s\n",
					curid,
					devidstr(res->data[0], res->data[1],
							 res->data[2], res->data[3]));
			curid++;
			break;
		}
	case IRQ_TAG:
		{
			fprintf(output_file,
					"irq[%d]=%ld ; irq_flags[%d]=0x%02x\n",
					curint, res->value, curint, res->data[2]);
			curint++;
			if (!starteddeps)
				depint = curint;
			break;
		}
	case DMA_TAG:
		{
			fprintf(output_file,
					"dma[%d]=%ld ; dma_flags[%d]=0x%02x\n",
					curdma, res->value, curdma, res->data[1]);
			curdma++;
			if (!starteddeps)
				depdma = curdma;
			break;
		}
	case StartDep_TAG:
		{
			starteddeps = 1;
			curio = depio;
			curdma = depdma;
			curint = depint;
			curmem = depmem;
			break;
		}
	case EndDep_TAG:
		{
			for_each_resource(res->alternatives[res->value].resources,
							  res->alternatives[res->value].len,
							  print_script,
							  output_file, 1);
			break;
		}
	case IOport_TAG:
		{
			fprintf(output_file,
					"io_start[%d]=0x%04lx ; "
					"io_len[%d]=%-2ld ; "
					"io_flags[%d]=0x%02x\n",
					curio, res->value,
					curio, res->size,
					curio, res->data[0]);
			curio++;
			if (!starteddeps)
				depio = curio;
			break;
		}
	case FixedIO_TAG:
		{
			fprintf(output_file,
					"io_start[%d]=0x%04lx ; io_len[%d]=%ld ; "
					"io_flags[%d]=FIXED_IO\n",
					curio, res->start, curio, res->size, curio);
			curio++;
			if (!starteddeps)
				depio = curio;
			break;
		}
	case MemRange_TAG:
		{
			fprintf(output_file,
					"mem_start[%d]=0x%04lx ; mem_len[%d]=%ld\n ; "
					"mem_flags[%d]=0x%04x\n",
					curmem, res->value, curmem, res->size,
					curmem, res->data[0]);
			curmem++;
			if (!starteddeps)
				depmem = curmem;
			break;
		}
	case ANSIstr_TAG:
		break;
	case UNICODEstr_TAG:
		break;
	case VendorShort_TAG:
	case VendorLong_TAG:
		break;
	case End_TAG:
		{
			if (nestdepth)
			{
				fprintf(output_file, "pnp_isa_configure\n\n");
				nestdepth = 0;
			}
			break;
		}
	case Mem32Range_TAG:
		{
			/* STUB: Need to add support for this. */
			break;
		}
	case FixedMem32Range_TAG:
		{
			/* STUB: Need to add support for this. */
			break;
		}
	default:
		{
			fprintf(output_file, "# %sUnknown tag %02x ...\n", indep, res->type);
			BREAKORCONTINUE;
		}
	}
	return;
#ifdef ABORT_ONRESERR
  oncorruptresourcedata:
	fprintf(output_file, "# Resource data dump aborted\n");
	exit(1);
#endif
}
#endif /* SCRIPT_OUTPUT */


#ifdef DUMPDATA
static void
dumpdata(int csn)
{
	int i;
	/* #define OLD_READ_STRATEGY */
#ifdef OLD_READ_STRATEGY
	struct resource res;
#endif /* OLD_READ_STRATEGY */
	unsigned long serialno = (unsigned long) serial_identifier[csn][4] + (serial_identifier[csn][5] << 8) +
	(serial_identifier[csn][6] << 16) + (serial_identifier[csn][7] << 24);
	struct resource *res_array;
	int array_len;
	int alloc_result;

	strncpy(devid, devidstr(serial_identifier[csn][0],
							serial_identifier[csn][1],
							serial_identifier[csn][2],
							serial_identifier[csn][3]), 8);

	nestdepth = 0;
	curdma = 0;
	depdma = 0;
	curio = 0;
	depio = 0;
	curint = 0;
	depint = 0;
	curmem = 0;
	depmem = 0;
	starteddeps = 0;

	Wake(csn);
	fprintf(o_file, "# Card %d: (serial identifier", csn);
	for (i = IDENT_LEN; i--;)
		fprintf(o_file, " %02x", serial_identifier[csn][i]);
	fprintf(o_file, ")\n");
	if (serialno == 0xffffffffUL)
	{
		fprintf(o_file, "# Vendor Id %s, No Serial Number (-1), checksum 0x%02X.\n", devid, serial_identifier[csn][8]);
	}
	else
	{
		fprintf(o_file, "# Vendor Id %s, Serial Number %lu, checksum 0x%02X.\n", devid, serialno, serial_identifier[csn][8]);
	}
	/*
	 * Check for broken cards that don't reset their resource pointer
	 * properly
	 */
	for (i = 0; i < TMP_LEN; i++)
		tmp[i] = 0;
	/* Get the first byte */
	if (read_resource_data(&tmp[0]) != 0)
		return;
	/* Just check the first byte, if these match we assume it's ok */
	if (tmp[0] != serial_identifier[csn][0])
	{
		fprintf(o_file, "# Ident byte %d, (%02x) differs from resource data (%02x)\n", i, serial_identifier[csn][i], tmp[0]);
		fprintf(o_file, "#Assuming the card is broken and this is the start of the resource data\n");
		unread_resource_data(tmp[0]);
		goto broken;			/* Wouldn't need this if people read the spec */
	}
	/*
	 * Read resource data past serial identifier - As it should be spec
	 * section 4.5 para 2
	 */
	for (i = 1; i < IDENT_LEN; i++)
	{
		/* Use tmp[0] as a temporary vbl */
		if (read_resource_data(&tmp[0]) != 0)
			return;
		/*
		 * Checksum byte is not guaranteed, but the previous bytes should
		 * match
		 */
		if ((tmp[0] != serial_identifier[csn][i]) && (i < (IDENT_LEN - 1)))
			fprintf(o_file, "# Ident byte %d, (%02x) differs from resource data (%02x)\n", i, serial_identifier[csn][i], tmp[0]);
	}
	/* Now for the actual resource data */
	csum = 0;
  broken:
	res_array = NULL;
	array_len = 0;
	read_board_resources(NULL, 0, &res_array, &array_len);
	if (do_autoconfig)
	{
		alloc_result = alloc_resources(res_array, array_len, NULL, 0);
	}
	else
	{
		alloc_result = 0;
	}
	for_each_resource(res_array, array_len,
					  print_resource, o_file, alloc_result);
}
#endif /* DUMPDATA */

static void
dumpregs(int csn)
{
	int logical_device;
	int i;
	int addr;
	int desc;
	unsigned long serialno = (unsigned long) serial_identifier[csn][4] + (serial_identifier[csn][5] << 8) +
	(serial_identifier[csn][6] << 16) + (serial_identifier[csn][7] << 24);
	strncpy(devid, devidstr(serial_identifier[csn][0],
							serial_identifier[csn][1],
							serial_identifier[csn][2],
							serial_identifier[csn][3]), 8);

	Wake(csn);
	fprintf(o_file, "# Configuration registers for card %d: (serial identifier", csn);
	for (i = IDENT_LEN; i--;)
		fprintf(o_file, " %02x", serial_identifier[csn][i]);
	fprintf(o_file, ")\n");
	if (serialno == 0xffffffffUL)
	{
		fprintf(o_file, "# Vendor Id %s, No Serial Number (-1), checksum 0x%02X.\n", devid, serial_identifier[csn][8]);
	}
	else
	{
		fprintf(o_file, "# Vendor Id %s, Serial Number %lu, checksum 0x%02X.\n", devid, serialno, serial_identifier[csn][8]);
	}
	ADDRESS(0x06);
	fprintf(o_file, "#\n# Card Select Number register [0x06]: %d\n", READ_DATA);
	fprintf(o_file, "#\n# Vendor defined card level registers 0x20..2f:");
	for (addr = 0x20; addr < 0x30; addr++)
	{
		ADDRESS(addr);
		fprintf(o_file, " %02x", READ_DATA);
	}
	putc('\n', o_file);
	for (logical_device = 0; logical_device < 256; logical_device++)
	{
		LOGICALDEVICENUMBER;
		WRITE_DATA(logical_device);
		if (READ_DATA != logical_device)
			break;
		fprintf(o_file, "#\n# Logical device %d\n", logical_device);
		ADDRESS(0x30);
		fprintf(o_file, "# Activate       register [0x30]: %s\n", READ_DATA ? "* Activated *" : "Inactive");
		ADDRESS(0x31);
		fprintf(o_file, "# IO range check register [0x31]: ");
		tmp[0] = READ_DATA;
		if(tmp[0] & 2)
			fprintf(o_file, "IO range check active, reads return %02x", tmp[0] & 1 ? 0x55 : 0xAA);
		else
			fprintf(o_file, "Inactive");
		fprintf(o_file, "\n# Logical device Control reserved reg 0x32..37:");
		for (addr = 0x32; addr < 0x38; addr++)
		{
			ADDRESS(addr);
			fprintf(o_file, " %02x", READ_DATA);
		}
		fprintf(o_file, "\n# Logical device Control Vendor defnd 0x38..40:");
		for (addr = 0x38; addr < 0x40; addr++)
		{
			ADDRESS(addr);
			fprintf(o_file, " %02x", READ_DATA);
		}
		putc('\n', o_file);
		for (addr = 0x40, desc = 0; addr < 0x60; addr += 8, desc++)
		{
			fprintf(o_file, "# 24 bit Memory descriptor %d at %02x..%02x: ", desc, addr, addr + 4);
			for (i = 0; i < 5; i++)
			{
				ADDRESS(addr + i);
				tmp[i] = READ_DATA;
			}
			fprintf(o_file, "Base address 0x%02x%02x00 %s 0x%02x%02x00, %d bit\n", tmp[0], tmp[1],
				   tmp[2] & 1 ? "...." : "size",
				   tmp[3], tmp[4], tmp[2] & 2 ? 16 : 8);
		}
		for (addr = 0x76, desc = 0; addr < 0xb0; addr += 16, desc++)
		{
			fprintf(o_file, "# 32 bit Memory descriptor %d at %02x..%02x: ", desc, addr, addr + 8);
			for (i = 0; i < 9; i++)
			{
				ADDRESS(addr + i);
				tmp[i] = READ_DATA;
			}
			fprintf(o_file, "Base address 0x%02x%02x%02x%02x %s 0x%02x%02x%02x%02x, %d bit\n", tmp[0], tmp[1], tmp[2], tmp[3],
				   tmp[4] & 1 ? "...." : "size",
				   tmp[5], tmp[6], tmp[7], tmp[8], tmp[8] & 4 ? 32 : (tmp[8] & 2 ? 16 : 8));
			addr &= 0xf0;
		}
		for (addr = 0x60, desc = 0; addr < 0x70; addr += 2, desc++)
		{
			fprintf(o_file, "# IO descriptor %d at %02x..%02x: ", desc, addr, addr + 1);
			for (i = 0; i < 2; i++)
			{
				ADDRESS(addr + i);
				tmp[i] = READ_DATA;
			}
			fprintf(o_file, "Base address 0x%02x%02x\n", tmp[0], tmp[1]);
		}
		for (addr = 0x70, desc = 0; addr < 0x74; addr += 2, desc++)
		{
			fprintf(o_file, "# Interrupt descriptor %d at %02x..%02x: ", desc, addr, addr + 1);
			for (i = 0; i < 2; i++)
			{
				ADDRESS(addr + i);
				tmp[i] = READ_DATA;
			}
			fprintf(o_file, "Interrupt level %d, active %s, %s triggered\n", tmp[0], tmp[1] & 2 ? "high" : "low", tmp[1] & 1 ? "level" : "edge");
		}
		for (addr = 0x74; addr < 0x76; addr++)
		{
			fprintf(o_file, "# DMA descriptor %d at %02x: ", addr - 0x74, addr);
			ADDRESS(addr);
			tmp[0] = READ_DATA;
			fprintf(o_file, "DMA channel %d\n", tmp[0]);
		}
	}
	putc('\n', o_file);
}

/* End of pnpdump_main.c */

/*
** Licence @ the bottom of the script
*/

/*
** freec - A simple program like free(1) which displays meminfo
** in a nice and coloured way.
*/

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <string.h>
#include <ctype.h>

#include "freec.h"

/* OKay, that is awefull, but much more simple */
struct conf_info conf;

int
main(int argc, char **argv)
{
	struct meminfo memory_info;

	conf_default_flags();
	get_opts(argc, argv);

	while(conf.count_times) {
		get_meminfo(&memory_info);
		/*
		printf("memtot:  %u\n", memory_info.mem_total);
		printf("memfree: %u\n", memory_info.mem_free);
		printf("buffers: %u\n", memory_info.buffers);
		printf("cached:  %u\n", memory_info.cached);
		printf("swap_total: %u\n", memory_info.swap_total);
		printf("swap_free:  %u\n", memory_info.swap_free);
		*/
		get_tty_info();
		conf.bar_length = conf.tty_width - 10; /* FIXME */
		work_meminfo(&memory_info);
		display_meminfo(&memory_info);

		if(conf.count_times > 0) /* -c -1 -> BUG FIXME */
			conf.count_times--;
		if(conf.count_times)
			sleep(conf.seconds);
	}

	/* free everithing if needed */

	return(EXIT_SUCCESS);
}

void
conf_default_flags(void)
{ /* {{{ */
	conf.size_unit   = DEFAULT_UNIT;
	conf.SI_unit     = 0;
	conf.humanize    = 0; /* Conflicts with size_unit */
	conf.colorize    = 1;
	conf.no_graph    = 0;
	conf.seconds     = 1;
	conf.count_times = 1;
	conf.is_a_tty    = 1;
	conf.tty_width   = 80; /* FIXME */
	conf.bar_length  = BAR_LEN;
} /* }}} */


void
get_opts(int argc, char **argv)
{ /* {{{ */
	int opt, option_index;

	static struct option long_options[] ={
		{"bytes", no_argument, 0, 'b'},
		{"kilo", no_argument, 0, 'k'},
		{"mega", no_argument, 0, 'm'},
		{"giga", no_argument, 0, 'g'},
		{"tera", no_argument, 0, 'T'},
		{"humanize", no_argument, 0, 'h'},
		{"human", no_argument, 0, 'h'},
		{"count", required_argument, 0, 'c'},
		{"seconds", required_argument, 0, 's'},
		{"si", no_argument, 0, 'S'},
		{"nocolor", no_argument, 0, 'C'},
		{"no-color", no_argument, 0, 'C'},
		{"nograph", no_argument, 0, 'G'},
		{"no-graph", no_argument, 0, 'G'},
		{"help", no_argument, 0, '?'},
		{0, 0, 0, 0}
	};
	const char *opts = "bkmgTSCs:c:G?h";

	option_index = 0;

	while((opt = getopt_long(argc, argv, opts, long_options,
	          &option_index)) != -1) {
		switch(opt) {
		case 'b': /* Bytes */
			conf.size_unit = BYTES;
			break;
		case 'k': /* Kilos */
			conf.size_unit = KILOS;
			break;
		case 'm': /* Megas */
			conf.size_unit = MEGAS;
			break;
		case 'g': /* Gigas */
			conf.size_unit = GIGAS;
			break;
		case 'T': /* Teras */
			conf.size_unit = TERAS;
			break;
		case 'S': /* SI units */
			conf.SI_unit = 1;
			break;
		case 'C': /* do not colorize */
			conf.colorize = 0;
			break;
		case 's': /* seconds */
			errno = 0; /* reset errno for error checking below */
			conf.seconds = (unsigned int)strtoul(optarg, NULL, 10);
			if (errno == ERANGE) /* TODO return on error? */
				fprintf(stderr, "Value conversion failed "
				         "when treating -s option\n");
			break;
		case 'c': /* count times */
			errno = 0; /* reset errno for error checking below */
			conf.count_times= (int)strtol(optarg, NULL, 10);
			if (errno == ERANGE) /* TODO return on error? */
				fprintf(stderr, "Value conversion failed "
					      "when treating -c option\n");
			break;
		case 'G': /* no bar-graph */
			conf.no_graph = 1;
	 		break;
		case 'h':
			conf.humanize = 1;
			break;
		case '?':
			display_help();
			break;
		default:
			print_usage(argv);
			break;
		}
	}
} /* }}} */


void
get_meminfo(struct meminfo *mem_info)
{ /* {{{ */
	FILE *meminfo_file;
	char buffer[200];
	char tag[200], value[200], unit[200]; /* yep, be sure */

	if((meminfo_file = fopen(MEMINFO_FILE, "r")) == NULL)
	{
		perror("Could not open file "MEMINFO_FILE", are-you sure you're"
		       "\non a Linux machine ? ");
		exit(EXIT_FAILURE);
	}

	while(fgets(buffer, 199, meminfo_file) != NULL) {
		convert_string_to_lower(buffer);
		explode_line(buffer, tag, value, unit);

		if(strcmp(tag, FREEC_MEM_TOTAL) == 0) 
			insert_data(value, unit, &mem_info->mem_total);

		else if(strcmp(tag, FREEC_MEM_FREE) == 0)
			insert_data(value, unit, &mem_info->mem_free);

		else if(strcmp(tag, FREEC_BUFFERS) == 0)
			insert_data(value, unit, &mem_info->buffers);

		else if(strcmp(tag, FREEC_CACHED) == 0)
			insert_data(value, unit, &mem_info->cached);

		else if(strcmp(tag, FREEC_SWAP_TOTAL) == 0)
			insert_data(value, unit, &mem_info->swap_total);

		else if(strcmp(tag, FREEC_SWAP_FREE) == 0)
			insert_data(value, unit, &mem_info->swap_free);
	}
	fclose(meminfo_file);
} /* }}} */

void
work_meminfo(struct meminfo *mem_info)
{ /* {{{ */
	work_central(mem_info);

	work_swap(mem_info);
} /* }}} */

void
work_central(struct meminfo *mem)
{ /* {{{ */
	unsigned int total;
	int ceiled; /* ceil the first time only ! */
	mem->mem_used = mem->mem_total -
	                mem->mem_free -
	                mem->buffers -
	                mem->cached; /* FIXME - I was tired... */
	/*Well, theoreticaly, buffers and cache are "memused", but we want to
	display them, so count them off */
	ceiled = 0;
	total  = 0;

	mem->pixels_mem_used =
	  proportionality(mem->mem_used, mem->mem_total, conf.bar_length,
	                  &ceiled);
	total += mem->pixels_mem_used;

	mem->pixels_mem_buffers =
	  proportionality(mem->buffers, mem->mem_total, conf.bar_length,
	                  &ceiled);
	total += mem->pixels_mem_buffers;

	mem->pixels_mem_cached =
	  proportionality(mem->cached, mem->mem_total, conf.bar_length,
	                  &ceiled);
	total += mem->pixels_mem_cached;

/*	mem->pixels_mem_free =
	  proportionality(mem->mem_free, mem->mem_total, conf->bar_length,
	                  &ceiled);
	*/
	mem->pixels_mem_free = conf.bar_length - total - 1;
	/* XXX FIXME: This IS crap... */
} /* }}} */

void
work_swap(struct meminfo *mem)
{ /* {{{ */
	if(mem->swap_free == 0 && mem->swap_free == 0) {
		mem->pixels_swap_free = 0;
		mem->pixels_swap_used = 0;
	}
	else {
		mem->swap_used = mem->swap_total - mem->swap_free;

		mem->pixels_swap_free = (mem->swap_free * conf.bar_length) /
		                         mem->swap_total - 1;
		mem->pixels_swap_used = conf.bar_length - mem->pixels_swap_free - 1;
		/* hahaha, dirty */
	}
} /* }}} */

void
display_meminfo(struct meminfo *mem)
{ /* {{{ */

	/* DOING THE "RAM" FIRST */

	if(!conf.no_graph) {
		fputs(MEM_TAG"[", stdout);
		display_pixel(mem->pixels_mem_used, CHAR_USED, COLOR_USED);
		display_pixel(mem->pixels_mem_buffers, CHAR_BUFFERS, COLOR_BUFFERS);
		display_pixel(mem->pixels_mem_cached, CHAR_CACHED, COLOR_CACHED);
		display_pixel(mem->pixels_mem_free, CHAR_FREE, COLOR_FREE);
		fputs("]\n", stdout);
	}

	/*
	printf("U: "COLOR_USED"%d"COLOR_NORMAL"%s, ", mem->mem_used, "Kb");
	printf("B: "COLOR_BUFFERS"%d"COLOR_NORMAL"%s, ", mem->buffers, "Kb");
	printf("C: "COLOR_CACHED"%d"COLOR_NORMAL"%s, ", mem->cached, "Kb");
	printf("F: "COLOR_FREE"%d"COLOR_NORMAL"%s\n", mem->mem_free, "Kb");
	*/

	if(conf.colorize)
		fputs(COLOR_USED"U: ", stdout);
	else
		fputs("U: ", stdout);

	display_unit(mem->mem_used);

	if(conf.colorize)
		fputs(COLOR_NORMAL", "COLOR_BUFFERS"B: ", stdout);
	else
		fputs(", B: ", stdout);

	display_unit(mem->buffers);

	if(conf.colorize)
		fputs(COLOR_NORMAL", "COLOR_CACHED"C: ", stdout);
	else
		fputs(", C: ", stdout);

	display_unit(mem->cached);

	if(conf.colorize)
		fputs(COLOR_NORMAL", "COLOR_FREE"F: ", stdout);
	else
		fputs(", F: ", stdout);

	display_unit(mem->mem_free);

	if(conf.colorize)
		fputs(" "COLOR_NORMAL, stdout);
	else
		fputs(" ", stdout);


	if(mem->swap_total) {
		putchar('\n');
		if(!conf.no_graph) {
			fputs(SWAP_TAG"[", stdout);
			display_pixel(mem->pixels_swap_used, CHAR_USED,
			              COLOR_USED);
			display_pixel(mem->pixels_swap_free, CHAR_FREE,
			              COLOR_FREE);
			fputs("]\n", stdout);
		}

		if(conf.colorize)
			fputs(COLOR_USED"U: ", stdout);
		else
			fputs("U: ", stdout);

		display_unit(mem->swap_used);

		if(conf.colorize)
			fputs(COLOR_NORMAL", "COLOR_FREE"F: ", stdout);
		else
			fputs(", F: ", stdout);

		display_unit(mem->swap_free);
	}

	putchar('\n');

} /* }}} */

void
display_pixel(unsigned int times, char pixel, char *color)
{ /* {{{ */
	if(conf.colorize)
		fputs(color, stdout);

	while(times-->0)
		putchar(pixel);
	if(conf.colorize)
		fputs(COLOR_NORMAL, stdout);
} /* }}} */

void
display_unit(unsigned int mem)
{ /* {{{ */
	int times_to_div;
	char *unit;

	times_to_div = conf.size_unit;

	if(conf.SI_unit) {

		if(conf.humanize) {
			humanize_unit(&mem, &times_to_div, 1000);
		}
		else {
			while(times_to_div > 0) {
				mem /= 1000;
				times_to_div--;
			}
			if(times_to_div == -1)
				mem *= 1000;
			times_to_div = conf.size_unit;
		}

		switch(times_to_div) {
		case BYTES:
			unit = "B";
			break;
		case KILOS:
			unit = "KB";
			break;
		case MEGAS:
			unit = "MB";
			break;
		case GIGAS:
			unit = "GB";
			break;
		case TERAS:
			unit = "TB";
			break;
		default:
			unit = "NaN";
			fprintf(stderr, "Error with unit @ display_unit!\n");
			break;
		}
	}
	else {

		if(conf.humanize) {
			humanize_unit(&mem, &times_to_div, 1024);
		}
		else {
			while(times_to_div > 0) {
				mem /= 1024;
				times_to_div--;
			}
			if(times_to_div == -1)
				mem *= 1024;
			times_to_div = conf.size_unit;
		}

		switch(times_to_div) {
		case BYTES:
			unit = "B";
			break;
		case KILOS:
			unit = "KiB";
			break;
		case MEGAS:
			unit = "MiB";
			break;
		case GIGAS:
			unit = "GiB";
			break;
		case TERAS:
			unit = "TiB";
			break;
		default:
			unit = "NaN";
			fprintf(stderr, "Error with unit @ display_unit!\n");
			break;
		}
	}

	printf("%d%s", mem, unit);
} /* }}} */

void
humanize_unit(unsigned int *mem, int *divs, unsigned int scaler)
{ /* {{{ */
	unsigned int before;

	before = *mem;
	*divs = 1; /* by default: kilos */
	while(*mem >= scaler) {
		before = *mem;
		*mem /= scaler;
		(*divs)++;
	}

	/* 1613 MB is 2GB, and not 1GB */
	if((before - ((*mem) * scaler)) > (scaler / 2)) {
		(*mem)++;
	}
	(*divs)--;
} /* }}} */

void
print_usage(char **argv)
{ /* {{{ */
	fprintf(stderr, "Usage: %s blah blah\n", argv[0]);
	exit(EXIT_FAILURE);
} /* }}} */

void
display_help(void)
{ /* {{{ */
	puts("RT(F)M");
	exit(EXIT_FAILURE);
} /* }}} */


/* UTILS */

void
get_tty_info(void)
{ /* {{{ */
	struct winsize win;

	if(!isatty(STDOUT_FILENO)) {
		conf.is_a_tty  = 0;
		conf.tty_width = 80;
		return;
	}
	else {
		if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) != 0) {
			fprintf(stderr, "Could not get TTY size !");
			win.ws_col = 80;
		}
		conf.is_a_tty  = 1;
		conf.tty_width = win.ws_col;
	}
} /* }}} */

void
explode_line(char *buffer, char *tag, char *value, char *unit)
{ /* {{{ */
	char *arg;

	arg = strtok(buffer, " ");
	tag[0] = '\0';
	strcpy(tag, arg);

	arg = strtok(NULL, " ");
	if(arg == NULL)
		fprintf(stderr, "ERROR parsing "MEMINFO_FILE" !\n");
	value[0] = '\0';
	strcpy(value, arg);


	arg = strtok(NULL, " ");
	if(arg == NULL)
		unit[0] = '\0';
	else {
		unit[0] = '\0';
		strcpy(unit, arg);
	}
} /* }}} */

void
insert_data(char *value, char *unit, unsigned int *where)
{ /* {{{ */
	if(strcmp(unit, PARSE_KILO_BYTES_UNIT)) {
		errno = 0; /* reset errno for error checking below */
		*where = (unsigned int)strtoul(value, NULL, 10);
		if (errno == ERANGE) /* TODO return on error? */
			fprintf(stderr, "Value conversion failed\n");
	}
	else /* TODO, but should not happen */
		printf("ERROR in insert_data");
} /* }}} */

void
convert_string_to_lower(char *s)
{
	for (; *s; ++s)
		*s = (char)tolower(*s);
}

unsigned int
proportionality(unsigned int have, unsigned int total, unsigned int ratio,
                int *ceiled)
{ /* {{{ */
	double n;
	unsigned int o;

	n = ((double)have * (double)(ratio))/((double)total);
	o = (unsigned int)n;
/*	printf("%d, %lf", o, n); */
	n -= o;
	if(n >= 0.5 && !(*ceiled)) {
/*		printf("I"); */
		o++;
		*ceiled = 1;
	}
/*	putchar('\n'); */
	return o;
} /* }}} */

/*
Copyright (c) 2013, Frank Villaro-Dixon
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	* Neither the name of the author  nor the names of its contributors may
	be used to endorse or promote products derived from this software
	without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLEFOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

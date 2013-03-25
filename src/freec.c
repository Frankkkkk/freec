/*
** LICENCE
*/

/*
** freec - A simple program like free(1) which displays meminfo
** in a nice and coloured way.
*/

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <string.h>

#include "freec.h"

int
main(int argc, char **argv)
{
	struct conf_info conf;
	struct meminfo memory_info;

	conf_default_flags(&conf);
	get_opts(argc, argv, &conf);

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
		get_tty_info(&conf);
		conf.bar_length = conf.tty_width - 10;
		work_meminfo(&memory_info, &conf);
		display_meminfo(&memory_info, &conf);

		conf.count_times--;
		if(conf.count_times)
			sleep(conf.seconds);
	}

	//free everithing if needed

	return(EXIT_SUCCESS);
}

void
conf_default_flags(struct conf_info *conf)
{
	conf->size_unit   = MEGAS;
	conf->SI_unit     = 0;
	conf->humanize    = 1;
	conf->colorize    = 1;
	conf->seconds     = 1;
	conf->count_times = 1;
	conf->is_a_tty    = 1;
	conf->tty_width   = 80; //FIXME
	conf->bar_length  = BAR_LEN;
}


void
get_opts(int argc, char **argv, struct conf_info *conf)
{
	int opt, option_index;
	option_index = 0;

	static struct option long_options[] ={
		{"bytes", no_argument, 0, 'b'},
		{"kilo", no_argument, 0, 'k'},
		{"mega", no_argument, 0, 'm'},
		{"giga", no_argument, 0, 'g'},
		{"tera", no_argument, 0, 'T'},
		//{"count", no_argument, 0, 'T'}, //TODO count
		//{"seconds", no_argument, 0, 's'}, //TODO seconds
		{"si", no_argument, 0, 'S'},
		{"help", no_argument, 0, 'H'},
		{"nocolor", no_argument, 0, 'C'},
		{"no-color", no_argument, 0, 'C'},
		{"help", no_argument, 0, '?'},
		{0, 0, 0, 0}
	};
	const char *opts = "bkmgTSCsc:?h";

	while((opt = getopt_long(argc, argv, opts, long_options,
	          &option_index)) != -1) {
		switch(opt) {
			case 'b': //Bytes
				conf->size_unit = BYTES;
				break;
			case 'k': //Kilos
				conf->size_unit = KILOS;
				break;
			case 'm': //Megas
				conf->size_unit = MEGAS;
				break;
			case 'g': //Gigas
				conf->size_unit = GIGAS;
				break;
			case 'T': //Teras
				conf->size_unit = TERAS;
				break;
			case 'S': //SI units
				conf->SI_unit = 1;
				break;
			case 'C': //do not colorize
				conf->colorize = 0;
				break;
			case 's': //seconds
				conf->seconds = atoi(optarg);
				break;
			case 'c': //count times
				conf->count_times= atoi(optarg);
				break;
			case 'h': //fallthrough for help
			case '?': //same
			default:
				print_usage(argv);
				break;
		}
	}
}


void
get_meminfo(struct meminfo *mem_info)
{
	FILE *meminfo_file;
	char buffer[200];
	char tag[200], value[200], unit[200]; //yep, be sure

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
}

void
work_meminfo(struct meminfo *mem_info, struct conf_info *conf)
{
	work_central(mem_info, conf);
	/* TODO: Check if has swap !!!! */
	work_swap(mem_info, conf);
}

void
work_central(struct meminfo *mem, struct conf_info *conf)
{
	int total;
	int ceiled; //ceil the first time only !
	mem->mem_used = mem->mem_total -
	                mem->mem_free -
	                mem->buffers -
	                mem->cached; //FIXME - I was tired...
	/*Well, theoreticaly, buffers and cache are "memused", but we want to
	display them, so count them off */
	ceiled = 0;
	total  = 0;

	mem->pixels_mem_used =
	  proportionality(mem->mem_used, mem->mem_total, conf->bar_length,
	                  &ceiled);
	total += mem->pixels_mem_used;

	mem->pixels_mem_buffers =
	  proportionality(mem->buffers, mem->mem_total, conf->bar_length,
	                  &ceiled);
	total += mem->pixels_mem_buffers;

	mem->pixels_mem_cached =
	  proportionality(mem->cached, mem->mem_total, conf->bar_length,
	                  &ceiled);
	total += mem->pixels_mem_cached;

/*	mem->pixels_mem_free =
	  proportionality(mem->mem_free, mem->mem_total, conf->bar_length,
	                  &ceiled);
	*/
	mem->pixels_mem_free = conf->bar_length - total - 1;
	/* XXX FIXME: This IS crap... */
}


void
work_swap(struct meminfo *mem, struct conf_info *conf)
{
	mem->swap_used = mem->swap_total - mem->swap_free;

	mem->pixels_swap_free = (mem->swap_free * conf->bar_length) /
	                        mem->swap_total - 1;
	mem->pixels_swap_used = conf->bar_length - mem->pixels_swap_free -
	                        1; //hahaha, dirty
}


void
display_meminfo(struct meminfo *mem, struct conf_info *conf)
{

	/* DOING THE "RAM" FIRST */
	fputs(MEM_TAG"[", stdout);
	display_pixel(mem->pixels_mem_used, CHAR_USED, COLOR_USED);
	display_pixel(mem->pixels_mem_buffers, CHAR_BUFFERS, COLOR_BUFFERS);
	display_pixel(mem->pixels_mem_cached, CHAR_CACHED, COLOR_CACHED);
	display_pixel(mem->pixels_mem_free, CHAR_FREE, COLOR_FREE);
	fputs("]\n", stdout);

	printf("U: "COLOR_USED"%d"COLOR_NORMAL"%s, ", mem->mem_used, "Kb");
	printf("B: "COLOR_BUFFERS"%d"COLOR_NORMAL"%s, ", mem->buffers, "Kb");
	printf("C: "COLOR_CACHED"%d"COLOR_NORMAL"%s, ", mem->cached, "Kb");
	printf("F: "COLOR_FREE"%d"COLOR_NORMAL"%s\n", mem->mem_free, "Kb");
//		mem->mem_used, mem->buffers, mem->cached, mem->mem_free);

	fputs(SWAP_TAG"[", stdout);
	display_pixel(mem->pixels_swap_used, CHAR_USED, COLOR_USED);
	display_pixel(mem->pixels_swap_free, CHAR_FREE, COLOR_FREE);
	fputs("]\n", stdout);
	printf("U: "COLOR_USED"%d"COLOR_NORMAL"%s, ", mem->swap_used, "Kb");
	printf("F: "COLOR_FREE"%d"COLOR_NORMAL"%s\n", mem->swap_free, "Kb");

}

void
display_pixel(int times, char pixel, char *color)
{
	fputs(color, stdout);
	while(times-->0)
		putchar(pixel);
	fputs(COLOR_NORMAL, stdout);
}


void
print_usage(char **argv)
{
	fprintf(stderr, "Usage: %s blah blah\n", argv[0]);
	exit(EXIT_FAILURE);
}

/* UTILS */

void
get_tty_info(struct conf_info *conf)
{
	struct winsize win;

	if(!isatty(STDOUT_FILENO)) {
		conf->is_a_tty  = 0;
		conf->tty_width = 80;
		return;
	}
	else {
		if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) != 0) {
			fprintf(stderr, "Could not get TTY size !");
			win.ws_col = 80;
		}
		conf->is_a_tty  = 1;
		conf->tty_width = win.ws_col;
	}
}

void
explode_line(char *buffer, char *tag, char *value, char *unit)
{
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
}


void
insert_data(char *value, char *unit, unsigned int *where)
{
	if(strcmp(unit, PARSE_KILO_BYTES_UNIT)) {
		*where = atoi(value);
	}
	else //TODO, but should not happen
		printf("ERROR in insert_data");
}

void
convert_string_to_lower(char *s)
{
	while(*s) {
		if(*s >= 'A' && *s <= 'Z')
			*s += 'a'-'A';
		s++;
	}
}

int
proportionality(int have, int total, int ratio, int *ceiled)
{
	double n;
	int o;

	n = ((double)have * (double)(ratio))/((double)total);
	o = (int)n;
//	printf("%d, %lf", o, n);
	n -= o;
	if(n >= 0.5 && !(*ceiled)) {
//		printf("I");
		o++;
		*ceiled = 1;
	}
//	putchar('\n');
	return o;
}

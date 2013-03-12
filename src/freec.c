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
#include <string.h>

#include "freec.h"

int
main(int argc, char **argv)
{
	struct flagsinfo flags;
	struct meminfo memory_info;

	conf_default_flags(&flags);
	get_opts(argc, argv, &flags);

	while(flags.count_times) {
		get_meminfo(&memory_info);
		printf("memtot: %u\n", memory_info.mem_total);
		printf("memfree: %u\n", memory_info.mem_free);
		printf("buffers: %u\n", memory_info.buffers);
		printf("cached: %u\n", memory_info.cached);
		printf("swap_total: %u\n", memory_info.swap_total);
		printf("swap_free: %u\n", memory_info.swap_free);
//		work_meminfo(&memory_info);
//		display_meminfo(&memory_info);

		flags.count_times--;
		if(flags.count_times)
			sleep(flags.seconds);
	}

	//free everithing if needed

	return(EXIT_SUCCESS);
}

void
conf_default_flags(struct flagsinfo *flags)
{
	flags->size_unit   = MEGAS;
	flags->SI_unit     = 0;
	flags->humanize    = 1;
	flags->colorize    = 1;
	flags->seconds     = 1;
	flags->count_times = 1;
}


void
get_opts(int argc, char **argv, struct flagsinfo *flags)
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
				flags->size_unit = BYTES;
				break;
			case 'k': //Kilos
				flags->size_unit = KILOS;
				break;
			case 'm': //Megas
				flags->size_unit = MEGAS;
				break;
			case 'g': //Gigas
				flags->size_unit = GIGAS;
				break;
			case 'T': //Teras
				flags->size_unit = TERAS;
				break;
			case 'S': //SI units
				flags->SI_unit = 1;
				break;
			case 'C': //do not colorize
				flags->colorize = 0;
				break;
			case 's': //seconds
				flags->seconds = atoi(optarg);
				break;
			case 'c': //count times
				flags->count_times= atoi(optarg);
				break;
			case 'h': //fallthrough for help
			case '?':
			default:
				print_usage(argv);
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
		printf("ERROR");
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

void
print_usage(char **argv)
{
	fprintf(stderr, "Usage: %s blah blah\n", argv[0]);
	exit(EXIT_FAILURE);
}

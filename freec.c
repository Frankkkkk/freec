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
//		work_meminfo(&memory_info);
//		display_meminfo(&memory_info);

		flags.count_times--;
		sleep(flags.seconds);
	}

	//free everithing if needed

	return(EXIT_SUCCESS);
}

void
conf_default_flags(struct flagsinfo *flags)
{
	flags->size_unit  = MEGAS;
	flags->SI_unit    = 0;
	flags->humanize   = 1;
	flags->colorize   = 1;
	flags->seconds    = 1;
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
		perror("Could not open file "MEMINFO_FILE", are-you sure you're"\
		       "\non a Linux machine ? ");
		exit(EXIT_FAILURE);
	}

	while(fgets(buffer, 199, meminfo_file) != NULL) {
		printf("Read >%s<\n", buffer);
		explode_line(buffer, tag, value, unit);
		printf("Got: >%s< >%s< >%s<\n", tag, value, unit);
		//WORK ON IT
	}
}

void
explode_line(char *buffer, char *tag, char *value, char *unit)
{
	char *arg;

	arg = strtok(buffer, " ");
	strcpy(tag, arg);

	arg = strtok(buffer, " ");
	if(arg == NULL)
		printf("ERROR");
	strcpy(tag, arg);


	arg = strtok(buffer, " ");
	if(arg == NULL)
		unit = '\0';
	else
		strcpy(tag, arg);
}


void
print_usage(char **argv)
{
	fprintf(stderr, "Usage: %s blah blah\n", argv[0]);
	exit(EXIT_FAILURE);
}

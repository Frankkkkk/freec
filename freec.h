#ifndef H_FREE_INCLUDED
#define H_FREE_INCLUDED

/* START OF CONFIGURATION DEFINES */


/* You should not have to touch below */
#define BYTES 0
#define KILOS 3
#define MEGAS 6
#define GIGAS 9
#define TERAS 12

#define MEMINFO_FILE "/proc/meminfo"
struct flagsinfo
{
	char size_unit; //b, m, g, t
	char SI_unit; //si/S - 1, (0)
	char humanize; //h - (1), 0
	char colorize; //nocolor/C - (1), 0
	int seconds; //default 1 
	int count_times; //default 1 :p
};

struct meminfo
{
	//raw data - in kB !
	unsigned int mem_total;
	unsigned int mem_free;

	unsigned int swap_total;
	unsigned int swap_free;
};

void
conf_default_flags(struct flagsinfo *flags);

void
get_opts(int argc, char **argv, struct flagsinfo *flags);

void
get_meminfo(struct meminfo *mem_info);

void
explode_line(char *buffer, char *tag, char *value, char *unit);

void
print_usage(char **argv);

#endif

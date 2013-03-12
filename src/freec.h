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

//for parsing MEMINFO_FILE
#define PARSE_BYTES_UNIT "b"
#define PARSE_KILO_BYTES_UNIT "kb"
#define PARSE_MEGA_BYTES_UNIT "mb"
#define PARSE_GIGA_BYTES_UNIT "gb"
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
	//defines from /proc/meminfo
#define FREEC_MEM_TOTAL "memtotal:"
	unsigned int mem_total;
#define FREEC_MEM_FREE "memfree:"
	unsigned int mem_free;

#define FREEC_BUFFERS "buffers:"
	unsigned int buffers;
#define FREEC_CACHED "cached:"
	unsigned int cached;

#define FREEC_SWAP_TOTAL "swaptotal:"
	unsigned int swap_total;
#define FREEC_SWAP_FREE "swapfree:"
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
insert_data(char *value, char *unit, unsigned int *where);

void
convert_string_to_lower(char *s);

void
print_usage(char **argv);
#endif

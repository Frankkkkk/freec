#ifndef H_FREE_INCLUDED
#define H_FREE_INCLUDED

/* START OF CONFIGURATION */
/* List of available colors */
#define RED    "\033[;31m"
#define GREEN  "\033[;32m"
#define YELLOW "\033[;33m"
#define BLUE   "\033[;34m"
#define PURPLE "\033[;35m"
#define CYAN   "\033[;36m"
#define NORMAL "\033[;m"

/* Which ones do you want ? */
#define COLOR_USED    RED 
#define COLOR_BUFFERS YELLOW
#define COLOR_CACHED  PURPLE
#define COLOR_FREE    GREEN
#define COLOR_NORMAL  NORMAL



/* You should not need to touch below */
#define BYTES 0
#define KILOS 3
#define MEGAS 6
#define GIGAS 9
#define TERAS 12


#define MEMINFO_FILE "/proc/meminfo"

//for parsing MEMINFO_FILE
#define PARSE_BYTES_UNIT      "b"
#define PARSE_KILO_BYTES_UNIT "kb"
#define PARSE_MEGA_BYTES_UNIT "mb"
#define PARSE_GIGA_BYTES_UNIT "gb"
struct conf_info 
{
	char size_unit; //b, m, g, t
	char SI_unit; //si/S - 1, (0)
	char humanize; //h - (1), 0
	char colorize; //nocolor/C - (1), 0
	int seconds; //default 1 
	int count_times; //default 1 :p

	//and TTYINFO
	int is_a_tty;
	int tty_width;

	int bar_length;
};

struct meminfo
{
	//raw data - in kB !
	//defines from /proc/meminfo
#define MEM_TAG "Mem.:"
#define FREEC_MEM_TOTAL "memtotal:"
	unsigned int mem_total;
#define FREEC_MEM_FREE "memfree:"
	unsigned int mem_free;

	unsigned int mem_used;
#define FREEC_BUFFERS "buffers:"
	unsigned int buffers;
#define FREEC_CACHED "cached:"
	unsigned int cached;

	int pixels_mem_used;
	int pixels_mem_free;
	int pixels_mem_buffers;
	int pixels_mem_cached;
#define SWAP_TAG "Swap:"
#define FREEC_SWAP_TOTAL "swaptotal:"
	unsigned int swap_total;
#define FREEC_SWAP_FREE "swapfree:"
	unsigned int swap_free;

	int pixels_swap_used;
	int pixels_swap_free;

};

void
conf_default_flags(struct conf_info *conf);

void
get_opts(int argc, char **argv, struct conf_info *conf);

void
get_meminfo(struct meminfo *mem_info);

void
work_meminfo(struct meminfo *mem_info, struct conf_info *conf);

void
work_central(struct meminfo *mem, struct conf_info *conf);

void
work_swap(struct meminfo *mem, struct conf_info *conf);

void
display_meminfo(struct meminfo *mem, struct conf_info *conf);

void
get_tty_info(struct conf_info *conf);

void
explode_line(char *buffer, char *tag, char *value, char *unit);

void
insert_data(char *value, char *unit, unsigned int *where);

void
convert_string_to_lower(char *s);

void
print_usage(char **argv);

int
proportionality(int have, int total, int ratio);
#endif

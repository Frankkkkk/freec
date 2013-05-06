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

#define CHAR_USED    'U'
#define CHAR_BUFFERS 'b'
#define CHAR_CACHED  'c'
#define CHAR_FREE    'f'

#define BAR_LEN      30



/* You should not need to touch below */
#define BYTES -1 /* division coef. */
#define KILOS 0
#define MEGAS 1
#define GIGAS 2
#define TERAS 3


#define MEMINFO_FILE "/proc/meminfo"

/* for parsing MEMINFO_FILE */
#define PARSE_BYTES_UNIT      "b"
#define PARSE_KILO_BYTES_UNIT "kb"
#define PARSE_MEGA_BYTES_UNIT "mb"
#define PARSE_GIGA_BYTES_UNIT "gb"
struct conf_info 
{
	char size_unit; /* b, m, g, t */
	char SI_unit; /* si/S - 1, (0) */
	char humanize; /* h - (1), 0 */
	char colorize; /* nocolor/C - (1), 0 */
	char no_graph; /* nocolor/C - (1), 0 */
	unsigned int seconds; /* default 1 */
	int count_times; /* default 1 :p */

	/* and TTYINFO */
	int is_a_tty;
	unsigned int tty_width;

	unsigned int bar_length;
};

struct meminfo
{
	/* raw data - in kB ! */
	/* defines from /proc/meminfo */
#define MEM_TAG "Mem.: "
#define FREEC_MEM_TOTAL "memtotal:"
/* FIXME: unsigned long long ?? */
	unsigned int mem_total;
#define FREEC_MEM_FREE "memfree:"
	unsigned int mem_free;

	unsigned int mem_used;
#define FREEC_BUFFERS "buffers:"
	unsigned int buffers;
#define FREEC_CACHED "cached:"
	unsigned int cached;

	unsigned int pixels_mem_used;
	unsigned int pixels_mem_free;
	unsigned int pixels_mem_buffers;
	unsigned int pixels_mem_cached;

#define SWAP_TAG "Swap: "
#define FREEC_SWAP_TOTAL "swaptotal:"
	unsigned int swap_total;
#define FREEC_SWAP_FREE "swapfree:"
	unsigned int swap_free;

	unsigned int swap_used;

	unsigned int pixels_swap_used;
	unsigned int pixels_swap_free;

};

void
conf_default_flags(void);

void
get_opts(int argc, char **argv);

void
get_meminfo(struct meminfo *mem_info);

void
work_meminfo(struct meminfo *mem_info);

void
work_central(struct meminfo *mem);

void
work_swap(struct meminfo *mem);

void
display_meminfo(struct meminfo *mem);

void
get_tty_info(void);

void
display_pixel(unsigned int times, char pixel, char *color);

void
display_unit(unsigned int mem);

void
print_usage(char **argv);

void
display_help(void);

void
explode_line(char *buffer, char *tag, char *value, char *unit);

void
insert_data(char *value, char *unit, unsigned int *where);

void
convert_string_to_lower(char *s);

unsigned int
proportionality(unsigned int have, unsigned int total, unsigned int ratio,
		int *ceiled);
#endif

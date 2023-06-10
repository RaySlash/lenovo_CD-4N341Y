#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* external variables */
extern int softboot, watchdog, temp, maxtemp, tint, lastts, nrts;
extern int maxload1, maxload5, maxload15, load, verbose, mem, minpages;
extern int hbstamps, logtick, ticker;
extern pid_t pid;
extern char *tempname, *admin, *devname, *progname, *timestamps, *heartbeat;
extern time_t timeout, rtimeout;
extern FILE *hb;
extern char* logdir, *filename_buf;

/* variable types */
struct pingmode
{
	struct sockaddr to;
	int sock_fp;
	unsigned char *packet;
};

struct filemode
{
	int mtime;
};

struct ifmode
{
	unsigned long bytes;
};

union wdog_options
{
        struct pingmode net;
        struct filemode file;
        struct ifmode iface;
};
                                        
struct list
{
        char *name;
        union wdog_options parameter;
        struct list *next;
};

/* constants */
#define DATALEN         (64 - 8)
#define MAXIPLEN        60
#define MAXICMPLEN      76
#define MAXPACKET       (65536 - 60 - 8)        /* max packet size */

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define TS_SIZE	12

/* function prototypes */
int check_file_stat(struct list *);
int check_file_table(void);
int keep_alive(void);
int check_load(void);
int check_net(char *target, int sock_fp, struct sockaddr to, unsigned char *packet, int time, int count);
int check_temp(void);
int check_bin(char *, time_t, int);
int check_pidfile(struct list *);
int check_iface(struct list *);
int check_memory(void);
int custom_check(void);
void copy_fts_to_properties();
void copy_properties_to_fts();

void do_shutdown(int errorcode, bool requested);
void sigterm_handler(int arg);

#ifdef __GNUC__
#define GCC_NORETURN __attribute__((noreturn))
#else
#define GCC_NORETURN
#endif

void terminate(void) GCC_NORETURN;

#ifdef __cplusplus
}
#endif

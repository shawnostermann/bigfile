/* bigfile -- make big file with just one data byte */
/* Fri Apr  9, 1999 - added code for full 64-bit offsets! */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LONG_OFFSET
#undef LONG_OFFSET


/* offset type */
#ifdef  LONG_OFFSET
typedef long long int my_long_t;
#define LSEEK lseek64
#else /*  LONG_OFFSET  */
typedef long int my_long_t;
#define LSEEK lseek
#endif /*  LONG_OFFSET  */

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif


/* local routines */
static my_long_t getnum(char *str);
static void usage(char *prog);
static void Quit(char *why);


/* global variables */
int debug = 0;
int trunc_me = 0;
int i;
my_long_t size = -1;
my_long_t interval = -1;
char *file = NULL;
char *size_text;
int fd;


int
main(
     int argc,
     char *argv[])
{

    /* parse the args */
    for (i=1; i < argc; ++i) {
	if (strcmp(argv[i],"-i") == 0) {
	    interval = getnum(argv[i+1]);
	    ++i;
	} else if (strcmp(argv[i],"-d") == 0) {
	    ++debug;
	} else if (strcmp(argv[i],"-t") == 0) {
	    ++trunc_me;
	} else if (size == -1) {
	    size = getnum(argv[i]);
	    if (size <= 0)
		usage(argv[0]);
	    size_text = argv[i];
	} else if (!file) {
	    file = argv[i];
	} else {
	    usage(argv[0]);
	}
    }

    /* create the default file name if not specified */
    if (file == NULL) {
	if (size != -1) {
	    char fbuf[100];
	    sprintf(fbuf,"lf%s", size_text);
	    file = strdup(fbuf);
	}
    }

    if ((file == NULL) || (size == -1))
	usage(argv[0]);

    if (debug) {
	fprintf(stderr,"debug flag: %d\n", debug);
#ifdef LONG_OFFSET
	fprintf(stderr,"size: %lld\n", size);
	fprintf(stderr,"interval: %lld\n", interval);
#else /* LONG_OFFSET */
	fprintf(stderr,"size: %ld\n", (long)size);
	fprintf(stderr,"interval: %ld\n", (long)interval);
#endif /* LONG_OFFSET */
	fprintf(stderr,"file: %s\n", file);
    }


    if ((fd = open(file,O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE,0644)) < 0) {
	Quit(file);
    }


    if (trunc_me) {
	if (interval != -1) {
	    Quit("-f and -i are mutually exclusive");
	}

	if (ftruncate(fd,size) != 0) {
	    Quit("ftrunc_meate");
	}
    } else {
	if (interval > 1) {
	    my_long_t off = interval;

	    while (off < size) {
		if (debug)
#ifdef LONG_OFFSET
		    printf("Writing at offset %lld\n", off-1);
#else /* LONG_OFFSET */
		    printf("Writing at offset %ld\n", off-1);
#endif /* LONG_OFFSET */
		if (LSEEK(fd,off-1,SEEK_SET) == -1) {
		    Quit("lseek 1");
		}

		if (write(fd,"i",1) != 1) {
		    Quit("write 1");
		}
		off += interval;
	    }
	}


	if (debug)
#ifdef LONG_OFFSET
	    printf("Writing at offset %lld\n", size-1);
#else /* LONG_OFFSET */
	    printf("Writing at offset %ld\n", size-1);
#endif /* LONG_OFFSET */
	if (LSEEK(fd,size-1,SEEK_SET) == -1) {
	    Quit("lseek 2");
	}

	if (write(fd,"!",1) != 1) {
	    Quit("write 2");
	}
    }
    
    if (close(fd) != 0) {
	Quit("close");
    }
	

#ifdef  LONG_OFFSET 
    printf("Created file '%s' with size %lld bytes\n", file,size);
#else /* LONG_OFFSET */
    printf("Created file '%s' with size %ld bytes\n", file,(long)size);
#endif /* LONG_OFFSET */

    exit(0);
}


static my_long_t
getnum(
     char *str)
{
    double strtod();
    double n;
    char *str_end;

    n = strtod(str, &str_end);
    if (*str_end != '\00') {
	/* there was more after the arg */
	switch (*str_end) {
	  case 'k': n *= 1000.0; break;
	  case 'm': n *= 1000.0 * 1000.0; break;
	  case 'g': n *= 1000.0 * 1000.0 * 1000.0; break;
	  case 't': n *= 1000.0 * 1000.0 * 1000.0 * 1000.0; break;
	  case 'K': n *= 1024.0; break;
	  case 'M': n *= 1024.0 * 1024.0; break;
	  case 'G': n *= 1024.0 * 1024.0 * 1024.0; break;
	  case 'T': n *= 1024.0 * 1024.0 * 1024.0 * 1024.0; break;
	  default: {
	      fprintf(stderr,"illegal number: '%s'\n", str);
	      exit(-1);
	  }
	}
    }

    return((my_long_t)(n));
}

static void
Quit(char *why)
{
    perror(why);
    close(fd);  /* don't care if it fails */
    unlink(file);

    exit(-1);
}


static void
usage(char *prog)
{
    fprintf(stderr,"usage: %s [-d] [-i size] size [fname]\n", prog);
    fprintf(stderr,"\
  -d     whistle while you work\n\
  -t     create file with truncate syscall() instead\n\
  -i     intervals, write a character every \"size\" bytes\n\
  size   size of file to write\n\
         can append 'T' for Terabytes, 'G' for Gigabytes, 'M' for Megabytes,\n\
            or 'K' for Kilobytes\n\
         (lower case is base 1000, upper case is base 1024)\n\
  fname  file name to use, default is lf<size>\n\
");
    exit(-1);
}

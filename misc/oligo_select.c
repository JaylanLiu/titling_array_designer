// oligo_select.c -- select oligos from databases by predefined coverage value
// oligo_select -coverage <coverage size> | -window <window size> -bed target.bed -o select.txt <oligos.txt.gz>
// oligos.bed.gz: oligos databases generated by designer, in bed format (4 columns)
// coverage     : coverage value, greater than 0 and smaller than oligo length (window size == 1)
// window       : windows size for selecting oligos
// bed          : target regions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "utils.h"
#include "bedutils.h"

extern bed_handles_t *bed_hand;

#include <htslib/kstring.h>
#ifndef KSTRINT_INIT
#define KSTRINT_INIT { 0, 0, 0}
#endif

#include <htslib/tbx.h>

//#include<khash.h>
int usage()
{
    fprintf(stderr, "Usage: \n");
    fprintf(stderr, "oligo_select [ -coverage <coverage size> | -window <window size> ] -o select.txt oligo_data.bed.gz\n");
    fprintf(stderr, "  -coverage  Coverage value, greater than 0 and smaller than oligo length\n");
    fprintf(stderr, "  -window    Windows size for selecting oligos\n");
    fprintf(stderr, "  -bed       Target regions file in bed format.\n");
    fprintf(stderr, "  -quiet     Quiet mode, no info or log output.\n");
    fprintf(stderr, "oligo.txt.gz should be a bed format like database. Sorted by genome locations and preindex by tabix.\n");
    fprintf(stderr, "Author: shiquan@link.cuhk.edu.hk\n");
    return 1;
}
struct region_core {
    int32_t start;
    int32_t end;
};

struct regions_cache {
    int l, m;
    struct region_core *a;
    char *name;
    uint32_t start;
    uint32_t end;
    int32_t gap; // gap between nearest downstream region
};
struct args {
    float coverage;
    int window;
    int quiet_mode;
    const char* output_fname;
    const char* input_fname;
    const char* bed_fname;
    struct regions_cache *cache;
    tbx_t *tbx;
};
static struct args args = {
    .coverage = 0,
    .window = 0,
    .quiet_mode = 0,
    .output_fname = NULL,
    .input_fname = NULL,
    .bed_fname = NULL,
    .cache = NULL,
    .tbx = NULL,
};
struct regions_cache *regions_cache_init()
{
    struct regions_cache *cache = (struct regions_cache*)malloc(sizeof(struct regions_cache));
    cache->l = cache->m = 0;
    cache->a = NULL;
    cache->name = NULL;
    cache->start = cache->end = 0;
    cache->gap = 0;
    return cache;
}
int prase_argv(int argc, char **argv)
{
    if (argc == 0) {
	return -1;
    }
    int i;
    kstring_t buff = KSTRINT_INIT;
    for (i = 0; i < argc; ++i) {
	if (i) kputc(' ', &buff);
	kputs(argv[i], &buff);
    }

    const char *coverage = 0;
    const char *window = 0;
    for (i = 0; i < argc; ) {
	const char *a = argv[i++];
	if ( strcmp(a, "-help") == 0 || strcmp(a, "-h") == 0 ) {
	    return 1;
	} else if (strcmp(a, "-quiet") == 0) {
	    args.quiet_mode = 1;
	    continue;
	}

	const char **arg_var = 0;
	if (strcmp(a, "-o") == 0 && args.output_fname == 0)
	    arg_var = &args.output_fname;
	else if (strcmp(a, "-coverage") == 0 && coverage == 0)
	    arg_var = &coverage;
	else if (strcmp(a, "-window") == 0 && window == 0)
	    arg_var = &window;
	else if (strcmp(a, "-bed") == 0 && args.bed_fname == 0)
	    arg_var = &args.bed_fname;
	
	if (arg_var != 0) {
	    if (i == argc) {
		error_print("Missing an argument after %s.", a);
		return -2;
	    }
	    *arg_var = argv[i++];
	    continue;
	}

	if (args.input_fname == 0) {
	    args.input_fname = a;
	    continue;
	}
	error_print("Unknown arguement : %s", a);
	return -2;
    }
    if (args.bed == 0) {
	error_print("Must set a bed file by using -bed.");
	return -2;
    }
    if (coverage != 0) args.coverage = atof(coverage);
    if (window != 0) args.window = atoi(window);

    if (args.input_fname == 0 ) {
	if (!isatty(fileno(stdin))) {
	    args.input_fname == "-";
	} else {
	    error_print("No input oligo databases");
	    return -2;
	}
    }
    if (args.quiet_mode == 0) {	
	LOG_print("The program was compiled at %s %s by %s.", __DATE__, __TIME__, getenv("USER"));
	LOG_print("Args: %s", buff.s);
	if (args.window != 0) {
	    LOG_print("Set windows size to %d", args.window);
	    if (args.coverage != 0) {
		LOG_print("Cannot set coverage value and windows size at same time. Reset coverage value to 0.");
	    }
	} else {
	    if (args.coverage != 0) {
		LOG_print("Set coverage value to %.1f", args.coverage);
	    } else {
		LOG_print("Use default coverage value 1.5.");
		args.coverage = 1.5;
	    }
	}
    }
    if ( buff.m ) free(buff.s);
    
    return 0;
}

int main(int argc, char **argv)
{
    int ret;
    if ((ret = prase_argv(--argc, ++argv)) != 0) {
	// usage 
	if (ret == 1) {
	    LOG_print("See usage..");
	    return usage();
	} else if (ret == -1) { // empty arguments
	    LOG_print("Empty arguments. Please see usage carefully.");
	    return usage();
	} else { // failed to prase arguments
	    LOG_print("%s", BE_SMART_STRING);
	    return 0;
	}
    }
    bedaux_t bed = INIT_BED_EMPTY;
    int ret;
    int trim = BD_IS_FLANK;
    int check = BD_CHECK_NO;
    bed_hand->read(args.bed_fname, &bed, 0, 0, &ret, trim);
    bedaux_t *b = bed_hand->merge(&bed, &check);
    bed_hand->clear(&bed, destroy_void);
    
    int i;
    for (i = 0; i < bed.
    return 0;
}

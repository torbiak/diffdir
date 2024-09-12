#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "diffdir.h"

static char usage[] = "usage: diffdir [-h|--help] <dir_a> <dir_b>\n";

// Return a FILE pointer on success, and NULL on error.
static FILE*
open_writable_file(const char *name)
{
	FILE *f = fopen(name, "w");
	if (f == NULL) {
		fprintf(stderr, "open %s: %s\n", name, strerror(errno));
		perror(NULL);
	}
	return f;
}

// Close a file and log an error, if any, to stderr.
static void
close_file(FILE *f, const char *name)
{
	int err = fclose(f);
	if (err != 0) {
		fprintf(stderr, "close %s: %s\n", name, strerror(errno));
		perror(NULL);
	}
}

// Check that the given filename can be accessed and exit if not.
static void
dir_accessible_or_die(char *filename)
{
	if (access(filename, R_OK) != 0) {
		fprintf(stderr, "directory %s not accessible: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

int
main(int argc, char *argv[])
{
	int c;
	int help_flag = 0;
	struct option long_options[] = {
		{"help", no_argument, &help_flag, 1},
		{0, 0, 0, 0}
	};
	while ((c = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
		switch (c) {
			case 'h':
				help_flag = 1;
				break;
			case '?':
				// getopt prints its own error message
				exit(EXIT_FAILURE);
			default:
				break;
		}
	}
	if (help_flag) {
		fputs(usage, stdout);
		exit(EXIT_SUCCESS);
	}
	if (argc - optind < 2) {
		fputs(usage, stderr);
		fputs("\nwrong number of arguments\n", stderr);
		exit(EXIT_FAILURE);
	}
	char *dir_a = argv[optind++];
	char *dir_b = argv[optind++];

	// Check that the given directories exist before making output files.
	dir_accessible_or_die(dir_a);
	dir_accessible_or_die(dir_b);

	int rc = EXIT_SUCCESS;
	char *err = NULL;
	FILE *common = NULL, *a_only = NULL, *b_only = NULL;

	common = open_writable_file("common");
	if (common == NULL) { rc = EXIT_FAILURE; goto cleanup; }
	a_only = open_writable_file("a_only");
	if (common == NULL) { rc = EXIT_FAILURE; goto cleanup; }
	b_only = open_writable_file("b_only");
	if (common == NULL) { rc = EXIT_FAILURE; goto cleanup; }

	err = diffdir(dir_a, dir_b, common, a_only, b_only);
	if (err) {
		fprintf(stderr, "%s\n", err);
		rc = EXIT_FAILURE;
		goto cleanup;
	}

cleanup:
	if (err) free(err);
	if (common) close_file(common, "common");
	if (a_only) close_file(a_only, "a_only");
	if (b_only) close_file(b_only, "b_only");

	return rc;
}

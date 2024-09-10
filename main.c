#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diffdir.h"

// Return zero on success, and nonzero on error.
static int
parse_args(int argc, char *argv[], char **dir_a, char **dir_b)
{
	// TODO
	if (argc != 3) {
		fprintf(stderr, "wrong number of args");
		exit(1);
	}
	*dir_a = argv[1];
	*dir_b = argv[2];
	return 0;  // TODO
}


// Return a FILE pointer on success, and NULL on error.
static FILE *
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

int
main(int argc, char *argv[])
{
	char *dir_a = NULL, *dir_b = NULL;
	int err = 0;
	int rc = EXIT_SUCCESS;

	err = parse_args(argc, argv, &dir_a, &dir_b);
	if (err) {
		rc = EXIT_FAILURE;
		goto cleanup;
	}

	FILE *common = open_writable_file("common");
	if (common == NULL) { goto cleanup; }
	FILE *a_only = open_writable_file("a_only");
	if (a_only == NULL) { goto cleanup; }
	FILE *b_only = open_writable_file("b_only");
	if (b_only == NULL) { goto cleanup; }

	err = diffdir(dir_a, dir_b, common, a_only, b_only);
	if (err) {
		perror("diff dirs");
		goto cleanup;
	}

cleanup:
	if (common) { close_file(common, "common"); }
	if (a_only) { close_file(a_only, "a_only"); }
	if (b_only) { close_file(b_only, "b_only"); }
	if (dir_a) { free(dir_a); }
	if (dir_b) { free(dir_b); }

	return rc;
}

#include "diffdir.h"

// Return zero on success, and return non-zero and set errno on error.
static int
_diffdir(
	const char *root_a,
	const char *subdir_a,
	const char *root_b,
	const char *subdir_b,
	FILE *common,
	FILE *a_only,
	FILE *b_only)
{
	(void)root_a; (void)subdir_a; (void)root_b; (void)subdir_b; (void)common; (void)a_only; (void)b_only;
	printf("not actually diffing anything");
	return 0;  // TODO
}

// Return zero on success, and return non-zero and set errno on error.
int
diffdir(
	const char *dir_a,
	const char *dir_b,
	FILE *common,
	FILE *a_only,
	FILE *b_only)
{
	return _diffdir(dir_a, "", dir_b, "", common, a_only, b_only);
}

#ifndef DIFFDIR_H
#define DIFFDIR_H

#include <stdio.h>

char*
diffdir(
	const char *dir_a,
	const char *dir_b,
	FILE *common,
	FILE *a_only,
	FILE *b_only);

#endif  /* DIFFDIR_H */

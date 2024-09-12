#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "diffdir.h"

static void
die (int line_number, const char *format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	fprintf(stderr, "%d: ", line_number);
	vfprintf(stderr, format, vargs);
	fprintf(stderr, "\n");
	va_end(vargs);
	abort();
}

static void*
xmalloc(size_t size)
{
	void *p = malloc(size);
	if (p == NULL) {
		die(__LINE__, "out of memory: malloc failed");
	}
	return p;
}

static char*
sprintf_alloc(const char *format, ...)
{
	char *s;
	va_list vargs;
	va_start(vargs, format);
	int n = vasprintf(&s, format, vargs);
	va_end(vargs);
	if (n == -1 || s == NULL) {
		die(__LINE__, "vasprintf failed: likely out of memory");
	}
	return s;
}

// Join two paths with a slash and return a malloc'd string.
static char*
join(const char *root, const char *rel_path)
{
	char *joined = NULL;
	int root_len = strlen(root);
	int need_slash = root_len > 0 && root[root_len - 1] != '/';
	joined = sprintf_alloc("%s%s%s", root, need_slash ? "/" : "", rel_path);
	return joined;
}

static int
scandir_filter(const struct dirent *dirent)
{
	return strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0;
}

static char*
scandir_chk(const char *dir, struct dirent ***restrict namelist, int *n)
{
	char *err = NULL;
	*n = scandir(dir, namelist, scandir_filter, alphasort);
	if (*n == -1) {
		err = sprintf_alloc("scandir: %s", strerror(errno));
	}
	return err;
}

static char*
stat_chk(const char *restrict path, struct stat *restrict buf)
{
	char *err = NULL;
	int res = stat(path, buf);
	if (res == -1) {
		err = sprintf_alloc("stat %s: %s", path, strerror(errno));
	}
	return err;
}

static void
free_namelist(struct dirent **namelist, int len)
{
	int i = len;
	while (i--) {
		free(namelist[i]);
	}
	free(namelist);
}

// Return a malloc'd error string on error. On sucess, NULL is returned and
// is_same is set to 1 if the given files are byte-for-byte identical, and 0
// otherwise.
static char*
cmp_file_contents(
		const char *filename_a,
		const char *filename_b,
		int *is_same)
{
	*is_same = 0;
    char *err = NULL;
	unsigned char *buf_a = xmalloc(BUFSIZ);
	unsigned char *buf_b = xmalloc(BUFSIZ);

	FILE *file_a = fopen(filename_a, "r");
	if (file_a == NULL) {
		err = sprintf_alloc("open %s: %s", filename_a, strerror(errno));
        goto cleanup;
	}
	FILE *file_b = fopen(filename_b, "r");
	if (file_b == NULL) {
		err = sprintf_alloc("open %s: %s", filename_b, strerror(errno));
        goto cleanup;
	}

	int eof_a = 0, eof_b = 0;
	while (!eof_a && !eof_b) {
		int n_a = fread(buf_a, 1, BUFSIZ, file_a);
		if (n_a < BUFSIZ) {
			if (ferror(file_a)) {
				err = sprintf_alloc("error reading file: %s", filename_a);
                goto cleanup;
			} else {
				eof_a = 1;
			}
		}
		int n_b = fread(buf_b, 1, BUFSIZ, file_b);
		if (n_b < BUFSIZ) {
			if (ferror(file_b)) {
				err = sprintf_alloc("error reading file: %s", filename_b);
                goto cleanup;
			} else {
				eof_b = 1;
			}
		}

		if (n_a != n_b) {
            goto cleanup;
		}

		if (memcmp(buf_a, buf_b, n_a)) {
            goto cleanup;
		}
	}
	*is_same = 1;

cleanup:
	if (buf_a) free(buf_a);
	if (buf_b) free(buf_b);
	return err;
}

// Return NULL on success, or a malloc'd error message on error.
static char*
_diffdir(
	const char *root_a,
	const char *rel_dir_a,
	const char *root_b,
	const char *rel_dir_b,
	FILE *common,
	FILE *a_only,
	FILE *b_only)
{
	// Path to the current dir, including the root.
	char *dir_a = NULL, *dir_b = NULL;
	// Path to the current file, including the root.
	char *path_a = NULL, *path_b = NULL;
	// Path to the current file, relative to the root.
	char *rel_path_a = NULL, *rel_path_b = NULL;
	char *err = NULL;

	dir_a = join(root_a, rel_dir_a);
	dir_b = join(root_b, rel_dir_b);
	printf("dir_a=[%s] dir_b=[%s]\n", dir_a, dir_b);  // TODO: remove

	struct dirent **names_a = NULL, **names_b = NULL;
	int n_a = 0, n_b = 0;
	err = scandir_chk(dir_a, &names_a, &n_a);
	if (err) goto cleanup;
	err = scandir_chk(dir_b, &names_b, &n_b);
	if (err) goto cleanup;

	int i_a = 0, i_b = 0;
	while (i_a < n_a || i_b < n_b) {
		// If we run out of names on one side, use up the other side.
		if (i_b >= n_b) {
			if (rel_path_a) free(rel_path_a);
			rel_path_a = join(rel_dir_a, names_a[i_a]->d_name);
			fprintf(a_only, "%s\n", rel_path_a);
			++i_a;
			printf("rel_path_a=[%s]\n", rel_path_a);  // TODO: remove
			continue;
		} else if (i_b >= n_b) {
			if (rel_path_b) free(rel_path_b);
			rel_path_b = join(rel_dir_b, names_b[i_b]->d_name);
			fprintf(b_only, "%s\n", rel_path_b);
			++i_b;
			printf("rel_path_b=[%s]\n", rel_path_b);  // TODO: remove
			continue;
		} else {
			if (rel_path_a) free(rel_path_a);
			rel_path_a = join(rel_dir_a, names_a[i_a]->d_name);
			if (rel_path_b) free(rel_path_b);
			rel_path_b = join(rel_dir_b, names_b[i_b]->d_name);
			printf("rel_path_a=[%s] rel_path_b=[%s]\n", rel_path_a, rel_path_b);  // TODO: remove
		}

		// If a name is only on one side, write it to the appropriate file and
		// continue.
		int cmp = strcmp(names_a[i_a]->d_name, names_b[i_b]->d_name);
		if (cmp < 0) {
			fprintf(a_only, "%s\n", rel_path_a);
			++i_a;
			continue;
		} else if (cmp > 0) {
			fprintf(b_only, "%s\n", rel_path_b);
			++i_b;
			continue;
		}

		// The same name is on both sides: choose whether to recurse into dirs
		// and compare bytes of files if needed.

		if (path_a) free(path_a);
		path_a = join(dir_a, names_a[i_a]->d_name);
		if (path_b) free(path_b);
		path_b = join(dir_b, names_b[i_b]->d_name);
		if (strchr(path_a, '\n') || strchr(path_b, '\n')) {
			// The output files are newline-delimited, so filenames that
			// contain newlines would corrupt the output.
			err = sprintf_alloc("error: filename contains newline");
			goto cleanup;
		}
		printf("path_a=[%s] path_b=[%s]\n", path_a, path_b);  // TODO: remove
		
		struct stat stat_a, stat_b;
		err = stat_chk(path_a, &stat_a);
		if (err) goto cleanup;
		err = stat_chk(path_b, &stat_b);
		if (err) goto cleanup;

		int is_same = 0;
		int is_dir_a = S_ISDIR(stat_a.st_mode);
		int is_dir_b = S_ISDIR(stat_b.st_mode);
		if (is_dir_a && is_dir_b) {
			is_same = 1;
		} else if (stat_a.st_size == stat_b.st_size) {
			err = cmp_file_contents(path_a, path_b, &is_same);
			printf("cmp_file_contents(%s, %s) = %d\n", path_a, path_b, is_same);  // TODO: remove
			if (err) goto cleanup;
		}
		if (is_same) {
			// Same name and contents.
			fprintf(common, "%s\n", rel_path_a);
		} else {
			// Same name on both sides, but they don't have the same contents,
			// or one is a dir and the other isn't.
			fprintf(a_only, "%s\n", rel_path_a);
			fprintf(b_only, "%s\n", rel_path_b);
		}
		if (is_same && is_dir_a && is_dir_b) {
			// Both sides are dirs: recurse.
			printf("recurse into %s\n", rel_path_a);  // TODO: remove
			err = _diffdir(dir_a, rel_path_a, dir_b, rel_path_b, common, a_only, b_only);
			if (err) goto cleanup;
		}
		++i_a;
		++i_b;
	}

cleanup:
	if (dir_a) free(dir_a);
	if (dir_b) free(dir_b);
	if (names_a) free_namelist(names_a, n_a);
	if (names_b) free_namelist(names_b, n_b);
	if (path_a) free(path_a);
	if (path_b) free(path_b);
	if (rel_path_a) free(rel_path_a);
	if (rel_path_b) free(rel_path_b);
	return err;
}

// Return NULL on success, or a malloc'd error message on error.
char*
diffdir(
	const char *dir_a,
	const char *dir_b,
	FILE *common,
	FILE *a_only,
	FILE *b_only)
{
	char *err = _diffdir(dir_a, "", dir_b, "", common, a_only, b_only);
	if (err) {
		char *wrap_err = NULL;
		wrap_err = sprintf_alloc("diffdir: %s", err);
		free(err);
		return wrap_err;
	}
	return err;
}

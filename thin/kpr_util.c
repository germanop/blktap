#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define PFX_SIZE 5

/**
 * This function parse the given path and populate the other char arrays
 * with volume group and logical volume names (if any).
 * The accepted patter is the following
 * /dev/<name1>/<name2> where <name1> and <name2> can contain any
 * char other than '/' and are interpreted, respectively as VG name
 * and LV name. Any other pattern is wrong (/dev/mapper/<name> is accepted
 * but it is not what you want..).
 * 
 * @param[in] path is a NULL terminated char array containing the path to parse
 * @param[out] vg if successful contains VG name. Caller must ensure allocated
 * space is bigger than #path
 * @param[out] lv if successful contains LV name. Caller must ensure allocated
 * space is bigger than #path
 * @return 0 if OK and 1 otherwise. On exit all the array are NULL terminated
 */
int
kpr_split_lvm_path(const char * path, char * vg, char * lv)
{
	static const char dev_pfx[PFX_SIZE] = "/dev/"; /* no \0 */
	bool flag = false;

	if( strncmp(dev_pfx, path, PFX_SIZE) ) {
		fprintf(stderr, "Not a device pattern\n");
		return 1;
	}
	path += PFX_SIZE;

	/* Extract VG */
	for( ; *path; ++path, ++vg ) {
		if( *path == '/' ) {
			*vg = '\0';
			break;
		}
		*vg = *path;
		flag = true;
	}

	/* Check why and how the loop ended */
	if ( *path == '\0' || !flag ) {
		/* terminate strings and error */
		*vg = '\0';
		*lv = '\0';
		return 1;
	}

	/* Extract LV */
	++path; /* skip slash */
	for( flag = false; *path; ++path, ++lv ) {
		if( *path == '/' ) {
			fprintf(stderr, "too many slashes\n");
			*lv = '\0';
			return 1;
		}
		*lv = *path;
		flag = true;
	}
	*lv = '\0'; /* terminate string */

	return flag ? 0 : 1;
}

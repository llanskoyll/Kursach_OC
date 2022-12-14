/**
 * File: XBM IO
 *
 * Read and write XBM images.
 */

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "gd.h"
#include "gd_errors.h"
#include "gdhelpers.h"

#define MAX_XBM_LINE_SIZE 255


/*
  Function: gdImageCreateFromXbm

    <gdImageCreateFromXbm> is called to load images from X bitmap
    format files. Invoke <gdImageCreateFromXbm> with an already opened
    pointer to a file containing the desired
    image. <gdImageCreateFromXbm> returns a <gdImagePtr> to the new
    image, or NULL if unable to load the image (most often because the
    file is corrupt or does not contain an X bitmap format
    image). <gdImageCreateFromXbm> does not close the file.

    You can inspect the sx and sy members of the image to determine
    its size. The image must eventually be destroyed using
    <gdImageDestroy>.

    X11 X bitmaps (which define a char[]) as well as X10 X bitmaps (which define
    a short[]) are supported.

  Parameters:

    fd - The input FILE pointer

  Returns:

    A pointer to the new image or NULL if an error occurred.

  Example:
    (start code)

    gdImagePtr im;
    FILE *in;
    in = fopen("myxbm.xbm", "rb");
    im = gdImageCreateFromXbm(in);
    fclose(in);
    // ... Use the image ...
    gdImageDestroy(im);

    (end code)
*/
BGD_DECLARE(gdImagePtr) gdImageCreateFromXbm(FILE * fd)
{
	char fline[MAX_XBM_LINE_SIZE];
	char iname[MAX_XBM_LINE_SIZE];
	char *type;
	int value;
	unsigned int width = 0, height = 0;
	int fail = 0;
	int max_bit = 0;

	gdImagePtr im;
	int bytes = 0, i;
	int bit, x = 0, y = 0;
	int ch;
	char h[8];
	unsigned int b;

	rewind(fd);
	while (fgets(fline, MAX_XBM_LINE_SIZE, fd)) {
		fline[MAX_XBM_LINE_SIZE-1] = '\0';
		if (strlen(fline) == MAX_XBM_LINE_SIZE-1) {
			return 0;
		}
		if (sscanf(fline, "#define %s %d", iname, &value) == 2) {
			if (!(type = strrchr(iname, '_'))) {
				type = iname;
			} else {
				type++;
			}

			if (!strcmp("width", type)) {
				width = (unsigned int) value;
			}
			if (!strcmp("height", type)) {
				height = (unsigned int) value;
			}
		} else {
			if ( sscanf(fline, "static unsigned char %s = {", iname) == 1
			  || sscanf(fline, "static char %s = {", iname) == 1)
			{
				max_bit = 128;
			} else if (sscanf(fline, "static unsigned short %s = {", iname) == 1
					|| sscanf(fline, "static short %s = {", iname) == 1)
			{
				max_bit = 32768;
			}
			if (max_bit) {
                bytes = (width + 7) / 8 * height;
				if (!bytes) {
					return 0;
				}
				if (!(type = strrchr(iname, '_'))) {
					type = iname;
				} else {
					type++;
				}
				if (!strcmp("bits[]", type)) {
					break;
				}
			}
 		}
	}
	if (!bytes || !max_bit) {
		return 0;
	}

	if(!(im = gdImageCreate(width, height))) {
		return 0;
	}
	gdImageColorAllocate(im, 255, 255, 255);
	gdImageColorAllocate(im, 0, 0, 0);
	h[2] = '\0';
	h[4] = '\0';
	for (i = 0; i < bytes; i++) {
		while (1) {
			if ((ch=getc(fd)) == EOF) {
				fail = 1;
				break;
			}
			if (ch == 'x') {
				break;
			}
		}
		if (fail) {
			break;
		}
		/* Get hex value */
		if ((ch=getc(fd)) == EOF) {
			break;
		}
		h[0] = ch;
		if ((ch=getc(fd)) == EOF) {
			break;
		}
		h[1] = ch;
		if (max_bit == 32768) {
			if ((ch=getc(fd)) == EOF) {
				break;
			}
			h[2] = ch;
			if ((ch=getc(fd)) == EOF) {
				break;
			}
			h[3] = ch;
		}
		if (sscanf(h, "%x", &b) != 1) {
			gd_error("invalid XBM");
			gdImageDestroy(im);
			return 0;
		}
		for (bit = 1; bit <= max_bit; bit = bit << 1) {
			gdImageSetPixel(im, x++, y, (b & bit) ? 1 : 0);
			if (x == im->sx) {
				x = 0;
				y++;
				if (y == im->sy) {
					return im;
				}
				break;
			}
		}
	}

	gd_error("EOF before image was complete");
	gdImageDestroy(im);
	return 0;
}


/* {{{ gdCtxPrintf */
static void gdCtxPrintf(gdIOCtx * out, const char *format, ...)
{
	char buf[1024];
	int len;
	va_list args;

	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf)-1, format, args);
	va_end(args);
	out->putBuf(out, buf, len);
}
/* }}} */

/* The compiler will optimize strlen(constant) to a constant number. */
#define gdCtxPuts(out, s) out->putBuf(out, s, strlen(s))


/**
 * Function: gdImageXbmCtx
 *
 *  Writes an image to an IO context in X11 bitmap format.
 *
 * Parameters:
 *
 *  image     - The <gdImagePtr> to write.
 *  file_name - The prefix of the XBM's identifiers. Illegal characters are
 *              automatically stripped.
 *  gd        - Which color to use as forground color. All pixels with another
 *              color are unset.
 *  out       - The <gdIOCtx> to write the image file to.
 *
 */
BGD_DECLARE(void) gdImageXbmCtx(gdImagePtr image, char* file_name, int fg, gdIOCtx * out)
{
	int x, y, c, b, sx, sy, p;
	char *name, *f;
	size_t i, l;

	name = file_name;
	if ((f = strrchr(name, '/')) != NULL) name = f+1;
	if ((f = strrchr(name, '\\')) != NULL) name = f+1;
	name = strdup(name);
	if ((f = strrchr(name, '.')) != NULL && !strcasecmp(f, ".XBM")) *f = '\0';
	if ((l = strlen(name)) == 0) {
		free(name);
		name = strdup("image");
	} else {
		for (i=0; i<l; i++) {
			/* only in C-locale isalnum() would work */
			if (!isupper(name[i]) && !islower(name[i]) && !isdigit(name[i])) {
				name[i] = '_';
			}
		}
	}

	/* Since "name" comes from the user, run it through a direct puts.
	 * Trying to printf it into a local buffer means we'd need a large
	 * or dynamic buffer to hold it all. */

	/* #define <name>_width 1234 */
	gdCtxPuts(out, "#define ");
	gdCtxPuts(out, name);
	gdCtxPuts(out, "_width ");
	gdCtxPrintf(out, "%d\n", gdImageSX(image));

	/* #define <name>_height 1234 */
	gdCtxPuts(out, "#define ");
	gdCtxPuts(out, name);
	gdCtxPuts(out, "_height ");
	gdCtxPrintf(out, "%d\n", gdImageSY(image));

	/* static unsigned char <name>_bits[] = {\n */
	gdCtxPuts(out, "static unsigned char ");
	gdCtxPuts(out, name);
	gdCtxPuts(out, "_bits[] = {\n  ");

	free(name);

	b = 1;
	p = 0;
	c = 0;
	sx = gdImageSX(image);
	sy = gdImageSY(image);
	for (y = 0; y < sy; y++) {
		for (x = 0; x < sx; x++) {
			if (gdImageGetPixel(image, x, y) == fg) {
				c |= b;
			}
			if ((b == 128) || (x == sx - 1)) {
				b = 1;
				if (p) {
					gdCtxPuts(out, ", ");
					if (!(p%12)) {
						gdCtxPuts(out, "\n  ");
						p = 12;
					}
				}
				p++;
				gdCtxPrintf(out, "0x%02X", c);
				c = 0;
			} else {
				b <<= 1;
			}
		}
	}
	gdCtxPuts(out, "};\n");
}

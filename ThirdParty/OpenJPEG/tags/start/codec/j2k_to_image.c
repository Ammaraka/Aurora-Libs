/* Copyright (c) 2001 David Janssens
 * Copyright (c) 2002 Yannick Verschueren
 * Copyright (c) 2002 Communications and remote sensing Laboratory, Universite catholique de Louvain, Belgium
 * 
 * All rights reserved. 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */



#include <openjpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int ceildiv(int a, int b)
{
	return (a + b - 1) / b;
}

int main(int argc, char **argv)
{
	FILE *f;
	char *src;
	char *dest, S1, S2, S3;
	int len;
	j2k_image_t *img;
	j2k_cp_t *cp;
	int w, h, max;
	int i, image_type = -1;

	if (argc < 3) {
		fprintf(stderr, "usage: %s j2k-file pnm-file\n", argv[0]);
		return 1;
	}

	f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "failed to open %s for reading\n", argv[1]);
		return 1;
	}

	dest = argv[2];

	while (*dest && *dest != '.') {
		dest++;
	}
	dest++;
	S1 = *dest;
	dest++;
	S2 = *dest;
	dest++;
	S3 = *dest;

	if ((S1 == 'p' && S2 == 'g' && S3 == 'x')
			|| (S1 == 'P' && S2 == 'G' && S3 == 'X')) {
		image_type = 0;
	}

	if ((S1 == 'p' && S2 == 'n' && S3 == 'm')
			|| (S1 == 'P' && S2 == 'N' && S3 == 'M') || (S1 == 'p' && S2 == 'g'
																									 && S3 == 'm')
			|| (S1 == 'P' && S2 == 'G' && S3 == 'M') || (S1 == 'P' && S2 == 'P'
																									 && S3 == 'M')
			|| (S1 == 'p' && S2 == 'p' && S3 == 'm')) {
		image_type = 1;
	}

	if ((S1 == 'b' && S2 == 'm' && S3 == 'p')
			|| (S1 == 'B' && S2 == 'M' && S3 == 'P')) {
		image_type = 2;
	}
	if (image_type == -1) {
		fprintf(stderr,
						"\033[0;33m!! Unrecognized format for infile [accept only *.pnm, *.pgm, *.ppm, *.pgx or *.bmp] !!\033[0;39m\n\n");
		return 1;
	}

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);
	src = (char *) malloc(len);
	fread(src, 1, len, f);
	fclose(f);

	if (!j2k_decode(src, len, &img, &cp)) {
		fprintf(stderr, "j2k_to_image: failed to decode image!\n");
		return 1;
	}
	free(src);

	/* ------------------  CREATE OUT IMAGE WITH THE RIGHT FORMAT ----------------------- */

	/* ------------------------ / */
	/* / */
	/* FORMAT : PNM, PGM or PPM / */
	/* / */
	/* ------------------------ / */

	if (image_type == 1) {				/* PNM PGM PPM */
		if (img->numcomps == 3 && img->comps[0].dx == img->comps[1].dx
				&& img->comps[1].dx == img->comps[2].dx
				&& img->comps[0].dy == img->comps[1].dy
				&& img->comps[1].dy == img->comps[2].dy
				&& img->comps[0].prec == img->comps[1].prec
				&& img->comps[1].prec == img->comps[2].prec) {
			f = fopen(argv[2], "wb");
			w = ceildiv(img->x1 - img->x0, img->comps[0].dx);
			h = ceildiv(img->y1 - img->y0, img->comps[0].dy);
			max = (1 << img->comps[0].prec) - 1;
			fprintf(f, "P6\n%d %d\n%d\n", w, h, max);
			for (i = 0; i < w * h; i++) {
				char r, g, b;
				r = img->comps[0].data[i];
				g = img->comps[1].data[i];
				b = img->comps[2].data[i];
				fprintf(f, "%c%c%c", r, g, b);
			}
			fclose(f);
		} else {
			int compno;
			for (compno = 0; compno < img->numcomps; compno++) {
				char name[256];
				if (img->numcomps > 1) {
					sprintf(name, "%d.%s", compno, argv[2]);
				} else {
					sprintf(name, "%s", argv[2]);
				}

				f = fopen(name, "wb");

				w = ceildiv(img->x1 - img->x0, img->comps[compno].dx);
				h = ceildiv(img->y1 - img->y0, img->comps[compno].dy);
				max = (1 << img->comps[compno].prec) - 1;
				fprintf(f, "P5\n%d %d\n%d\n", w, h, max);
				for (i = 0; i < w * h; i++) {
					char l;
					l = img->comps[compno].data[i];
					fprintf(f, "%c", l);
				}
				fclose(f);
			}
		}
	} else
		/* ------------------------ / */
		/* / */
		/* FORMAT : PGX             / */
		/* / */
		/* ------------------------ / */

	if (image_type == 0) {				/* PGX */
		int compno;
		for (compno = 0; compno < img->numcomps; compno++) {
			j2k_comp_t *comp = &img->comps[compno];
			char name[256];
			/* sprintf(name, "%s-%d.pgx", argv[2], compno); */
			sprintf(name, "%s", argv[2]);
			f = fopen(name, "wb");
			w = ceildiv(img->x1 - img->x0, comp->dx);
			h = ceildiv(img->y1 - img->y0, comp->dy);
			fprintf(f, "PG LM %c %d %d %d\n", comp->sgnd ? '-' : '+', comp->prec,
							w, h);
			for (i = 0; i < w * h; i++) {
				int v = img->comps[compno].data[i];
				if (comp->prec <= 8) {
					char c = (char) v;
					fwrite(&c, 1, 1, f);
				} else if (comp->prec <= 16) {
					short s = (short) v;
					fwrite(&s, 2, 1, f);
				} else {
					fwrite(&v, 4, 1, f);
				}
			}
			fclose(f);
		}
	} else
		/* ------------------------ / */
		/* / */
		/* FORMAT : BMP             / */
		/* / */
		/* ------------------------ / */


	if (image_type == 2) {				/* BMP */
		if (img->numcomps == 3 && img->comps[0].dx == img->comps[1].dx
				&& img->comps[1].dx == img->comps[2].dx
				&& img->comps[0].dy == img->comps[1].dy
				&& img->comps[1].dy == img->comps[2].dy
				&& img->comps[0].prec == img->comps[1].prec
				&& img->comps[1].prec == img->comps[2].prec) {
			/* -->> -->> -->> -->>

			   24 bits color

			   <<-- <<-- <<-- <<-- */

			f = fopen(argv[2], "wb");
			w = ceildiv(img->x1 - img->x0, img->comps[0].dx);
			h = ceildiv(img->y1 - img->y0, img->comps[0].dy);

			fprintf(f, "BM");

			/* FILE HEADER */
			/* ------------- */
			fprintf(f, "%c%c%c%c",
							(unsigned char) (h * w * 3 + 3 * h * (w % 2) + 54) & 0xff,
							(unsigned char) ((h * w * 3 + 3 * h * (w % 2) + 54) >> 8) &
							0xff,
							(unsigned char) ((h * w * 3 + 3 * h * (w % 2) + 54) >> 16) &
							0xff,
							(unsigned char) ((h * w * 3 + 3 * h * (w % 2) + 54) >> 24) &
							0xff);
			fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
							((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (54) & 0xff, ((54) >> 8) & 0xff,
							((54) >> 16) & 0xff, ((54) >> 24) & 0xff);
			/* INFO HEADER */
			/* ------------- */
			fprintf(f, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,
							((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (unsigned char) ((w) & 0xff),
							(unsigned char) ((w) >> 8) & 0xff,
							(unsigned char) ((w) >> 16) & 0xff,
							(unsigned char) ((w) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (unsigned char) ((h) & 0xff),
							(unsigned char) ((h) >> 8) & 0xff,
							(unsigned char) ((h) >> 16) & 0xff,
							(unsigned char) ((h) >> 24) & 0xff);
			fprintf(f, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
			fprintf(f, "%c%c", (24) & 0xff, ((24) >> 8) & 0xff);
			fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
							((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c",
							(unsigned char) (3 * h * w + 3 * h * (w % 2)) & 0xff,
							(unsigned char) ((h * w * 3 + 3 * h * (w % 2)) >> 8) & 0xff,
							(unsigned char) ((h * w * 3 + 3 * h * (w % 2)) >> 16) & 0xff,
							(unsigned char) ((h * w * 3 + 3 * h * (w % 2)) >> 24) &
							0xff);
			fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
							((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
							((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
							((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
							((0) >> 16) & 0xff, ((0) >> 24) & 0xff);

			for (i = 0; i < w * h; i++) {
				unsigned char R, G, B;

				R = img->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
				G = img->comps[1].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
				B = img->comps[2].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
				fprintf(f, "%c%c%c", B, G, R);
				if (((i + 1) % w == 0 && w % 2))
					fprintf(f, "%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
									((0) >> 16) & 0xff);
			}
			fclose(f);
		} else {										/* Gray-scale */

			/* -->> -->> -->> -->>

			   8 bits non code (Gray scale)

			   <<-- <<-- <<-- <<-- */
			f = fopen(argv[2], "wb");
			w = ceildiv(img->x1 - img->x0, img->comps[0].dx);
			h = ceildiv(img->y1 - img->y0, img->comps[0].dy);

			fprintf(f, "BM");

			/* FILE HEADER */
			/* ------------- */
			fprintf(f, "%c%c%c%c",
							(unsigned char) (h * w + 54 + 1024 + h * (w % 2)) & 0xff,
							(unsigned char) ((h * w + 54 + 1024 + h * (w % 2)) >> 8) &
							0xff,
							(unsigned char) ((h * w + 54 + 1024 + h * (w % 2)) >> 16) &
							0xff,
							(unsigned char) ((h * w + 54 + 1024 + w * (w % 2)) >> 24) &
							0xff);
			fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
							((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (54 + 1024) & 0xff, ((54 + 1024) >> 8) & 0xff,
							((54 + 1024) >> 16) & 0xff, ((54 + 1024) >> 24) & 0xff);
			/* INFO HEADER */
			/* ------------- */
			fprintf(f, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,
							((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (unsigned char) ((w) & 0xff),
							(unsigned char) ((w) >> 8) & 0xff,
							(unsigned char) ((w) >> 16) & 0xff,
							(unsigned char) ((w) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (unsigned char) ((h) & 0xff),
							(unsigned char) ((h) >> 8) & 0xff,
							(unsigned char) ((h) >> 16) & 0xff,
							(unsigned char) ((h) >> 24) & 0xff);
			fprintf(f, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
			fprintf(f, "%c%c", (8) & 0xff, ((8) >> 8) & 0xff);
			fprintf(f, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff,
							((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (unsigned char) (h * w + h * (w % 2)) & 0xff,
							(unsigned char) ((h * w + h * (w % 2)) >> 8) & 0xff,
							(unsigned char) ((h * w + h * (w % 2)) >> 16) & 0xff,
							(unsigned char) ((h * w + h * (w % 2)) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
							((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,
							((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff,
							((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
			fprintf(f, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff,
							((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
		}

		for (i = 0; i < 256; i++) {
			fprintf(f, "%c%c%c%c", i, i, i, 0);
		}

		for (i = 0; i < w * h; i++) {
			fprintf(f, "%c",
							img->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)]);
			if (((i + 1) % w == 0 && w % 2))
				fprintf(f, "%c", 0);

		}
	}
	return 0;
}

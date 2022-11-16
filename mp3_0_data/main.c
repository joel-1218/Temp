#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// window 768 floats, buf 1024 floats, out 32 floats, incr 1
void ff_mpadsp_apply_window_float(float *synth_buf, float *window,
                                  int *dither_state, float *samples,
                                  ptrdiff_t incr);

static void readfile(const char name[], void *buf, const unsigned len) {
	FILE *f = fopen(name, "r");
	if (!f)
		exit(1);

	if (fread(buf, len, 1, f) != 1)
		exit(1);
	fclose(f);
}

int main() {

	float win[768], buf[1024], out[32], outexpected[32];
	int unused = 0;

	readfile("win0", win, 768 * 4);

	int i;
	for (i = 0; i < 10; i++) {
		char name[32];

		sprintf(name, "samples%u", i * 1000);
		readfile(name, outexpected, 32 * 4);

		sprintf(name, "buf%u", i * 1000);
		readfile(name, buf, 1024 * 4);

		ff_mpadsp_apply_window_float(buf, win, &unused, out, 1);

		if (memcmp(out, outexpected, 32 * 4)) {
			printf("Failed at %u\n", i);
			return 1;
		}
	}

	puts("ok");
	return 0;
}

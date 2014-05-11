#include <stdio.h>
#include <string.h>
#include "utils.h"
#include <stdlib.h>

struct int2binf_params {
	char *WANT;
	char *format;
	uint64_t value;
	int size;
};

int
test_call(struct int2binf_params *p, char **out)
{
	int r;
	*out = int2binf(p->format, p->value, p->size);
	r = strncmp(p->WANT, *out, strlen(p->WANT)) != 0;
	return r;
}

int
main()
{
	struct int2binf_params TEST[] = {
		{ "110", "...", 6, 3 },
		{ "111", "...", 7, 3 },
		{ "A111", "A...", 7, 3 },
		{ "1010 111", ".... ...", 0x57, 7 },
		{ "a01b11c01d", "a..b..c..d", 0x1d, 6 },
		{ NULL, NULL, 0, 0 },
        };

	struct int2binf_params *p;
	int rr, r;

	for (rr = 0, p = TEST; p->WANT; p++) {
		char *output;
		rr += r = test_call(p, &output);
		printf("\"%s\" == got \"%s\" for \"%s\", 0x%016lx, %d: %s\n",
			p->WANT, output, p->format, p->value, p->size,
			(r ? "failed" : "ok"));
		free(output);
	}
	return rr;
}

#include <stdio.h>
#include <string.h>
#include "utils.h"
#include <stdlib.h>

struct testp {
	int  flag;
	char *want;
	struct int2binf_params {
		char *format;
		uint64_t value;
		int size;
	} int2binf_;
};

static int
test_call(struct testp *p, char **out)
{
	int r;
	*out = int2binf(p->int2binf_.format, p->int2binf_.value, p->int2binf_.size);
	if (p->want == NULL && *out == NULL)
		return 0;
	if (p->want == NULL || *out == NULL)
		return 1;
	r = strncmp(p->want, *out, strlen(p->want)) != 0;
	return r;
}

int
main()
{
	struct testp TEST[] = {
		{ 0, "110",  { "...", 6, 3},  },
		{ 0, "111",  { "...", 7, 3},  },
		{ 0, "A111", { "A...", 7, 3}, },
		{ 0, "1010 111", {".... ...", 0x57, 7 }, },
		{ 0, "a01b11c01d", {"a..b..c..d", 0x1d, 6 }, },
		{ 0, "a10b1?c??d", {"a..b..c..d", 0x1d, 3 }, },
		{ 0, "a??b??c??d", {"a..b..c..d", 0x1d, 0 }, },
		{ 0, "a??b??c??d", {"a..b..c..d", 0x1d, -2 }, },
		{ 0, "0000 0010 0100 0110 1000 1010 1100 1111 0101 0111 1001 1011 1101 1110 0000 001?",
			{".... .... .... .... .... .... .... .... .... .... .... .... .... .... .... .... ",
				0x01234567abcdef01, 63 }, },
		{ 0, "0000 0001 0010 0011 0100 0101 0110 0111 1010 1011 1100 1101 1110 1111 0000 0001",
			{".... .... .... .... .... .... .... .... .... .... .... .... .... .... .... .... ",
				0x01234567abcdef01, 64 }, },
		{ 0, "0000 0001 0010 0011 0100 0101 0110 0111 1010 1011 1100 1101 1110 1111 0000 0001",
			{".... .... .... .... .... .... .... .... .... .... .... .... .... .... .... .... ",
				0x01234567abcdef01, 65 }, },
		{ 0, "", {"", 0x1d, 6 }, },
		{ 0, NULL, {NULL, 0x1d, 6 }, },
		{ -1, NULL,  { NULL, 0, 0},  },
        };

	struct testp *p;
	int rr, r;

	for (rr = 0, p = TEST; !(p->flag); p++) {
		char *output;
		rr += r = test_call(p, &output);
		printf("\"%s\" == got \"%s\" for \"%s\", 0x%016lx, %d: %s\n",
			p->want, output, 
				p->int2binf_.format, 
				p->int2binf_.value,
				p->int2binf_.size,
			(r ? "failed" : "ok"));
		free(output);
	}
	return rr;
}

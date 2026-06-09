#ifndef MEMSEARCH_H
#define MEMSEARCH_H

#include <QString>
#include <QVector>

class EmuModel;

// -----------------------------------------------------------------------
// The memory-search engine behind the memory view's Ctrl-F strip. Pure model
// logic over EmuModel: no widgets, no view state. MemView owns the strip and
// the search cursor and drives this; find() is a stateless query (memory +
// parameters in, a match location out), so it is straightforward to test in
// isolation.
class MemSearch {

public:
	// Search modes, in the strip's dropdown order (index == enum value, so the
	// combobox index casts straight to a Mode). NUM is one numeric mode: bare
	// token = decimal, 0x prefix = hex, each masked to 16 bits.
	enum Mode { NUM, ASCII, R40 };

	// A located match: segment nb, the word run [found .. last] (inclusive), and
	// whether the scan had to wrap its range to reach it (drives the "wrapped"
	// cue).
	struct Result {
		int nb;
		int found;
		int last;
		bool wrapped;
	};

	explicit MemSearch(EmuModel *emu) : e(emu) {}

	// Validate query against mode; an empty/blank query is a valid partial state
	// (returns true). num accepts space-separated decimal/0x-hex tokens, ascii
	// matches any text, r40 accepts only radix-40 alphabet characters.
	static bool query_valid(const QString &query, Mode mode);

	// Search from the cursor in the given direction. The cursor is the start word
	// `origin` of the last hit in segment `cur_nb`; forward resumes one word past
	// it, backward one word before. origin < 0 means "from the top" (forward) /
	// "from the bottom" (backward). With all_segments the whole (segment, address)
	// space is scanned linearly from cur_nb, wrapping once around; otherwise only
	// cur_nb is scanned. Returns true and fills `out` on a hit; returns false
	// (out untouched) on no match or an invalid/empty query.
	bool find(const QString &query, Mode mode, bool all_segments,
	          int cur_nb, int origin, bool forward, Result &out) const;

private:
	EmuModel *e;

	// True if the word sequence matches memory at addr in segment nb. An
	// unreadable word (no memory mapped, reads back -1) never matches.
	bool num_match_at(int nb, int addr, const QVector<uint16_t> &words) const;
	// Numeric windowed match locator: scan segment nb over the closed start-address
	// window [lo..hi] for the first word-sequence match (ascending if forward,
	// descending otherwise). No wrap - find() stitches windows together. hi is
	// clamped so a multi-word match still fits the segment.
	bool num_scan(int nb, int lo, int hi, bool forward,
	              const QVector<uint16_t> &words, int &found, int &last) const;
	// ascii / r40 windowed stream match locator: substring-search the prebuilt
	// segment stream for a match whose START word lies in the closed window
	// [lo..hi] (no wrap). A match starting mid-word still frames the whole word it
	// begins in. find() builds and caches the stream so each segment is decoded at
	// most once per search.
	bool stream_scan(const QString &stream, int lo, int hi, bool forward,
	                 const QString &query, Mode mode, int &found, int &last) const;
	// Decode segment nb into one flat char stream (2 chars per word for ascii, 3
	// for r40) so a query can substring-match across word boundaries; word `a`'s
	// chars occupy stream[a*cpw .. a*cpw+cpw-1]. Unmapped words become NUL
	// sentinels: a fixed-width, non-matchable gap (the query can't hold a NUL), so
	// search never hits unreadable memory - same as the numeric path.
	QString build_search_stream(int nb, Mode mode) const;
};

#endif // MEMSEARCH_H

// vim: tabstop=4 shiftwidth=4 autoindent

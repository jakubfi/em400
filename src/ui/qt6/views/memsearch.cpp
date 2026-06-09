#include <cctype>
#include <QHash>
#include <QStringList>
#include <QRegularExpression>
#include <emcrk/r40.h>
#include "memsearch.h"
#include "emumodel.h"

// -----------------------------------------------------------------------
// Parse one numeric search token into a 16-bit word: a bare token is decimal
// (-32768..65535, masked), a 0x prefix makes it hex. Returns false for an
// invalid token; word may be null to validate without producing the value.
static bool num_token_to_word(const QString &t, uint16_t *word)
{
	if (t.startsWith("0x", Qt::CaseInsensitive)) {
		QString h = t.mid(2);
		if (h.isEmpty()) return false;
		for (QChar c : h) {
			if (!isxdigit((unsigned char)c.toLatin1())) return false;
		}
		// leading zeros are fine (0x000f), but no more than 4 significant
		// digits so the value fits a 16-bit word (max 0xffff)
		int i = 0;
		while (i < h.length() - 1 && h[i] == '0') i++;
		if (h.length() - i > 4) return false;
		if (word) *word = (uint16_t)h.toUInt(nullptr, 16);
		return true;
	}
	// decimal token (optional sign), masked to 16 bits
	bool ok = false;
	long v = t.toLong(&ok, 10);
	if (!ok || v < -32768 || v > 65535) return false;
	if (word) *word = (uint16_t)v;
	return true;
}

// -----------------------------------------------------------------------
bool MemSearch::query_valid(const QString &s, Mode mode)
{
	if (s.trimmed().isEmpty()) return true;

	switch (mode) {
		case NUM: {
			const QStringList toks = s.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
			for (const QString &t : toks) {
				if (!num_token_to_word(t, nullptr)) return false;
			}
			return true;
		}
		case ASCII:
			return true;
		case R40: {
			for (QChar c : s) {
				// toLatin1() folds anything outside Latin-1 to 0, and code 0 is
				// a valid r40 char (space) - so reject non-Latin1 explicitly,
				// otherwise e.g. a Polish letter would slip through as a space
				if (c.unicode() > 0xFF) return false;
				if (!r40_valid_char(c.toUpper().toLatin1())) return false;
			}
			return true;
		}
	}
	return true;
}

// -----------------------------------------------------------------------
bool MemSearch::num_match_at(int nb, int addr, const QVector<uint16_t> &words) const
{
	for (int i = 0 ; i < words.size() ; i++) {
		int v = e->get_mem(nb, addr + i);
		if (v < 0 || (uint16_t)v != words[i]) return false;
	}
	return true;
}

// -----------------------------------------------------------------------
bool MemSearch::num_scan(int nb, int lo, int hi, bool forward,
                         const QVector<uint16_t> &words, int &found, int &last) const
{
	const int n = words.size();
	if (n == 0) return false;
	// last address a multi-word match may start at and still fit the segment
	const int maxstart = 0xffff - (n - 1);
	if (lo < 0) lo = 0;
	if (hi > maxstart) hi = maxstart;
	if (lo > hi) return false;

	if (forward) {
		for (int a = lo ; a <= hi ; a++) {
			if (num_match_at(nb, a, words)) { found = a; last = a + n - 1; return true; }
		}
	} else {
		for (int a = hi ; a >= lo ; a--) {
			if (num_match_at(nb, a, words)) { found = a; last = a + n - 1; return true; }
		}
	}
	return false;
}

// -----------------------------------------------------------------------
QString MemSearch::build_search_stream(int nb, Mode mode) const
{
	const int cpw = (mode == R40) ? 3 : 2;
	QString s;
	s.reserve(0x10000 * cpw);

	// Read a page at a time (16 pages of 4096 words) instead of 64K single-word
	// calls. The block read's own bool is authoritative for "is this page mapped
	// right now" - we never gate on a separate get_mem_map() snapshot (that plus
	// a read is a TOCTOU race with the CPU thread's mem_update_map). On false we
	// never touch the buffer, so both race directions stay benign: a page that
	// went unmapped reads as a sentinel gap, one that just got mapped is a
	// stale-view miss (same accepted class as memory changing mid-search).
	uint16_t buf[0x1000];
	for (int p = 0 ; p < 16 ; p++) {
		if (!e->get_mem(nb, p << 12, buf, 0x1000)) {
			s += QString(0x1000 * cpw, QChar(0));
			continue;
		}
		if (mode == ASCII) {
			for (int i = 0 ; i < 0x1000 ; i++) {
				unsigned char hi = (buf[i] >> 8) & 0xff;
				unsigned char lo = buf[i] & 0xff;
				s += QChar(isprint(hi) ? hi : '.');
				s += QChar(isprint(lo) ? lo : '.');
			}
		} else {
			for (int i = 0 ; i < 0x1000 ; i++) {
				char dec[4];
				if (!r40_to_ascii(&buf[i], 1, dec)) {
					s += QString(3, QChar(0));
					continue;
				}
				// code 0 decodes to NUL (the blank slot); render it as a space so
				// it stays a real, matchable char and keeps the fixed cell width
				for (int j = 0 ; j < 3 ; j++) {
					if (dec[j] == '\0') dec[j] = ' ';
				}
				s += QString::fromLatin1(dec, 3);
			}
		}
	}
	return s;
}

// -----------------------------------------------------------------------
bool MemSearch::stream_scan(const QString &stream, int lo, int hi, bool forward,
                            const QString &query, Mode mode, int &found, int &last) const
{
	if (query.isEmpty()) return false;
	if (lo < 0) lo = 0;
	if (hi > 0xffff) hi = 0xffff;
	if (lo > hi) return false;

	const int cpw = (mode == R40) ? 3 : 2;
	const int qlen = query.length();

	int off = -1;
	if (forward) {
		// leftmost match starting at or after lo; reject if it starts past hi
		off = stream.indexOf(query, lo * cpw);
		if (off >= 0 && off / cpw > hi) off = -1;
	} else {
		// rightmost match whose start is at or before hi (the +cpw-1 lets a
		// match start anywhere within that word); reject if it starts before lo
		off = stream.lastIndexOf(query, hi * cpw + cpw - 1);
		if (off >= 0 && off / cpw < lo) off = -1;
	}

	if (off < 0) return false;
	found = off / cpw;
	last = (off + qlen - 1) / cpw;
	return true;
}

// -----------------------------------------------------------------------
bool MemSearch::find(const QString &q, Mode mode, bool all,
                     int cnb, int origin, bool forward, Result &out) const
{
	if (!e) return false;

	// parse the query once: a word sequence for num, a folded string for streams
	QVector<uint16_t> words;
	QString query;
	if (mode == NUM) {
		const QStringList toks = q.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
		for (const QString &t : toks) {
			uint16_t w;
			if (!num_token_to_word(t, &w)) return false;
			words.append(w);
		}
		if (words.isEmpty()) return false;
	} else {
		// r40 has no lowercase and decodes to uppercase, so fold the query to match
		query = (mode == R40) ? q.toUpper() : q;
		if (query.isEmpty()) return false;
	}

	// Build the ordered probe list. Each probe is a single-segment, non-wrapping
	// address window. Forward resumes one word past the cursor and runs up to the
	// top of the space; the "wrapped" probes are those reached after rolling off
	// the end back to the cursor. Single-segment mode is just first/last = cnb,
	// which reduces to the previous two-window (forward, then wrapped) behaviour.
	const int first_seg = all ? 0 : cnb;
	const int last_seg  = all ? 15 : cnb;
	struct Probe { int nb, lo, hi; bool wrapped; };
	QVector<Probe> probes;
	if (forward) {
		for (int s = cnb ; s <= last_seg ; s++) {
			int lo = (s == cnb) ? origin + 1 : 0;
			probes.append({s, lo, 0xffff, false});
		}
		for (int s = first_seg ; s <= cnb ; s++) {
			int hi = (s == cnb) ? origin : 0xffff;
			probes.append({s, 0, hi, true});
		}
	} else {
		// -1 (fresh search) means "from the bottom": start one past the top
		const int cur = (origin < 0) ? 0x10000 : origin;
		for (int s = cnb ; s >= first_seg ; s--) {
			int hi = (s == cnb) ? cur - 1 : 0xffff;
			probes.append({s, 0, hi, false});
		}
		for (int s = last_seg ; s >= cnb ; s--) {
			int lo = (s == cnb) ? cur : 0;
			probes.append({s, lo, 0xffff, true});
		}
	}

	// streams are immutable for the duration of one search, so decode each
	// touched segment at most once (a segment can appear in both the forward and
	// the wrapped probe list)
	QHash<int, QString> stream_cache;

	int found = -1, last = -1, hit_nb = cnb;
	bool wrapped = false;
	for (const Probe &p : probes) {
		bool ok;
		if (mode == NUM) {
			ok = num_scan(p.nb, p.lo, p.hi, forward, words, found, last);
		} else {
			auto it = stream_cache.find(p.nb);
			if (it == stream_cache.end()) {
				it = stream_cache.insert(p.nb, build_search_stream(p.nb, mode));
			}
			ok = stream_scan(it.value(), p.lo, p.hi, forward, query, mode, found, last);
		}
		if (ok) { hit_nb = p.nb; wrapped = p.wrapped; break; }
	}

	if (found < 0) return false;

	out = { hit_nb, found, last, wrapped };
	return true;
}

// vim: tabstop=4 shiftwidth=4 autoindent

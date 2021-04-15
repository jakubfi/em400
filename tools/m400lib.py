import struct

# ------------------------------------------------------------------------
def r2a(i):
    if i >= 1 and i <= 26:
        return chr(i + 64)
    elif i >= 27 and i <= 36:
        return chr(i + 21)
    elif i == 37:
        return "_"
    elif i == 38:
        return "%"
    elif i == 39:
        return "#"
    elif i == 0:
        return " "
    else:
        return ""

# ------------------------------------------------------------------------
def r40_triplet(w):
    triplet = [
        r2a((w // 1600) % 40),
        r2a((w // 40) % 40),
        r2a(w % 40),
    ]
    return ''.join(triplet)

# ------------------------------------------------------------------------
def r40_str(words):
    chars = [r40_triplet(w) for w in words]
    return ''.join(chars)

# ------------------------------------------------------------------------
def wload(ifile, offset, ilen):
    f = open(ifile, "rb")
    f.seek(offset)
    odata = []
    pos = 0
    while pos < ilen:
        data = struct.unpack('!H', f.read(2))[0]
        odata.append(data)
        pos += 1;
    f.close();
    return odata

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

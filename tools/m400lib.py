#!/usr/bin/python

# ------------------------------------------------------------------------
def r2a(i):
	if (i>=1) and (i<=26):
		return chr(i+64)
	elif (i>=27) and (i<=36):
		return chr(i+21)
	elif i==37:
		return "_"
	elif i==38:
		return "%"
	elif i==39:
		return "#"
	elif i==0:
		return " "
	else:
		return ""

# ------------------------------------------------------------------------
def r40(w):
	kz1 = r2a((w/1600) % 40)
	kz2 = r2a((w/40) % 40)
	kz3 = r2a(w % 40)
	return "%s%s%s" % (kz1, kz2, kz3)

# ------------------------------------------------------------------------
def wload(ifile, sector, ilen):
	f = open(ifile, "r")
	f.seek(sector * 512)
	odata = []
	pos = 0
	while pos<ilen:
		odata.append(ord(f.read(1))*256 + ord(f.read(1)))
		pos += 1;
	f.close();
	return odata

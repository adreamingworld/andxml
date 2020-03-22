AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4 --install

# Files we want to be packaged in the tar ball distribution
EXTRA_DIST = 	README.md 

# Make one binary called <hello>
bin_PROGRAMS = xml

# Sources used to create the <hello> binary
xml_SOURCES = \
	src/xml.h \
	src/xml.c \
	src/main.c

xml_CFLAGS = -DDATA_PATH='"$(pkgdatadir)"'
xml_LDADD = 

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4 --install

# Files we want to be packaged in the tar ball distribution
EXTRA_DIST = 	README.md 

lib_LIBRARIES = libandxml.a
libandxml_a_SOURCES = \
	src/xml.h \
	src/xml.c 

if BUILD_TEST
# Sources used to create the <hello> binary
bin_PROGRAMS = xml_test
xml_test_SOURCES = \
	src/main.c

xml_test_CFLAGS = -DDATA_PATH='"$(pkgdatadir)"'
xml_test_LDADD = libandxml.a
endif

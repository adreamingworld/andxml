#include <xml.h>

#include <config.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	char* file_name = 0;
	struct XML xml = {0};

	if (argc < 2) return -1;
	file_name = argv[1];

	puts(PACKAGE_NAME);

	if (xml_load(file_name, &xml) != XML_ERROR_OK) return 123;
//	puts(xml.version);
//	puts(xml.encoding);

	return 0;
}


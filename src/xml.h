#ifndef XML_H
#define XML_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	XML_ERROR_OK = 0,
	XML_ERROR,
	XML_ERROR_EOF
	};

struct XML_Attribute {
	char* name;
	char* value;
	};

struct XML_QueryResult {
	int count;
	int *indices;
	};

struct XML_Element {
	char* name;
	int parent;
	int child;
	int sibling;
	struct XML_Attribute* attributes;
	int attribute_count;
	char* content;
	};

struct XML {
	struct XML_Element root;
	struct XML_Element *elements;
	int element_count;
	char* version;
	char* encoding;
	};

int xml_load(char* filename, struct XML* xml);

#endif /* XML_H */

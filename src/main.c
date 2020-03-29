#include <xml.h>

#include <config.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	char* file_name = 0;
	struct XML xml = {0};
	struct XML_Element *node = 0;
	int i;

	if (argc < 2) return -1;
	file_name = argv[1];

	puts(PACKAGE_NAME);

	if (xml_load(file_name, &xml) != XML_ERROR_OK) {
		puts("Something went terribly wrong");
		return 123;
		}
//	puts(xml.version);
//	puts(xml.encoding);
	struct XML_Element *current_node = &xml.elements[0];
	while (1) {
		char buffer[256];
		char command[256];
		printf("<%s>: ", current_node->name);
		fgets(buffer, 256, stdin);
		sscanf(buffer, "%s", command);
		if (!strcmp("exit", command)) break;
		if (!strcmp("ls", command)) {
			struct XML_QueryResult qr;
			int i;
			xml_ls(&xml, current_node, &qr);
			for (i=0; i<qr.count; i++) {
				printf("%s\n", xml.elements[qr.indices[i]].name);
				}
			xml_destroy_query_result(&qr);
			}
		if (!strcmp("content", command)) {
			struct XML_QueryResult qr;
			char name[256];

			sscanf(buffer, "%*s %s", name);
			xml_find_child_by_name(&xml, current_node, name, &qr);
			if (qr.count > 0) {
				struct XML_Element *node = &xml.elements[qr.indices[0]];
				if (node->content)
					puts(node->content);
				}
			xml_destroy_query_result(&qr);
			}
		if (!strcmp("attr", command)) {
			struct XML_QueryResult qr;
			char name[256];

			sscanf(buffer, "%*s %s", name);
			xml_find_child_by_name(&xml, current_node, name, &qr);
			if (qr.count > 0) {
				struct XML_Element *node = &xml.elements[qr.indices[0]];
				for (i=0; i<node->attribute_count; i++) {
					printf("%s = %s\n", 
						node->attributes[i].name,
						node->attributes[i].value
						);
					}
				}
			xml_destroy_query_result(&qr);
			}
		if (!strcmp("cd", command)) {
			char name[256];
			int index = 0;
			int argc;
			argc = sscanf(buffer, "%*s %s %i", name, &index);
			printf("got %i args\n", argc);
			if (argc < 0) continue;
			if (argc == 0) continue;
			struct XML_QueryResult qr;
			xml_find_child_by_name(&xml, current_node, name, &qr);
			if (qr.count) current_node = &xml.elements[qr.indices[index]];
			else printf("Failed to find <%s>\n", name);
			xml_destroy_query_result(&qr);
			}
		}

	return 0;
}


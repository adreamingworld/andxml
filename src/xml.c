#include <andxml.h>
#include <unistd.h>

int
xml_skip_whitespace(FILE* fp)
	{
	while ( isspace(fgetc(fp)) );
	/* Put back before non-whitespace character */
	fseek(fp, -1, SEEK_CUR);
	if(feof(fp)) return XML_ERROR_EOF;

	return XML_ERROR_OK;
	}

int
xml_equals_token(char c, char* tokens)
	{
	int i;
	for (i=0; i<strlen(tokens); i++) if (tokens[i] == c) return 1;

	return 0;
	}


int
xml_get_string_till_tokens(FILE* fp, char** string, char* tokens, int include_token)
	{
	int length = 0;
	xml_skip_whitespace(fp);

	while (1) {
		char c;
		c = fgetc(fp);
		if (feof(fp)) return XML_ERROR;
		if (xml_equals_token(c, tokens)) {length++; break;}
		length++;
		}
	fseek(fp, -length, SEEK_CUR);
	if (!include_token) length--;
	*string = calloc(1, length+1);
	fread(*string, 1, length, fp);

	return XML_ERROR_OK;
	}

int
xml_get_name(FILE* fp, char** name)
	{
	*name = 0;
	int length;

	xml_skip_whitespace(fp);

	/* Count name length */
	for (length=0; length<256; length++) {
		char c = fgetc(fp);
		if (length > 0 && (
			isspace(c) ||
			c == '=' ||
			c == '/' ||
			c == '>' )
			) break;
		}

	fseek(fp, -length-1, SEEK_CUR);
	*name = calloc(1, length+1);
	if (fread(*name, 1, length, fp) != length) return XML_ERROR;

	return XML_ERROR_OK;
	}
void
xml_error(FILE* fp, char* message)
	{
	int i;
	int line_count = 1;
	int char_count = 1;
	char sample[256];
	/* Find line number */
	int file_position = 0;
	int original_position = ftell(fp);
	fread(sample, 1, 255, fp);
	printf("%s\n", message);
	//printf("Sample: @%s@\n", sample);
	rewind(fp);

	while (!feof(fp)) {
		char c = fgetc(fp);
		file_position = ftell(fp);
		if (file_position == original_position) break;
		char_count++;
		if (c=='\n') {
			line_count++;
			char_count=1;}
		}
	printf("%s: Line: %i, character: %i\n", message, line_count, char_count);

	}

int
xml_get_attributes(FILE* fp, struct XML_Attribute** attributes, int *count)
	{
	*count = 0;

	*attributes = calloc(1, 0);

	while (!feof(fp)) {
		char *name;
		char *value;

		/*Get name */
		xml_skip_whitespace(fp);
		xml_get_string_till_tokens(fp, &name, " =>\"\n\r\t", 0);

		/* End of attributes ? */
		if (
			!strcmp(name, "?") ||
			!strcmp(name, "/") ||
			!strcmp(name, "") 
			) {
			fseek(fp, -strlen(name), SEEK_CUR);
			break;
			}

		if (!strlen(name)) break;
		xml_skip_whitespace(fp);
		if (fgetc(fp) !='=') {
			printf("Name: @%s@\n", name);
			xml_error(fp, "Error in get attribute");
			return XML_ERROR;
			}
		if (fgetc(fp) != '"') {
			printf("Invalid string\n");
			exit(-1);
			}
		/* Get value*/
		xml_get_string_till_tokens(fp, &value, ">\"", 1);
		int length = strlen(value);
		if (value[length-1] == '"') 
			value[length-1] = 0;
		(*count)++;
		*attributes = realloc(*attributes, sizeof(struct XML_Attribute) * (*count));
		(*attributes)[*count-1].name = name;
		(*attributes)[*count-1].value = value;
		}

	return XML_ERROR_OK;
	}
int
xml_get_cdata(FILE* fp, char** cdata)
	{
	char c;
	char string[8] = {0};
	*cdata = 0;
	fread(string, 1, 7, fp);
	if (strcmp("[CDATA[", string)) return XML_ERROR;

//	xml_get_string_till_tokens(fp, cdata, "]");

	/* Get length of data */
	int length = 0;
	int in_string = 0;
	int in_square_brackets = 0;
	while (!feof(fp)) {
		char c = fgetc(fp);
		length++;
		if (c == '\\') {fgetc(fp); length++;}
		else if (c == '"') in_string = 1;
		else if (c == ']' && !in_square_brackets) break;
		else if (c == ']' && in_square_brackets) 
			{in_square_brackets--;}
		else if (c == '[') {in_square_brackets++;}
		};
	fseek(fp, -length, SEEK_CUR);
	*cdata = calloc(1, length+1);
	fread(*cdata, 1, length-1, fp);

	c = fgetc(fp);
	if (c != ']') return XML_ERROR;
	c = fgetc(fp);
	if (c != ']') return XML_ERROR;
	c = fgetc(fp);
	if (c != '>') return XML_ERROR;

	return XML_ERROR_OK;
	}

int
xml_get_element(FILE* fp, struct XML_Element *element, int* self_ending)
	{
	char c;
	char *name = 0;
	int attribute_count = 0;
	struct XML_Attribute* attributes;
	char* end = 0;
	
	/* Assume not self ending, <tag />*/
	*self_ending = 0;

	xml_skip_whitespace(fp);
	c = fgetc(fp);
	if (c != '<') return XML_ERROR;
	c = fgetc(fp);
	/** @TODO Get <!-- --> comments in a more
		reliable way!
	*/
	if (c == '!') { /* CDATA ? */
		c = fgetc(fp);
		if (c == '-') {
			while (fgetc(fp) != '>');
			while (fgetc(fp) != '<');
			}
		else if (c == '[') {
			fseek(fp, -1, SEEK_CUR);
			char *cdata = 0;
			if (xml_get_cdata(fp, &cdata) != XML_ERROR_OK) {
				xml_error(fp, "Error in get cdata");
				printf("@%s@\n", cdata);
				return XML_ERROR;
				}
			element->name = malloc(6);
			strcpy(element->name, "CDATA");
			element->content = cdata;
			*self_ending = 1;
			return XML_ERROR_OK;
			}
		}
	else fseek(fp, -1, SEEK_CUR);

	/* get name */
	xml_get_name(fp, &name);
	if (!name) return XML_ERROR;

	/* get attributes */
	if (xml_get_attributes(fp, &attributes, &attribute_count)!= XML_ERROR_OK)
		return XML_ERROR;

	/* How does it end */
	xml_get_string_till_tokens(fp, &end, ">", 1);
	if (
		!strcmp("?>", end)||
		!strcmp("/>", end)
		) *self_ending = 1;
	free(end);

	element->attributes = attributes;
	element->attribute_count = attribute_count;
	element->name = name;

	return XML_ERROR_OK;
	}
void
xml_init_element(struct XML_Element *element, int parent)
	{
	element->parent = parent;
	element->child = -1;
	element->sibling = -1;
	element->content = 0;
	}

void
xml_init(struct XML *xml)
	{
	xml->elements = malloc(0);
	xml->element_count = 0;
	xml->version = 0;
	xml->encoding = 0;
	}
int
xml_get_content(FILE* fp, char **content)
	{
	int length = 0;
	*content = 0;
	content[0] = 0;
	while (!feof(fp)) {
		char c = fgetc(fp);
		if (c == '<') break;
		length++;
		}
	if (feof(fp)) return XML_ERROR_EOF;
	fseek(fp, -length-1, SEEK_CUR);
	if (length <= 0) return XML_ERROR;
	*content = calloc(1, length+1);
	fread(*content, 1, length, fp);

	return XML_ERROR_OK;
	}

int
xml_print_element(struct XML_Element *element)
	{
	int i;

	if (!element->name) return XML_ERROR;
	printf("Name: %s\n", element->name);
	printf("%i attributes\n", element->attribute_count);
	for (i =0; i < element->attribute_count; i++) {
		struct XML_Attribute* attr = &element->attributes[i];
		printf("%s = %s\n", attr->name, attr->value);
		}

	return XML_ERROR_OK;
	}

#define XML_STACK_SIZE 256
struct XML_ElementStack {
	int pointer;
	int stack[XML_STACK_SIZE];
	};

int
xml_push_stack(struct XML_ElementStack* stack, int element_id)
	{
	if (stack->pointer == XML_STACK_SIZE-1) return XML_ERROR;
	stack->stack[stack->pointer++] = element_id;
	return XML_ERROR_OK;
	}
int
xml_pop_stack(struct XML_ElementStack* stack, int *element_id)
	{
	if (stack->pointer <= 0) return XML_ERROR;
	*element_id = stack->stack[stack->pointer--];
	return XML_ERROR_OK;
	}

void
xml_init_stack(struct XML_ElementStack* stack)
	{
	int i;
	stack->pointer = 0;

	for (i=0; i<XML_STACK_SIZE; i++)
		stack->stack[i] = -1;
	}
void
xml_add_node_as_sibling(struct XML *xml, int node, int sibling)
	{
	struct XML_Element *sibling_node = &xml->elements[sibling];
	if (sibling > xml->element_count) {printf("Element overflow! %i\n", sibling); exit(-1);}
	/* If has sibling then add as its sibling's sibling */
	if (sibling_node->sibling == -1) {
		sibling_node->sibling = node;
		}
	else /* Add as sibling */
		xml_add_node_as_sibling(xml, node, sibling_node->sibling);
	}
void
xml_add_node_as_child(struct XML *xml, int node, int parent)
	{
	struct XML_Element *parent_node = &xml->elements[parent];
	if (parent > xml->element_count) {printf("Element overflow! %i\n", parent); exit(-1);}
	/* If has child then add as sibling */
	if (parent_node->child == -1) {
		parent_node->child = node;
		}
	else /* Add as sibling */
		xml_add_node_as_sibling(xml, node, parent_node->child);
	}
void
xml_print_tree(struct XML *xml, int node_index, int level)
	{
	struct XML_Element *node = &xml->elements[node_index];
	int i;
	for (i=0; i<level; i++) printf(" ");
	printf("%s\n", node->name);
	// print children
	if (node->child > -1) xml_print_tree(xml, node->child, level+1);
	if (node->sibling > -1) xml_print_tree(xml, node->sibling, level);

	}

int
xml_ls(struct XML *xml, struct XML_Element *node, struct XML_QueryResult *qr)
	{
	int i=0;
	int child_index = node->child;

	qr->count = 0;

	while (child_index > -1) {
		node = &xml->elements[child_index];
		printf("%s\n", node->name);
		child_index = node->sibling;
		}
	return 0;
	}

int
xml_find_child_by_name(struct XML* xml, struct XML_Element *parent_node, char* string, struct XML_QueryResult *qr)
	{
	int i=0;
	int child_index = parent_node->child;
	int current_index = child_index;
	qr->count = 0;

	/* Is it .. to parent */
	if (!strcmp("..", string)) { 
		if (parent_node->parent > -1) {
			qr->count = 1;
			qr->indices = malloc(sizeof(int));
			qr->indices[0] = parent_node->parent;
			printf("Back to parent %i\n", parent_node->parent);
			return 0;
			} else puts("No parent");
		}

	while (current_index > -1) {
		struct XML_Element *node = &xml->elements[current_index];
		//printf("found: %s\n", node->name);
		if (!strcmp(node->name, string)) qr->count++;
		current_index = node->sibling;
		}

	if (qr->count == 0) return 0;

	qr->indices = malloc(sizeof(int) * qr->count);
	current_index = child_index;
	while (current_index > -1) {
		struct XML_Element *node = &xml->elements[current_index];
		//printf("found: %s\n", node->name);
		if (!strcmp(node->name, string)) qr->indices[i++] = current_index;
		current_index = node->sibling;
		}

	return 0;
	}

void
xml_destroy_query_result(struct XML_QueryResult *qr)
	{
	if (qr->count) {
		qr->count = 0;
		free(qr->indices);
		}
	}

char*
xml_get_attribute_by_name(struct XML_Element *node, char* string)
	{
	int i;
	for (i=0; i<node->attribute_count;i++) 
		if (!strcmp(string, node->attributes[i].name))
			return node->attributes[i].value;
	/* Failed to find atribute of that name */
	return 0;
	}

int
xml_load(char* file_name, struct XML* xml)
	{
	FILE* fp;
	struct XML_Element current_element;
	int self_ending = 0;
	int i;
	struct XML_ElementStack stack;

	xml_init_stack(&stack);

	printf("Opening %s\n", file_name);
	fp = fopen(file_name, "r");

	if (!fp) {
		puts("Failed to load the file");
		return XML_ERROR;
		}

	xml_init(xml);
	xml_init_element(&xml->root, 0);

	/* Get xml declaration
	<?xml version="1.0" encoding="utf-8"?>
	*/
	if (xml_get_element(fp, &current_element, &self_ending) != XML_ERROR_OK) {
		puts("Failed to get root element");
		return XML_ERROR;
		}
	xml_print_element(&current_element);
	if (self_ending) puts("SELF ENDING");
	puts("Getting elements");

	/* Get tags */
	int level = 0;
	while (!feof(fp)) {
		char * content = 0;
		xml_init_element(&current_element, -1);
		if (xml_get_element(fp, &current_element, &self_ending) != XML_ERROR_OK) return XML_ERROR;
		/* Get content */
		xml_skip_whitespace(fp);
		if (xml_get_content(fp, &content) != XML_ERROR_OK) {
			/* Has content not already been set, i.e. by CDATA*/
			if (current_element.content == 0) current_element.content = 0;
			}
		else {
			//printf("Content: %s\n", content);
			current_element.content = content;
			}


		if (current_element.name[0] != '/') {
			if (level) {
				current_element.parent = stack.stack[level-1];
				xml_add_node_as_child(xml, xml->element_count, stack.stack[level-1]);
				}
			else current_element.parent = -1;
			stack.stack[level] = xml->element_count;
			xml->elements = realloc(xml->elements,
				sizeof(struct XML_Element) * (xml->element_count+1));
			memcpy(&xml->elements[xml->element_count], &current_element, sizeof(struct XML_Element) );
			xml->element_count++;
			if (!self_ending) level++;
			}


//		for (i=0; i<level; i++) printf(" ");
//		printf("%s ", current_element.name);

		if (
			current_element.name[0] == '/' 
			) {
			level--;
			int parent_index = stack.stack[level];
			struct XML_Element * parent = &xml->elements[parent_index];
			if (strcmp(parent->name, current_element.name+1)) {
				printf("Mismatch at level %i '%s' '%s'\n", parent_index, parent->name, current_element.name);
				printf("level %i\n", level);
				printf("total elements %i\n", xml->element_count);
				}
			}


		if (level >= XML_STACK_SIZE) {
			puts("Stack overflow");
			exit(-1);
			return XML_ERROR;
			}

		}
	printf("Level: %i\n", level);

puts("END");
	return 0;
	}



#include <string.h>
#include "string_extra.h"
#include "config.h"
#include "linkedlist.h"
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 255

linkedlist *options;

bool *heap_true, *heap_false;

typedef struct key_value_pair_
{
	char *key;

	//valid types are 's' for char, 'i' for integer, b for boolean
	char type;

	union
	{
		char *asChrPtr;
		int *asInt;
		bool *asBool;
	} value;

} key_value_pair;

key_value_pair *find_by_key(char *key, char type);

bool config_load(char *filename)
{

	char *buffer; //each line of file is read into here.
	int delim_pos; //position of the deliminitor.
	int key_len, val_len; //length of key and value
	char *in_place_value; //points to the values start position within buffer
	char *key, *val; //holds the key and the value (for strings)
	int *i_val; //holds the value if it is an Integer
	FILE *file; //file pointer.

	//initialise linkedlist
	options = linkedlist_new();

	//we use pointer booleans.
	heap_true = malloc(sizeof(bool));
	*heap_true = true;
	heap_false = malloc(sizeof(bool));
	*heap_false = false;

	file = fopen(filename, "r");
	if (file == NULL)
	{
		printf("Cannot open file for reading: %s", filename);
		return false;
	}

	//allocate room for buffer
	buffer = (char*) malloc (sizeof(char) * MAX_LINE_SIZE);

	while (fgets(buffer, MAX_LINE_SIZE - 1, file) == buffer)
	{
		//first chop off the newline.
		*(buffer + (strlen(buffer) - 1)) = '\0';

		//ignore blank lines
		if (strlen(buffer) == 0)
		{
			printf("Skipping blank line\n");
			continue;
		}

		//ignore comment lines starting with #
		if (*buffer == '#')
		{
			printf("Comment line ignored: %s\n", buffer);
			continue;
		}

		if (strchr(buffer, '=') == NULL)
		{
			printf("No deliminator found: %s\n", buffer);
			continue;
		}

		delim_pos = strchr(buffer, '=') - (buffer - 1);

		key_len = delim_pos;

		printf("delim_pos: %d, key_len: %d, line: %s\n", delim_pos, key_len, buffer);

		//lets copy the key out to it's own memory.
		key = (char*) malloc (sizeof(char) * key_len);

		memcpy(key, buffer, key_len);
		*(key + (key_len - 1)) = '\0';

		printf("Key: %s\n", key);

		//lets evaluate the value part of the string whilst in the buffer
		//and if it is a string copy it out.

		in_place_value = strchr(buffer, '=') + 1;

		if (strncmp(in_place_value, "true", 4) == 0)  //it's a boolean set to true
		{
			printf("boolean value set to true\n");
			config_set_bool(key, heap_true);
		}

		else if (strncmp(in_place_value, "false", 5) == 0) //it's a boolean set to false
		{
			printf("boolean value set to false\n");
			config_set_bool(key, heap_false);
		}

		else if (*in_place_value == '"' && *(in_place_value + (strlen(in_place_value) - 1)) == '"') //it's a string
		{
			//copy the value out.
			val_len = strlen(in_place_value) - 1; // + 1 so we have room for \0 but - 2 because we remove quotes.

			val = (char*) malloc (sizeof(char) * val_len);
			memcpy(val, (in_place_value + 1), val_len);
			*(val + (val_len - 1)) = '\0';

			printf("value: %s\n", val);
			config_set_string(key, val);
		}

		else //assume number
		{
			printf("in_place_value [%x]: %s\n", in_place_value, in_place_value);
			i_val = malloc(sizeof(int));
			*i_val = (int) strtol(in_place_value, &val, 10);

			printf ("val pointer %x\n", val);
			if (val == in_place_value)
				printf("String for key %s is not in quotes. Skipped.\n", key);
			else
			{
				printf("val as int %d\n", *i_val);
				config_set_int(key, i_val);
			}
		}
	}

	free(buffer);

	fclose(file);

}

bool config_get_string(char *key, char **output)
{
	key_value_pair *match = find_by_key(key, 's');

	if (match == NULL)
		return false;
	else
	{
		*output = match->value.asChrPtr;
		return true;
	}
}

bool config_get_int(char *key, int **output)
{
	key_value_pair *match = find_by_key(key, 'i');

	if (match == NULL)
		return false;
	else
	{
		*output = match->value.asInt;
		return true;
	}
}

bool config_get_bool(char *key, bool **output)
{
	key_value_pair *match = find_by_key(key, 'b');

	if (match == NULL)
		return false;
	else
	{
		*output = match->value.asBool;
		return true;
	}
}

void config_set_string(char *key, char *value)
{
	printf("error occurs AFTER this line\n");
	key_value_pair *match = find_by_key(key, 's');

	printf("but before this one.\n");
	if (match == NULL)
	{
		printf("no match found for key %s\n", key);
		match = (key_value_pair*) malloc (sizeof(key_value_pair));
		match->type = 's';
		match->key = key;
		match->value.asChrPtr = value;

		linkedlist_add_last(options, match);

	}
	else
	{
		printf("match for %s key found, changing value");
		match->value.asChrPtr = value;
	}
}

void config_set_int(char *key, int *value)
{

	key_value_pair *match = find_by_key(key, 'i');

	if (match == NULL)
	{
		printf("no match found for key %s\n", key);
		match = (key_value_pair*) malloc (sizeof(key_value_pair));
		match->type = 'i';
		match->key = key;
		match->value.asInt = value;

		linkedlist_add_last(options, match);

	}
	else
	{
		printf("match for %s key found, changing value");
		match->value.asInt = value;
	}
}

void config_set_bool(char *key, bool *value)
{

	key_value_pair *match = find_by_key(key, 'b');

	if (match == NULL)
	{

		match = (key_value_pair*) malloc (sizeof(key_value_pair));
		match->type = 'b';
		match->key = key;
		match->value.asBool = value;

		linkedlist_add_last(options, match);

	}
	else
		match->value.asBool = value;
}

void config_del_value(char *key)
{

	//iterator shoots to the last record
	linkedlist_node *iter = options->head;


	//keep the seond to last record so we can NULL it.
	linkedlist_node *second_to_last = NULL;

	//to be returned to user.
	key_value_pair *item = NULL;

	if (iter == NULL)
		return;

	//only the head item
	if (iter->next == NULL)
	{
		item = (key_value_pair*) options->head->data;

		if (strcmp(item->key,key) == 0)
		{
			free(options->head);
			free(item);
			options->head = NULL;
			return;
		}
	}
	else
		second_to_last = options->head;

	while (iter->next != NULL)
	{
		item = (key_value_pair*) iter->data;

		if (strcmp(item->key,key) == 0)
		{
			second_to_last->next = iter->next; //join
			free(iter);
			free(item);
			return;
		}

		second_to_last = iter;
		iter = iter->next;
	}
}

key_value_pair *find_by_key(char *key, char type)
{
	linkedlist_node *node = options->head;
	key_value_pair *currentItem;

	while (node != NULL)
	{
		currentItem = (key_value_pair*) node->data;

		if (currentItem->type == type && (strcmp(currentItem->key, key) == 0))
			return currentItem;

		node = node->next;
	}

	return NULL;
}


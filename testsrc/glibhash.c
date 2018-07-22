#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmodule.h>

int main(int argc, char *argv[]) {
	GHashTable * hashtable;

	hashtable = g_hash_table_new(g_int_hash, g_str_equal);
}


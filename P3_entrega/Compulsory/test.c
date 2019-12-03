#include <stdlib.h>
#include <stdio.h>
#include "table.h"

int main(){
	char *databaseDir = "./testDatabase.dat";
	int int1, int2;
	table_t *table;
	long pos;
	/*The database sctructure*/
	type_t types[] = {STR, INT, LLNG, STR, DBL};
	

	table_create(databaseDir, 5, types);
	table = table_open(databaseDir);
	if(table == NULL){
		printf("Error while opening the table");
		return -1;
	}

	int1 = 1;
	int2 = 11;
	double e=2.3, f = 4.5;
	long g = 3, h = 4;
	char *c1 = "Short text to test";
	char *c2 = "Text with more lenght";
	char *a1 = "More text 1";
	char *a2 = "More text 2";

	void *datos1[] = {c1, &int1, &g, a1, &e};
	void *datos2[] = {c2, &int2, &h, a2, &f};
	table_insert_record(table, datos1);
	table_insert_record(table, datos2);

	table_close(table);

	/*Re-open the table and read the data*/
	table = table_open(databaseDir);

	pos = table_first_pos(table);

	while( (pos = table_read_record(table, pos)) != -1){
		/*Print the record*/
		print_value(stdout, types[0], table_column_get(table, 0));
		printf("\t\t");
		print_value(stdout, types[1], table_column_get(table, 1));
		printf("\t\t");
		print_value(stdout, types[2], table_column_get(table, 2));
		printf("\t\t");
		print_value(stdout, types[3], table_column_get(table, 3));
		printf("\t\t");
		print_value(stdout, types[4], table_column_get(table, 4));
		printf("\n");
	}

	table_close(table);
	return 0;
}

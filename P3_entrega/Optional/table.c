#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "table.h"
#include "type.h"

struct table_ {
  int nColumns;
  type_t *types;
  FILE *f;
  void **values;
};

/* 
   Creates a file that stores an empty table. This function doesn't
   keep any information in memory: it simply creates the file, stores
   the header information, and closes it.
*/
void table_create(char* path, int ncols, type_t* types) {
	FILE *f = NULL;
	if(path == NULL || types == NULL) return;
	if(ncols < 0) return;
	f = fopen(path, "w");
	if(f == NULL) return;
	fwrite(&ncols, sizeof(int), 1, f);
	fwrite(types, sizeof(type_t), ncols, f);
	fclose(f);
	return;
}

/* 
   Opens a table given its file name. Returns a pointer to a structure
   with all the information necessary to manage the table. Returns
   NULL is the file doesn't exist or if there is any error.
*/
table_t* table_open(char* path) {
  FILE* f;
  table_t* table;
  int ncols, i;
  type_t* types;
  if(path == NULL) return NULL;
  f=fopen(path, "r+");
  if(f == NULL) return NULL;
  fread(&ncols, sizeof(int), 1, f);
  types = (type_t*)malloc(sizeof(int)*ncols);
  if(types == NULL){
    fclose(f);
    return NULL;
  }
  for (i = 0; i<ncols; i++){
  	fread(&types[i], sizeof(type_t), 1, f);
  }

  table = (table_t*)malloc(sizeof(table_t));
  if(table == NULL){
    free(types);
    fclose(f);
    return NULL;
  }

  table->f = f;
  table->types = types;
  table->nColumns = ncols;
  table->values = (void**)malloc(ncols*sizeof(void*));
  if(table->values == NULL){
    free(types);
    free(table);
    fclose(f);
    return NULL;
  }

  for (i = 0; i<ncols; i++){
  	table->values[i] = NULL;
  }

  return table;
}


/* 
   Closes a table freeing the alloc'ed resources and closing the file
   in which the table is stored.
*/
void table_close(table_t* table) {
  if(table == NULL) return;
  if(table->types != NULL)
    free(table->types);
  if(table->f != NULL)
    fclose(table->f);
  if(table->values[0] != NULL)
    free(table->values[0]);
  if(table->values != NULL)
    free(table->values);
  free(table);
}

/* 
   Returns the number of columns of the table 
*/
int table_ncols(table_t* table) {
  if(table == NULL) return -1;
  return table->nColumns;
}

/* 
   Returns the array with the data types of the columns of the
   table. Note that typically this kind of function doesn't make a
   copy of the array, rather, it returns a pointer to the actual array
   contained in the table structure. This means that the calling
   program should not, under any circumstance, modify the array that
   this function returns.
 */
type_t* table_types(table_t* table) {
  if(table == NULL) return NULL;
  return table->types;
}

/* 
   Returns the position in the file of the first record of the table 
*/
long table_first_pos(table_t* table) {
  int headerSize;
  if(table == NULL) return 0L;
  if(table->f == NULL) return 0L;

  headerSize = (table->nColumns)*sizeof(type_t)+sizeof(int);
  fseek(table->f, headerSize, SEEK_SET);
  return ftell(table->f);
}

/* 
   Returns the position in the file in which the table is currently
   positioned. 
*/
long table_cur_pos(table_t* table) {
  if(table == NULL) return -1L;
  if(table->f==NULL) return -1L;
  return ftell(table->f);
}

/* 
   Returns the position just past the last byte in the file, where a
   new record should be inserted.
*/
long table_last_pos(table_t* table) {
  if(table == NULL) return -1L;
  if(table->f == NULL) return -1L;
  fseek(table->f, 0, SEEK_END);
  return ftell(table->f);
}

/* 
   Reads the record starting in the specified position. The record is
   read and stored in memory, but no value is returned. The value
   returned is the position of the following record in the file or -1
   if the position requested is past the end of the file.
*/
long table_read_record(table_t* table, long pos) {
  char *buffer;
  int i, length;
  if(table == NULL) return -1L;
  if(table->f == NULL) return -1L;
  if(table_last_pos(table) == pos) return -1L;

  if(table_last_pos(table) < pos || pos < 0){
    fprintf(stderr, "Error in table_read-record: Position out of range\n");
    return -1L;
  }

  if(table->values[0] != NULL)
    free(table->values[0]);

  fseek(table->f, pos, SEEK_SET);
  fread(&length, sizeof(int), 1, table->f);
  buffer = malloc((length+1)*sizeof(char));
  
  if(buffer == NULL) {
    fprintf(stderr, "Error in table_read-record: Out of memory\n");
    return -1L;
  }

  fread(buffer, sizeof(char), length, table->f);
  for(i = 0; i < table->nColumns; i++){
    table->values[i]=buffer;
    buffer += value_length(table->types[i], table->values[i]);
  }

  return ftell(table->f);
}

/*
  Returns a pointer to the value of the given column of the record
  currently in memory. The value is cast to a void * (it is always a
  pointer: if the column is an INT, the function will return a pointer
  to it).. Returns NULL if there is no record in memory or if the
  column doesn't exist.
*/
void *table_column_get(table_t* table, int col) {
  if(table==NULL) return NULL;
  if(table->values==NULL) return NULL;
  if(table->types==NULL) return NULL;

  return table->values[col];
}

/* 
   Inserts a record in the last available position of the table. Note
   that all the values are cast to void *, and that there is no
   indication of their actual type or even of how many values we ask
   to store... why?
  */
void table_insert_record(table_t* table, void** values) {
  int length, lenAux, i;
  if(table==NULL || values==NULL) return;
  if(table->values==NULL) return;
  if(table->types==NULL) return;

  fseek(table->f, 0, SEEK_END);

  /*First we meassure the total record length*/
  for(i=0, length=0; i < table->nColumns; i++){
     length += (int)value_length(table->types[i], values[i]);
  }

  fwrite(&length, sizeof(int), 1, table->f);
  for(i=0; i < table->nColumns; i++){
  	lenAux = (int)value_length(table->types[i], values[i]);
    if(table->types[i]==INT){
      fwrite(values[i], lenAux, 1, table->f);
    }else if(table->types[i]==STR){
      fwrite(values[i], lenAux, 1, table->f);
    }else if(table->types[i]==LLNG){
      fwrite(values[i], lenAux, 1, table->f);
    }else if(table->types[i]==DBL){
      fwrite(values[i], lenAux, 1, table->f);
    }
  }
  return;
}
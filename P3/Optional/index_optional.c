#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "type.h"
#include "index_optional.h"

#define NUM_ADD 100

/*Here we store a set of records which have the same key*/
typedef struct {
	void *key;
	int nPos;
	int maxPos;
	long *pos;
} irecord;

struct index_ {
	/*We define an array of records pointers*/
	int numRecords, numMax;
	type_t keyType;
	char *filename;
	irecord **records;
    
};

int cmp(type_t type, void *value, void *key){
	if(type == INT){
		return *(int *)value - *(int *)key;
	}else{
		return strcmp((char*)value, (char*)key);
	}
}


/* 
   Creates a file for saving an empty index. The index is initialized
   to be of the specific tpe (in the basic version this is always INT)
   and to contain 0 entries.
 */
int index_create(type_t type, char* path) {
	FILE *f = NULL;
	int numRecords;
	numRecords = 0;
	if(path == NULL) return -1;
	if(type != INT && type != STR) return -1;
	f = fopen(path, "w");
	if(f == NULL) return -1;
	fwrite(&type, sizeof(type_t), 1, f);
	fwrite(&numRecords, sizeof(int), 1, f);
	fclose(f);
	return 0;
}

/*Auxiliar function to move on the index.
It returns the last position of the index but 
does not modify the actual position*/
long index_last_pos(FILE* f) {
  long actual, last;
  if(f == NULL) return -1L;
  actual = ftell(f);
  fseek(f, 0, SEEK_END);
  last = ftell(f);
  fseek(f, actual, SEEK_SET);
  return last;
}

/* 
   Opens a previously created index: reads the contents of the index
   in an index_t structure that it allocates, and returns a pointer to
   it (or NULL if the files doesn't exist or there is an error). 

   NOTE: the index is stored in memory, so you can open and close the
   file in this function. However, when you are asked to save the
   index, you will not be given the path name again, so you must store
   in the structure either the FILE * (and in this case you must keep
   the file open) or the path (and in this case you will open the file
   again).
 */
index_t* index_open(char* path) {
	FILE *f = NULL;
	char *filename;
	index_t *index;
	long lastPos;
	int numRecords, keyLen, i, j;
	type_t keyType;

	if(path == NULL) return NULL;
	f=fopen(path, "r+");
  	if(f == NULL) return NULL;
  	fread(&keyType, sizeof(type_t), 1, f);
  	fread(&numRecords, sizeof(int), 1, f);

  	filename = (char*)malloc(sizeof(char)*(strlen(path)+1));
  	if(filename ==NULL){
  		fclose(f);
  		return NULL;
  	}
  	strcpy(filename, path);

  	/*We alloc extra memory so that we dont need to re-alloc on each insert*/
  	index = (index_t *)malloc(sizeof(index_t));
  	if(index == NULL){
  		free(filename);
  		fclose(f);
  		return NULL;
  	}

  	index->records = (irecord **)malloc(sizeof(irecord *)*(numRecords+NUM_ADD));
  	if(index->records == NULL){
  		free(filename);
  		fclose(f);
  		free(index);
  		return NULL;
  	}

  	index->keyType = keyType;
  	index->numRecords = numRecords;
  	index->numMax = numRecords+NUM_ADD;
  	index->filename = filename;

  	/*Now we need to read all the index into memory,
  	so we need a loop till the end of the file*/
  	lastPos = index_last_pos(f);
  	j=0;
  	while(ftell(f) != lastPos){
  	  	irecord *record;
  	  	record = (irecord *)malloc(sizeof(irecord));
  	  	if(record == NULL){
  	  		free(filename);
  			fclose(f);
  			free(index->records);
  			for(i = 0; i < j; i++){
  				free(index->records[j]->pos);
  				free(index->records[j]);
  			}
  			free(index);
  			return NULL;
  	  	}
  	  	
  	  	fread(&keyLen, sizeof(int), 1, f);
  	  	if(keyType == INT){
  	  		record->key = (int*)malloc(sizeof(int));
  	  	}else if(keyType == STR){
  	  		record->key = (char*)malloc((sizeof(char)*(keyLen+1)));
  	  	}

  	  	if(record->key == NULL){
  	  		free(filename);
  			fclose(f);
  			free(record);
  			free(index->records);
  			for(i = 0; i < j; i++){
  				free(index->records[j]->pos);
  				free(index->records[j]);
  			}
  			free(index);
  			return NULL;
  	  	}

  	  	fread((record->key), keyLen, 1, f);
  	  	fread(&(record->nPos), sizeof(int), 1, f);
  	  	
  	  	record->maxPos = record->nPos + NUM_ADD;
  	  	record->pos = (long*)malloc(sizeof(long)*(record->nPos + NUM_ADD));
  	  	if(record->pos == NULL){
  	  		free(filename);
  			fclose(f);
  			free(record);
  			free(record->key);
  			free(index->records);
  			for(i = 0; i < j; i++){
  				free(index->records[j]->pos);
  				free(index->records[j]);
  			}
  			free(index);
  			return NULL;
  	  	}
  	  	for(i=0; i<record->nPos; i++){
  	  		fread(&(record->pos[i]), sizeof(long), 1, f);
  	  	}
  	  	index->records[j]=record;
  	  	j++;
  	}

  	fclose(f);
  return index;
}

/* 
   Saves the current state of index in the file it came from. See the
   NOTE to index_open.
*/
int index_save(index_t* index) {
	/*Re-write the file entirely*/
	FILE *f;
	int keySize, i, j;
	irecord *record;
	if(index == NULL) return -1;
	if(index->filename == NULL) return -1;

	f = fopen(index->filename, "w");
	if(f == NULL) return -1;

	fwrite(&(index->keyType), sizeof(int), 1, f);
	fwrite(&(index->numRecords), sizeof(int), 1, f);
	for(i=0; i<index->numRecords; i++){
		record = index->records[i];
		keySize = (int)value_length(index->keyType, record->key);
		fwrite(&keySize, sizeof(int), 1, f);
		fwrite((record->key), keySize, 1, f);
		fwrite(&(record->nPos), sizeof(int), 1, f);
		for(j=0; j < record->nPos; j++){
			fwrite(&(record->pos[j]), sizeof(long), 1, f);
		}
	}
	fclose(f);
	return 0;
}

/*Auxiliar function in order to know if a key is inserted on the table
It allows us to simplify the index_put and index_get functions.
Returns -1 if the key is not on the table, or the position where the key is
inside index->records 
*/
int indexKeyExists(index_t *index, void* key){
	int m, P, U;
	P=0;
	U=index->numRecords-1;

	while(P<=U){
		m=(P+U)/2;	
		if(cmp(index->keyType, index->records[m]->key, key) == 0){
			return m;
		}
		if(cmp(index->keyType, index->records[m]->key, key) > 0){
			U=m-1;
		}
		else{
			P=m+1;
		}	
	}
	return -1;
}


/* 
   Puts a pair key-position in the index. Note that the key may be
   present in the index or not... you must manage both situation. Also
   remember that the index must be kept ordered at all times.
*/
int index_put(index_t *index, void *key, long pos) {
	int i;
	irecord *record;
	void *keyAux;
	if(index == NULL) return -1;
	/*First of all we need to check whether the key is or nor in the list*/
	i = indexKeyExists(index, key);
	/*If the key exists, we just need to insert another position.*/
	if(i != -1){
		record=index->records[i];
		/*Check if there is enought space left*/
		if(record->nPos +1 >= record->maxPos){
			long *pos = (long*)realloc(record->pos, record->nPos +1+NUM_ADD);
			if(pos == NULL) return -1;
			free(record->pos);
			record->pos=pos;
			record->maxPos = index->numRecords + 1 + NUM_ADD;
		}
		record->pos[record->nPos] = pos;
		record->nPos += 1;
	}else{
		/*Check if there is enought space left*/
		if(index->numRecords + 1 >= index->numMax){
			irecord **recordsAux;
			recordsAux = (irecord**)realloc(index->records, index->numRecords + 1+NUM_ADD);
			if(recordsAux == NULL) return -1;
			index->numMax = index->numRecords + 1 + NUM_ADD;
			/*Now we need to free our previous array*/
			free(index->records);
			index->records = recordsAux;
	
		}
		for(i=index->numRecords-1; i>=0 && cmp(index->keyType, index->records[i]->key, key) > 0; i--){
			index->records[i+1] = index->records[i];
		}
	
	  	record = (irecord *)malloc(sizeof(irecord));
	  	if(record == NULL){
	  		return -1;
	  	}
	
	  	/*Cambiarlo para la practica opcional, seria cambiar el type_t*/
	  	if(index->keyType == INT){
	  		keyAux = (int *)malloc(sizeof(int));
	  		if(keyAux == NULL){
	  			free(record);
	  			return -1;
	  		}
	  		*(int *)keyAux = *(int *)key;
	  	}else{
	  		keyAux = (char *)malloc(value_length(STR, key) +1);
	  		if(keyAux == NULL){
	  			free(record);
	  			return -1;
	  		}
	  		strcpy((char *)keyAux, (char *)key);
	  	}

	  	record->key = keyAux;
	  	record->nPos = 1;
	  	record->maxPos = record->nPos + NUM_ADD;
	  	record->pos = (long*)malloc(sizeof(long)*(record->nPos + NUM_ADD));
	  	if(record->pos == NULL){
	  	  	free(record);
	  	  	return -1;
	  	}
	  	record->pos[0]=pos;
	  	index->numRecords += 1;
		index->records[i+1]=record;
	}

 	return 0;
}

/* 
   Retrieves all the positions associated with the key in the index. 
   
   NOTE: the parameter nposs is not an array of integers: it is
   actually an integer variable that is passed by reference. In it you
   must store the number of elements in the array that you return,
   that is, the number of positions associated to the key. The call
   will be something like this:

   int n
   long **poss = index_get(index, key, &n);

   for (int i=0; i<n; i++) {
       Do something with poss[i]
   }

   ANOTHER NOTE: remember that the search for the key MUST BE DONE
   using binary search.

*/
long *index_get(index_t *index, void *key, int *nposs) {
	int result;
	/*We return the original array without allocating memory for it,
	as we consider the user will not modify it, just read it.*/
	result = indexKeyExists(index, key);
	if(result == -1){
		*nposs = 0;
		return NULL;
	}

	*nposs = index->records[result]->nPos;
 	return index->records[result]->pos;
}

/* 
   Closes the index by freeing the allocated resources 
*/
void index_close(index_t *index) {
	int i;
	irecord *record;
	if(index == NULL) return;
	if(index->filename != NULL) free(index->filename);
	if(index->records != NULL){
		for(i=0; i<index->numRecords; i++){
			record = index->records[i];
			if(record != NULL){
				free(record->key);
				free(record->pos);
				free(record);
			}
		}
		free(index->records);
	}
	free(index);
}



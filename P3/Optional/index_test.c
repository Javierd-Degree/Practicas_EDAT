#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "table.h"
#include "index_optional.h"

void createIndex(char *path, char* db_path){
	FILE *f = NULL;
	long pos, prevPos;
	int result;
	index_t *index;
	table_t *table;

	f = fopen(path, "r");
    if(f==NULL){
        /*El score es un int*/
        index_create(INT, path);    
    }else fclose(f);

    index = index_open(path);
    if(index == NULL){
    	printf("Error while opening the index\n");
    	return;
    } 

    table = table_open(db_path);
    if(table==NULL){
    	printf("Error while opening the database\n");
    	index_close(index);
    	return;
    } 
	
	/*Read the entire tabe and create the index*/
	prevPos = table_first_pos(table);
	while( (pos = table_read_record(table, prevPos)) != -1){
		/*Print the record*/
		result = index_put(index, table_column_get(table, 2), prevPos);
	    if(result == -1){
	        printf("Error while inserting in the index\n");
	        index_close(index);
	        table_close(table);
	        return;
	    }

	    prevPos = pos;
	}

	index_save(index);
}

int* get_books(int score, int *numResults){
	FILE *f;
    long *books_pos, pos;
    int *books, i;
    char *index_path = "index_score.dat";
    char *db_path = "score.dat";
    type_t types[]={STR, STR, INT, INT};
    index_t* index;
    table_t* table;
    
    if(score < 1||score > 100){
    	printf("The score searched is not accepted, try a score between 1 and 100.\n");
    	return NULL;
    }	

    f = fopen(index_path, "r");
    if(f==NULL){
        /*El score es un int*/
        createIndex(index_path, db_path);    
    }else fclose(f);

    index = index_open(index_path);
    if(index==NULL){
    	printf("Error while opening the index\n");
    	return NULL;
    }

    table = table_open(db_path);
    if(table==NULL){
    	printf("Error while opening the database\n");
    	index_close(index);
    	return NULL;
    } 

    /*First of all, print the entire table*/
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
		printf("\n");
	}
	printf("\n\n\nNow we get all the records with score %d\n", score);

    /*First, we need to read all the positions of the records*/
    books_pos = index_get(index, &score, numResults);
    if(*numResults == 0){
    	index_close(index);
    	table_close(table);
    	return NULL;
    }

    books = (int*)malloc(sizeof(int) * (*numResults));
    if(books == NULL){
    	printf("Not enough memory.\n");
    	table_close(table);
    	index_close(index);
    	return NULL;
    }

    for(i=0; i<(*numResults); i++){
    	table_read_record(table, books_pos[i]);
    	books[i]=*(int*)table_column_get(table, 3);
    }

    index_close(index);
    table_close(table);
    return books;	
}

int main(int argc, char** argv) {
    int *books, score, i, numResults;

    if (argc < 2) {
        printf("Not enough arguments.\n");
        return EXIT_FAILURE;
    }

    score = atoi(argv[1]);
    books = get_books(score, &numResults);
    if(books == NULL){
    	return EXIT_SUCCESS;
    }

    if(numResults == 0){
    	printf("There are no books with the given score.\n");
    	/*As there are no results, we have not allocated memory for 
    	books table, so we do not need to free it*/
    	return EXIT_SUCCESS;
    }

    for(i=0; i < numResults; i++){
        printf("Book id: %d\n", books[i]);
    }
    free(books);
    return EXIT_SUCCESS;
}
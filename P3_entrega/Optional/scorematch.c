#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "table.h"
#include "index_optional.h"

#define MAX_TITLE 1024

void createIndex(char *path, char* db_path){
	long pos, prevPos;
	int result;
	index_t *index;
	table_t *table;

    index_create(STR, path);

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
		result = index_put(index, table_column_get(table, 1), prevPos);
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

void get_booksTitle(int score, table_t* table){
    long *books_pos;
    int i, numResults;
    char *index_path = "index_score.dat";
    index_t* index;

    /*We assume that score_index and suggest_index have
    already been run, otherwise the index would't exists*/
    index = index_open(index_path);
    if(index==NULL){
        printf("Error while opening the index\n");
        return;
    }

    /*First, we need to read all the positions of the records*/
    books_pos = index_get(index, &score, &numResults);
    if(numResults == 0){
        index_close(index);
        return;
    }

    /*Once we have the bookscores of each book, we need to
    get the title of every book with that score*/

    for(i=0; i< numResults; i++){
        table_read_record(table, books_pos[i]);
        printf("%s\n", (char*)table_column_get(table, 1));
    }

    index_close(index);
    return;   
}


void get_books(char* title, int *numResults){
    long *books_pos;
    int i, score;
    char *index_path = "index_scorematch.dat";
    char *db_path = "score.dat";
    index_t* index;
    table_t* table;
    
    if(title == NULL){
    	return;
    }	

    createIndex(index_path, db_path);

    index = index_open(index_path);
    if(index==NULL){
    	printf("Error while opening the index\n");
    	return;
    }

    table = table_open(db_path);
    if(table==NULL){
    	printf("Error while opening the database\n");
    	index_close(index);
    	return;
    } 

    /*First, we need to read all the positions of the records*/
    books_pos = index_get(index, title, numResults);
    if(*numResults == 0 || books_pos == NULL){
    	index_close(index);
    	table_close(table);
    	return;
    }

    /*We need to get the scores of the books with that title
    Then, we call get_booksTitle to print its names*/
    for(i=0; i<(*numResults); i++){
        table_read_record(table, books_pos[i]);
        score = *(int*)table_column_get(table, 2);
        get_booksTitle(score, table);
    }

    index_close(index);
    table_close(table);
    return;	
}

int main(int argc, char** argv) {
    int numResults;
    char title[MAX_TITLE];

    if (argc < 2) {
        printf("Not enough arguments.\n");
        return EXIT_FAILURE;
    }

    strcpy(title, argv[1]);
    get_books(title, &numResults);
    if(numResults == 0){
    	printf("There are no books with the given title.\n");
    	/*As there are no results, we have not allocated memory for 
    	books table, so we do not need to free it*/
    	return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
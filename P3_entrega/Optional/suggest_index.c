#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "table.h"
#include "index_optional.h"

SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt;
SQLRETURN ret; /* ODBC API return status */


int* get_books(int score, int *numResults){
    long *books_pos;
    int *books, i;
    char *index_path = "index_score.dat";
    char *db_path = "score.dat";
    index_t* index;
    table_t* table;
    
    if(score < 1||score > 100){
    	printf("The score searched is not accepted, try a score between 1 and 100.\n");
    	return NULL;
    }	

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
    char book[513], author[257];

    if (argc < 2) {
        printf("Not enough arguments.\n");
        return EXIT_FAILURE;
    }

    score = atoi(argv[1]);
    books = get_books(score, &numResults);
    if(numResults == 0 || books == NULL){
    	printf("There are no books with the given score.\n");
        /*As there are no results, we have not allocated memory 
        for books table, so we do not need to free it*/
    	return EXIT_SUCCESS;
    }

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    for(i=0; i < numResults; i++){
        /* Allocate a statement handle */
        SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

        SQLPrepare(stmt, (SQLCHAR*) "SELECT DISTINCT authors.name, books.title FROM books JOIN relations ON(books.ID = relations.book_id) JOIN authors ON(authors.ID = relations.author_id) WHERE authors.id IN(SELECT author_id FROM relations WHERE book_id = ?) ORDER BY authors.name", SQL_NTS);
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &books[i], 0, NULL);
        ret = SQLExecute(stmt);
        if(!SQL_SUCCEEDED(ret)){
        	printf("BindParameter ERROR\n");
        	return -1;
        }
        SQLBindCol(stmt, 1, SQL_C_CHAR, author, sizeof(author), NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, book, sizeof(book), NULL);
       
        while(SQL_SUCCEEDED(ret=SQLFetch(stmt))){
        	printf("%s\t%s\n", author, book);
        }
       
        SQLCloseCursor(stmt);

        /* free up statement handle */
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    }
    free(books);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


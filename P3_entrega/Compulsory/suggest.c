#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "table.h"

SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt;
SQLRETURN ret; /* ODBC API return status */


/*This function searches the table and returns an array with the ids of the books with the given score (passed as argument)*/
int* get_books(int score){
    long pos;
    int *books, k=1;
    char* path = "score.dat";
    table_t* table;
    
    if(score < 1||score > 100){
    	printf("The score searched is not accepted, try a score between 1 and 100.\n");
    	return NULL;
    }	

    table = table_open(path);
    if(table==NULL){
    	printf("Error while opening the database\n");
    	return NULL;
    } 

    /*We need to count the number of records in order to allocate memory*/
    /*Start with k=1 so that le last position is -1 and we can know where 
    the array ends*/
    pos = table_first_pos(table);
	while( (pos = table_read_record(table, pos)) != -1){
		if(*(int*)table_column_get(table, 2) == score){
        	k++;
    	}
	}

    books=(int*)malloc(sizeof(int)*k);
    if(books == NULL){
        printf("Not enough memory.\n");
        table_close(table);
        return NULL;
    }

    k=0;
    /*Now, we read the data into the array*/
    pos = table_first_pos(table);
	while( (pos = table_read_record(table, pos)) != -1){
		if(*(int*)table_column_get(table, 2) == score){
			books[k]=*(int*)table_column_get(table, 3);
        	k++;
    	}
	}

    table_close(table);
    /*We set the last element of the array to -1 so that we know where the array ends*/
    books[k]=-1;
    return books;	
}

int main(int argc, char** argv) {
    int *books, score, i;
    char book[513], author[257];

    if (argc < 2) {
        printf("Not enough arguments.\n");
        return EXIT_FAILURE;
    }

    score = atoi(argv[1]);
    /*We call the function get_books so that we get the array of the books with the given score*/
    books = get_books(score);
    if(books == NULL){
    	return EXIT_SUCCESS;
    }
    /*if the first element of the array is -1 there are no books with the given score*/
    if(books[0] == -1){
    	printf("There are no books with the given score.\n");
    	return EXIT_SUCCESS;
    }

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    for(i=0; books[i]!=-1; i++){
        

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

     
    }

    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    free(books);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


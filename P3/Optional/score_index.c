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
SQLHSTMT stmt1, stmt2;
SQLRETURN ret; /* ODBC API return status */


int fill_scores(char *db_path, char *index_path, void** values){
    table_t *table;
    index_t* index;
    long pos;
    int result;
    if(db_path == NULL || index_path == NULL || values == NULL) return EXIT_FAILURE;

    table = table_open(db_path);
    if(table == NULL) return EXIT_FAILURE;
    index = index_open(index_path);
    if(index == NULL){
        table_close(table);
        return EXIT_FAILURE;
    }

    /*Get the position of the record that is going to be inserted*/
    pos = table_last_pos(table);
    table_insert_record(table, values);
    
    /*Once the record is inserted, we need to insert the score in the index.
    We need to store the score as the key, that is the content of values[2]*/
    result = index_put(index, values[2], pos);
    if(result == -1){
        printf("Error while inserting in the index\n");
        index_close(index);
        table_close(table);
        return EXIT_FAILURE;
    }


    index_save(index);
    index_close(index);
    table_close(table);
    return EXIT_SUCCESS;
}


int main(int argc, char** argv) {
    int score, book_id, res;
    char *db_path = "score.dat", *index_path = "index_score.dat";
    char *title, ISBN[20];
    void** values;
    FILE* f=NULL;
    type_t types[]={STR, STR, INT, INT};


    if (argc < 3) {
        printf("Faltan datos\n");
        return EXIT_FAILURE;
    }

    

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt1);
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt2);
    title=(char*)malloc(sizeof(char)*(strlen(argv[1])+1));
    title=strcpy(title,argv[1]);
    score=atoi(argv[2]);

    if(score<1||score>100){
    	printf("The score is not valid, try a score between 1 and 100.\n");
    	free(title);
    	return EXIT_SUCCESS;
    }

    SQLPrepare(stmt1, (SQLCHAR*) "SELECT ID FROM books WHERE title = ?", SQL_NTS);
    SQLBindParameter(stmt1, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, title, 0, NULL);
    ret = SQLExecute(stmt1);
    if(!SQL_SUCCEEDED(ret)){
    	printf("BindParameter ERROR.\n");
    	return EXIT_FAILURE;
    }
    
    SQLBindCol(stmt1, 1, SQL_C_SLONG, &book_id, sizeof(book_id), NULL);
   
    ret=SQLFetch(stmt1);
    if (!SQL_SUCCEEDED(ret)) {
        printf("The given book does not exist.\n");
        return EXIT_SUCCESS;
    }

    SQLCloseCursor(stmt1);

    
    SQLPrepare(stmt2, (SQLCHAR*) "SELECT ISBN FROM editions WHERE book_id = ?", SQL_NTS);
    SQLBindParameter(stmt2, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &book_id, 0, NULL);
    ret = SQLExecute(stmt2);
    if(!SQL_SUCCEEDED(ret)){
    	printf("BindParameter ERROR.\n");
    	return EXIT_FAILURE;
    }

    SQLBindCol(stmt2, 1, SQL_C_CHAR, ISBN, sizeof(ISBN), NULL);
    ret=SQLFetch(stmt2);
    if (!SQL_SUCCEEDED(ret)){
       printf("The given book does not exist.\n");
        return EXIT_SUCCESS;
    }

    SQLCloseCursor(stmt2);

    values=(void**)malloc(sizeof(void*)*4);
    if(values==NULL){
        free(title);
        return EXIT_FAILURE;
    }

    /*The table needs 4 columns, ISBN, book title, book score
    and book id*/
    values[0]=(void*)ISBN;
    values[1]=(void*)title;
    values[2]=(void*)&score;
    values[3]=(void*)&book_id;

    /*Check if the table already exists, otherwise, create it*/ 
    f = fopen(db_path, "r");
    if(f==NULL){
    	table_create(db_path, 4, types);	
    }else fclose(f);
 	
    /*We check if the index already exists before creating it*/ 
    f = fopen(index_path, "r");
    if(f==NULL){
        /*El score es un int*/
        index_create(INT, index_path);    
    }else fclose(f);

    res = fill_scores(db_path, index_path, values);
    if (res == EXIT_FAILURE){
        printf("The scores table/index could not be updated.\n");
        return EXIT_FAILURE;
    }

    free(title);
    free(values);
    
    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt2);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
    	printf("Disconnection ERROR.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


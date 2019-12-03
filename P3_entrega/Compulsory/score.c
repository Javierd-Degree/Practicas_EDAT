#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "table.h"


SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt1, stmt2;
SQLRETURN ret; /* ODBC API return status */

/*This function fills the table in the file (given by the path) with the values received as voi** */
int fill_scores(char *path, void** values){
    table_t *table;
    if(path == NULL || values == NULL) return EXIT_FAILURE;

    table = table_open(path);
    if(table == NULL){
        printf("Error while opening the database\n");
        return EXIT_FAILURE;
    } 
    table_insert_record(table, values);
    table_close(table);

    return EXIT_SUCCESS;
}


int main(int argc, char** argv) {
    int score, book_id, res;
    char *path = "score.dat", *title, ISBN[20];
    void** values;
    FILE* f=NULL;
    /*We set the types of the table in the declaration*/
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
    title = (char*)malloc(sizeof(char)*(strlen(argv[1])+1));
    title = strcpy(title,argv[1]);
    score = atoi(argv[2]);
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

    /*We check if the table already exists before creating it*/ 
    f=fopen(path, "r");
    if(f==NULL){
    	table_create(path, 4, types);	
    }else fclose(f);
 	
    /*We call fill_scores to fill the table with the data in values** */
    res = fill_scores(path, values);
    if (res==EXIT_FAILURE){
        printf("The scores table could not be updated.\n");
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


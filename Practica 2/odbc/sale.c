#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"

SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt;
SQLRETURN ret; /* ODBC API return status */

int storeSale(int discount, char *startDate, char *endDate, char *ISBN){
	if(startDate == NULL || endDate == NULL || ISBN == NULL) return EXIT_FAILURE;

	/*stmt, numero de interrogacion, tipo, tipo en c, tipo en sql, tamaño columna, numro decimales, 
    direccion memoria, buffer lenght, el tamaño ( te lo reserva solo)*/
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 13, 0, ISBN, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &discount, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(startDate), 0, (SQLCHAR *)startDate, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(endDate), 0, (SQLCHAR *)endDate, 0, NULL);
        
    ret = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", &env, SQL_HANDLE_ENV);
    }

    SQLCloseCursor(stmt);
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    SQLINTEGER discount;
    int i, result;
    char startDate[11], endDate[11], ISBN[20];

    if (argc < 5) {
        printf("Faltan datos\n");
        return EXIT_FAILURE;
    }

    discount = atoi(argv[1]);
    strcpy(startDate, argv[2]);
    strcpy(endDate, argv[3]);

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLPrepare(stmt, (SQLCHAR*) "INSERT INTO discounts(ISBN, discount, start_date, end_date) VALUES (?, ?, CAST(? AS DATE), CAST(? AS DATE))", SQL_NTS);

	for(i=4; i<argc; i++){
        strcpy(ISBN, argv[i]);

        result = storeSale(discount, startDate, endDate, ISBN);
        if( result == EXIT_FAILURE ){
        	printf("Fallo al insertar %s\n", ISBN);
        	return EXIT_FAILURE;
        }else{
        	printf("Libro con ISBN %s insertado.\n", ISBN);
        }
    }


    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


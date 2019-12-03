#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"

SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt1, stmt2, stmt3;
SQLRETURN ret; /* ODBC API return status */


int fill_sales(int inv, int user, char *ISBN, float price, int discount, int pay, char *sDate){
	if(sDate == NULL || ISBN == NULL) return EXIT_FAILURE;

	/*stmt, numero de interrogacion, tipo, tipo en c, tipo en sql, tamaño columna, numro decimales, 
    direccion memoria, buffer lenght, el tamaño ( te lo reserva solo)*/
    ret = SQLBindParameter(stmt3, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inv, 0, NULL);

    ret = SQLBindParameter(stmt3, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &user, 0, NULL);

    ret = SQLBindParameter(stmt3, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, ISBN, 0, NULL);

    ret = SQLBindParameter(stmt3, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_DECIMAL, 0, 0, &price, 0, NULL);

    ret = SQLBindParameter(stmt3, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &discount, 0, NULL);
 
    ret = SQLBindParameter(stmt3, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &pay, 0, NULL);

    ret = SQLBindParameter(stmt3, 7, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(sDate), 0, (SQLCHAR*)sDate, 0, NULL);

    ret = SQLExecute(stmt3);
    if (!SQL_SUCCEEDED(ret)) {
        printf("executed3 ");
        odbc_extract_error(" here3", &env, SQL_HANDLE_ENV);
        printf("end3");
    }

    SQLCloseCursor(stmt3);
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    int inv, user, result, discount, dis_perc;
    float price, ini_price;
    char sDate[11], ISBN[20];
    FILE* pf;


    if (argc < 2) {
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
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt3);


    pf=fopen(argv[1], "r");
    while(fscanf(pf, "%d\t%d\t%s\t%s", &inv, &user, ISBN, sDate)==4){

        SQLPrepare(stmt1, (SQLCHAR*) "SELECT ID, discount FROM discounts WHERE ISBN = ?", SQL_NTS);
        SQLBindParameter(stmt1, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, ISBN, 0, NULL);
        ret = SQLExecute(stmt1);
        if(!SQL_SUCCEEDED(ret)){
        	printf("NOOOOO\n\n\n\n");
        	return -1;
        }
        
        SQLBindCol(stmt1, 1, SQL_C_SLONG, &discount, sizeof(discount), NULL);
        SQLBindCol(stmt1, 2, SQL_C_SLONG, &dis_perc, sizeof(dis_perc), NULL);
        ret=SQLFetch(stmt1);
        if (!SQL_SUCCEEDED(ret)) {
            printf("executed1 ");
            odbc_extract_error(" here1 ", &env, SQL_HANDLE_ENV);
            printf("end1");
        }

        SQLCloseCursor(stmt1);

        
        SQLPrepare(stmt2, (SQLCHAR*) "SELECT price FROM editions WHERE ISBN = ?", SQL_NTS);
        SQLBindParameter(stmt2, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, ISBN, 0, NULL);
        ret = SQLExecute(stmt2);
        if(!SQL_SUCCEEDED(ret)){
        	printf("NOOOOO\n\n\n\n");
        	return -1;
        }

        SQLBindCol(stmt2, 1, SQL_C_FLOAT, &ini_price, sizeof(float), NULL);
        ret=SQLFetch(stmt2);
        if (!SQL_SUCCEEDED(ret)){
            printf("executed2 ");
            odbc_extract_error(" here2 ", &env, SQL_HANDLE_ENV);
            printf("end2");
        }

        SQLCloseCursor(stmt2);


        price = ini_price-(ini_price*dis_perc/100);

        
        SQLPrepare(stmt3, (SQLCHAR*) "INSERT INTO sales(invoiceid, user_id, ISBN, price, discount_id, payment_method, sale_date) VALUES (?, ?, ?, ?, ?, ?, ?)", SQL_NTS);

        result = fill_sales(inv, user, ISBN, price, discount, 0, sDate);
        if( result == EXIT_FAILURE ){
            printf("Fallo al insertar %s\n", ISBN);
            fclose(pf);
            return EXIT_FAILURE;
        }else{
            /*printf("Libro con ISBN %s insertado.\n", ISBN);*/
        }  
    }

    fclose(pf);

    

    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt2);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt3);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


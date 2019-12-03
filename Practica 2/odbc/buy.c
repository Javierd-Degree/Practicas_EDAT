#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"

SQLHENV env;
SQLHDBC dbc;
SQLHSTMT stmt_del;
SQLHSTMT stmt_add;
SQLRETURN ret; /* ODBC API return status */

int getInvoiceId(){
    SQLHSTMT stmt;
    int invoiceId;

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLPrepare(stmt, (SQLCHAR*) "SELECT MAX(invoiceid) FROM sales", SQL_NTS);
    ret = SQLExecute(stmt);
    if(!SQL_SUCCEEDED(ret)){
        SQLFreeHandle(SQL_HANDLE_STMT, stmt_del);
        return -1;
    }

    SQLBindCol(stmt, 1, SQL_C_SLONG, &invoiceId, sizeof(invoiceId), NULL);
    if (SQL_SUCCEEDED(ret = SQLFetch(stmt))){
        invoiceId++;
    }

    printf("Factura %d\n", invoiceId);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return invoiceId;
}

float getPrice(char *ISBN){
    SQLHSTMT stmt;
    float price;
    if(ISBN == NULL) return -1;

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLPrepare(stmt, (SQLCHAR*) "SELECT price FROM editions WHERE ISBN = ?", SQL_NTS);
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 13, 0, (SQLCHAR *)ISBN, 0, NULL);
    ret = SQLExecute(stmt);
    if(!SQL_SUCCEEDED(ret)){
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return -1;
    }

    SQLBindCol(stmt, 1, SQL_C_FLOAT, &price, sizeof(price), NULL);
    if (!SQL_SUCCEEDED(ret = SQLFetch(stmt))){
        price =-1;
    }
    
    printf("Precio %lf\n", price);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return price; 
}

int getUserId(char *nick){
    SQLHSTMT stmt;
    int id;
    if(nick == NULL) return -1;

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLPrepare(stmt, (SQLCHAR*) "SELECT ID FROM users WHERE nick = ?", SQL_NTS);
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, strlen(nick), 0, (SQLCHAR *)nick, 0, NULL);
    ret = SQLExecute(stmt);
    if(!SQL_SUCCEEDED(ret)){
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return -1;
    }

    SQLBindCol(stmt, 1, SQL_C_SLONG, &id, sizeof(id), NULL);
    if (!SQL_SUCCEEDED(ret = SQLFetch(stmt))){
        id = -1;
    }
    printf("UserId %d\n", id);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return id;
}

int addBuy(char *ISBN, int invoiceId, int userId){
    float price;
	if(ISBN == NULL) return EXIT_FAILURE;

    /*Coger el precio de editions segun ISBN, coger el descuento de discounts egun ISBN, quitarle el descuento, guardar este precio.
    Coger los expenses del usuario, sumarselo, y guardarlo (update)*/

	/*stmt, numero de interrogacion, tipo, tipo en c, tipo en sql, tamaño columna, numro decimales, 
    direccion memoria, buffer lenght, el tamaño ( te lo reserva solo)*/

    price = getPrice(ISBN);

    SQLBindParameter(stmt_add, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &invoiceId, 0, NULL);
    SQLBindParameter(stmt_add, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
    SQLBindParameter(stmt_add, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 13, 0, (SQLCHAR *)ISBN, 0, NULL);
    SQLBindParameter(stmt_add, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_DECIMAL, 0, 0, &price, 0, NULL);
        
    ret = SQLExecute(stmt_add);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error(" here ", &env, SQL_HANDLE_ENV);
    }

    SQLCloseCursor(stmt_add);
    return EXIT_SUCCESS;
}

int delBuy(int invoiceId){
    /*stmt, numero de interrogacion, tipo, tipo en c, tipo en sql, tamaño columna, numro decimales, 
    direccion memoria, buffer lenght, el tamaño ( te lo reserva solo)*/
    SQLBindParameter(stmt_del, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &invoiceId, 0, NULL);
      
    /*Cogemos el mayor invoiceid que haya y le sumamos 1, pero solo la pimera vez*/

    ret = SQLExecute(stmt_del);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error(" here ", &env, SQL_HANDLE_ENV);
    }

    SQLCloseCursor(stmt_del);
    return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    int i, result;
    int invoiceId;
    char operation[4], userName[128], ISBN[20];

    if (argc < 3) {
        printf("Faltan datos\n");
        return EXIT_FAILURE;
    }

    strcpy(operation, argv[1]);

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt_del);
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt_add);
    SQLPrepare(stmt_del, (SQLCHAR*) "DELETE FROM sales WHERE invoiceid = ?", SQL_NTS);
    SQLPrepare(stmt_add, (SQLCHAR*) "INSERT INTO sales(invoiceid, user_id, ISBN, price, payment_method) VALUES (?, ?, ?, ?, 0)", SQL_NTS);

    /*We don't insert any kind of discounts as we really don't know when the sales is inserted, and thats why we neither insert the sale date*/


    if(strcmp(operation, "del") == 0){
        invoiceId = atoi(argv[2]);
        delBuy(invoiceId);
    }else if(strcmp(operation, "add") == 0){
        int invoiceId, userId;

        strcpy(userName, argv[2]);
        invoiceId = getInvoiceId();
        userId = getUserId(userName);

        for(i=3; i<argc; i++){
            strcpy(ISBN, argv[i]);
            result = addBuy(ISBN, invoiceId, userId);
            if( result == EXIT_FAILURE ){
                printf("Fallo al insertar %s\n", ISBN);
                return EXIT_FAILURE;
            }else{
                /*printf("Libro con ISBN %s insertado.\n", ISBN);*/
            }
        }
    }


    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt_del);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt_add);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


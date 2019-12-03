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

int getDiscountid(char *ISBN){
    SQLHSTMT stmt;
    int id;
    if(ISBN == NULL) return -1;

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLPrepare(stmt, (SQLCHAR*) "SELECT ID FROM discounts WHERE ISBN = ?", SQL_NTS);
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 13, 0, (SQLCHAR *)ISBN, 0, NULL);
    ret = SQLExecute(stmt);
    if(!SQL_SUCCEEDED(ret)){
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return -1;
    }

    SQLBindCol(stmt, 1, SQL_C_SLONG, &id, sizeof(id), NULL);
    if (!SQL_SUCCEEDED(ret = SQLFetch(stmt))){
        id =-1;
    }
    
    printf("Precio %lf\n", id);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return id; 
}

int getDiscount(char *ISBN){
    SQLHSTMT stmt;
    int discount;
    if(ISBN == NULL) return -1;

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLPrepare(stmt, (SQLCHAR*) "SELECT discount FROM discounts WHERE ISBN = ?", SQL_NTS);
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 13, 0, (SQLCHAR *)ISBN, 0, NULL);
    ret = SQLExecute(stmt);
    if(!SQL_SUCCEEDED(ret)){
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return -1;
    }

    SQLBindCol(stmt, 1, SQL_C_SLONG, &discount, sizeof(discount), NULL);
    if (!SQL_SUCCEEDED(ret = SQLFetch(stmt))){
        discount =-1;
    }
    
    printf("Precio %lf\n", discount);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return discount; 
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

int addSale(char *ISBN, int invoiceId, int userId){
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

int main(int argc, char** argv) {
    int invoiceId, userId, discount, discountId;
    float price;
    char sDate[11], ISBN[20];
    FILE* pf = NULL;

    if (argc < 2) {
        printf("Faltan datos\n");
        return EXIT_FAILURE;
    }

    pf=fopen(argv[1], "r");
    if(pf == NULL) return -1;

    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt_add);
    SQLPrepare(stmt_add, (SQLCHAR*) "INSERT INTO sales(invoiceid, user_id, ISBN, price, discount_id, payment_method, sale_date) VALUES (?, ?, ?, ?, ?, ?, ?)", SQL_NTS);


    while(fscanf(pf, "%d\t%d\t%s\t%s", &invoiceId, &userId, ISBN, sDate)==4){
        price = getPrice(ISBN);
        discount = getDiscount(ISBN);
        discountId = getDiscountid(ISBN);

        printf("%f, %d, %d", price, discount, discountId);

    }

    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt_add);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


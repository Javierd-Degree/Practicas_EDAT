--Javier Delgado del Cerro
--Javier López Cano

/*
psql -U alumnodb -l
psql -U alumnodb -d  books -a -f ./practica.sql

psql -U alumnodb <name_file.txt> name_db.log 2>&1
*/

CREATE TABLE IF NOT EXISTS books_temp(ID SERIAL,
	author_name VARCHAR(512),
	book_name VARCHAR(512),
	format VARCHAR(512),
	--Int
	pages VARCHAR(128),
	editorial VARCHAR(512),
	--Date
	publication_date VARCHAR(20),
	language VARCHAR(512),
	ISBN VARCHAR(20));

COPY books_temp(author_name, book_name, format, pages, editorial, publication_date, language, ISBN) FROM '/home/javierd/Escritorio/LIBROS_FINAL.txt' WITH ENCODING 'ISO-8859-1' NULL '';

--Working
CREATE TABLE IF NOT EXISTS usuarios_temp(ID SERIAL,
	uid VARCHAR(10),
	nick VARCHAR(128),
	name VARCHAR(128),
	--Timestamp
	join_date VARCHAR(50),
	ccard VARCHAR(16),
	--Date
	expiration VARCHAR(20));

COPY usuarios_temp(uid, nick, name, join_date, ccard, expiration) FROM '/home/javierd/Escritorio/usuarios.txt' WITH ENCODING 'ISO-8859-1' NULL '';

--It fails because of the two las \n at the end
CREATE TABLE IF NOT EXISTS ISBN_Precios_temp(
	ISBN VARCHAR(20),
	--Real
	price VARCHAR(20));

COPY ISBN_Precios_temp(ISBN, price) FROM '/home/javierd/Escritorio/isbns_precios.txt'  WITH ENCODING 'ISO-8859-1' NULL '';

--Working
CREATE TABLE IF NOT EXISTS ventas_temp(
	invoiceid VARCHAR(10),
	userId VARCHAR(10),
	ISBN VARCHAR(20),
	sDate VARCHAR(20)
);

COPY ventas_temp(invoiceid, userId, ISBN, sDate) FROM '/home/javierd/Escritorio/ventas.txt' WITH ENCODING 'ISO-8859-1' NULL '';

/*=============================================================================
==============================================================================*/
--Remove the headers of each table
DELETE FROM books_temp WHERE  author_name='Autor';
DELETE FROM usuarios_temp WHERE  uid='uid';
DELETE FROM ISBN_Precios_temp WHERE  ISBN='ISBN';

--Standardize some authors' names
UPDATE books_temp SET author_name = 'Anton Rivas Sanchez' WHERE author_name LIKE 'Anton Rivas%';
UPDATE books_temp SET author_name = 'Arturo Perez-Reverte' WHERE 
	author_name LIKE 'Arturo Perez%' OR author_name LIKE 'Arturo Pérez%' OR author_name LIKE 'ARTURO PEREZ%';
UPDATE books_temp SET author_name = 'Felix Lope Vega' 
	WHERE author_name LIKE 'Felix Lope%' OR author_name LIKE 'Félix Lope%'
	OR author_name LIKE 'Lope de Vega%' OR author_name LIKE 'Lope De Vega%'
	OR author_name = 'LOPE DE VEGA' OR author_name LIKE 'Lope Feli%';
UPDATE books_temp SET author_name = 'Miguel de Cervantes Saavedra' 
	WHERE author_name LIKE 'Miguel de Cerv%' OR author_name LIKE 'Miguel De Cerv%' 
	OR author_name='Miguel Cervantes' OR author_name='Miguel CERVANTES SAAVEDRA'
	OR author_name='Mr Miguel de Cervantes' OR author_name='Miqeul de Cervantes Saavedra';


--Standardize some books' info
UPDATE books_temp SET format = 'Libro de bolsillo' WHERE format LIKE 'Libro de %';
UPDATE books_temp SET format = 'Tapa dura' WHERE format='Turtleback';

--Change all the "0000-00-00" dates to a random one. This way, we will not store books which are duplicated on de database one with date and one without it.
UPDATE books_temp SET publication_date='2017-10-31' WHERE publication_date = '0000-00-00';

/*=============================================================================
==============================================================================*/
--Create the definitive tables with its format

--TABLE CREATION

CREATE TABLE IF NOT EXISTS books(
	ID SERIAL PRIMARY KEY,
	title VARCHAR(512) NOT NULL,
	publication_date DATE
);

CREATE TABLE IF NOT EXISTS editions(
	ISBN CHAR(13) PRIMARY KEY,
	book_id INT REFERENCES books(ID),
	editorial_name VARCHAR(256),
	language VARCHAR(20),
	price REAL,
	--In this field we include, for exaple if it is a paperback or hardcover edition etc
	--The different characteristics are stored in upper case and separated by _
	--Using this, we can store multiple characteristics of a product and search them using 
	-- %CHARACTERISTIC%.
	info VARCHAR(1024)
);

CREATE TABLE IF NOT EXISTS authors(
	ID SERIAL PRIMARY KEY,
	name VARCHAR(256)
);

--This table relates authors with their books
CREATE TABLE IF NOT EXISTS relations(
	ID SERIAL PRIMARY KEY,
	book_id INT REFERENCES books(ID) NOT NULL,
	author_id INT REFERENCES authors(ID) NOT NULL
);

CREATE TABLE IF NOT EXISTS discounts(
	ID SERIAL PRIMARY KEY,
	--We use the ISBN so that we don't have to repeat it for each product.
	--If it is NULL, the discount is applied to all the editions
	ISBN CHAR(13) REFERENCES editions(ISBN),
	discount INT NOT NULL,
	start_date DATE,
	end_date DATE
);

CREATE TABLE IF NOT EXISTS users(
	ID INT PRIMARY KEY,
	name VARCHAR(128),
	nick VARCHAR(128),
	card CHAR(19),
	expenses REAL
);

CREATE TABLE IF NOT EXISTS paymentMethods(
	ID INT PRIMARY KEY,
	description VARCHAR(128)
);

--If there is not a user, or a discount, that field is just NULL
CREATE TABLE IF NOT EXISTS sales(
	ID SERIAL PRIMARY KEY,
	invoiceid INT, 
	user_id INT REFERENCES users(ID),
	ISBN CHAR(13) REFERENCES editions(ISBN),
	price REAL,
	discount_id INT REFERENCES discounts(ID),
	payment_method INT REFERENCES paymentMethods(ID),
	sale_date DATE
);


/*=============================================================================
==============================================================================*/
--Insert the data into our tables


--Set up the different payment mehods. In tis case, ccard will be the one with id 0
INSERT INTO paymentMethods VALUES (0, 'Credit Card'), (1, 'Cash'), (2, 'Bitcoin');

INSERT INTO authors(name) 
	SELECT DISTINCT author_name FROM books_temp;

INSERT INTO books(title, publication_date)
	SELECT MIN(book_name), MIN(to_date(publication_date, 'YYYY-MM-DD'))
	FROM books_temp GROUP BY ISBN;


INSERT INTO relations (book_id, author_id) 
	SELECT DISTINCT books.ID, authors.ID 
	FROM (authors JOIN books_temp ON (authors.name = author_name)) JOIN books ON
	(books.title = book_name AND books.publication_date = to_date(books_temp.publication_date, 'YYYY-MM-DD'));


INSERT INTO editions (ISBN, book_id, editorial_name, language, price, info)
	SELECT books_temp.ISBN, MIN(books.ID), MIN(books_temp.editorial), MIN(books_temp.language), 
		MIN(CAST(REPLACE(ISBN_Precios_temp.price, ',', '.') AS REAL)), MIN(books_temp.format) 
	FROM books, books_temp, ISBN_Precios_temp 
	WHERE books.title = books_temp.book_name AND books.publication_date = to_date(books_temp.publication_date, 'YYYY-MM-DD') AND books_temp.ISBN = ISBN_Precios_temp.ISBN GROUP BY books_temp.ISBN; 


--Initialize the users table without the users expenses, we will update them once we have the sales table
INSERT INTO users(ID, name, nick, card)
	SELECT CAST(uid AS BIGINT), name, nick, ccard FROM 
	usuarios_temp;

--First we initialize the sales tables
--As we dont have discounts, we set it to null, and we set all the payment methods to 0 as they are all ccards
INSERT INTO sales(invoiceid, user_id, ISBN, price, discount_id, payment_method, sale_date)
	SELECT CAST(invoiceid AS BIGINT), CAST(userId AS BIGINT), ventas_temp.ISBN,  CAST(REPLACE(price, ',', '.') AS REAL) , NULL, 0, to_date(sDate, 'YYYY-MM-DD') FROM
	ventas_temp JOIN ISBN_Precios_temp ON (ventas_temp.ISBN = ISBN_Precios_temp.ISBN);
	
--Get the expenses of every user and update its rows.
CREATE VIEW expenses AS SELECT user_id, SUM(price) AS total FROM sales GROUP BY user_id;

UPDATE users SET expenses = expenses.total FROM expenses WHERE users.ID = expenses.user_id;


DROP TABLE books_temp CASCADE;
DROP TABLE usuarios_temp CASCADE;
DROP TABLE ISBN_Precios_temp CASCADE;
DROP TABLE ventas_temp CASCADE;
DROP VIEW expenses;
--Javier Delgado del Cerro
--Javier López cano

--LAB 1




--TABLE CREATION

CREATE TABLE books(
	ID SERIAL PRIMARY KEY,
	title VARCHAR(512) NOT NULL,
	publication_date DATE
);

CREATE TABLE editions(
	ISBN CHAR(13) PRIMARY KEY,
	book_id INT REFERENCES books(ID),
	name VARCHAR(512) NOT NULL,
	editor_name VARCHAR(256),
	editorial_name VARCHAR(256),
	--We use the ISO 639-1 code
	language_ISO VARCHAR(2),
	price REAL NOT NULL,
	edition_date DATE,
	--In this field we include, for exaple if it is a paperback or hardcover edition etc
	--The different characteristics are stored in upper case and separated by _
	--Using this, we can store multiple characteristics of a product and search them using 
	-- %CHARACTERISTIC%.
	info VARCHAR(1024)
);

CREATE TABLE authors(
	ID SERIAL PRIMARY KEY,
	name VARCHAR(128),
	surname VARCHAR(256)
);

--This table relates authors with their books
CREATE TABLE relations(
	ID SERIAL PRIMARY KEY,
	book_id INT REFERENCES books(ID) NOT NULL,
	author_id INT REFERENCES authors(ID) NOT NULL
);

CREATE TABLE product(
	ID SERIAL PRIMARY KEY,
	edition_id CHAR(13) REFERENCES editions(ISBN) NOT NULL
);

CREATE TABLE discounts(
	ID SERIAL PRIMARY KEY,
	--We use the ISBN so that we don't have to repeat it for each product.
	--If it is NULL, the discount is applied to all the editions
	ISBN CHAR(13) REFERENCES editions(ISBN),
	discount REAL NOT NULL,
	start_date DATE,
	end_date DATE
);

CREATE TABLE users(
	ID SERIAL PRIMARY KEY,
	name VARCHAR(128),
	card CHAR(19),
	expenses REAL
);

CREATE TABLE paymentMethods(
	ID SERIAL PRIMARY KEY,
	description VARCHAR(128)
);

--If there is not a user, or a discount, that field is just NULL
CREATE TABLE sales(
	ID SERIAL PRIMARY KEY,
	user_id INT REFERENCES users(ID),
	product_id INT REFERENCES product(ID),
	price REAL NOT NULL,
	discount_id INT REFERENCES discounts(ID),
	payment_method INT REFERENCES paymentMethods(ID),
	sale_date DATE NOT NULL
);

--=========================================================
--=========================================================

--QUERTIES

--1 WORKING
	--Take the different ISBNs from a book  and count its editions and different languages
SELECT COUNT(ISBN), COUNT(DISTINCT(language_ISO)) FROM editions WHERE book_id IN (SELECT ID FROM books WHERE title = 'Titulo')

--2 WORKING
	--First, we create a view that contains all the books from an author
	--	This view is used in the following 3 exercises
CREATE VIEW authorBooks AS SELECT books.id FROM (authors JOIN relations ON (authors.id = relations.author_id))
				JOIN books ON (books.id = relations.book_id)
				WHERE authors.name = 'Author3';

	--Take all the sold products of an edition from a book of the given author
SELECT COUNT(sales.id) FROM sales, product, editions WHERE 
	sales.product_id = product.ID AND 
	product.edition_id = editions.ISBN AND
	editions.book_id IN (SELECT * FROM authorBooks);

--3 WORKING
	-- authorBooks is re-used as it was created in the previous querty

	--This is basically the previous query but with the condition that 
	--it has a discount (sales.discount_id!=NULL)

SELECT COUNT(sales.id) FROM sales, product, editions WHERE 
	sales.discount_id IS NOT NULL AND
	sales.product_id = product.ID AND 
	product.edition_id = editions.ISBN AND
	editions.book_id IN (SELECT * FROM authorBooks);

--4 WORKING
	--A For editors
	--Sum the price of all the sales of a product from a given editior
SELECT SUM(sales.price) FROM sales, product, editions WHERE 
	sales.product_id = product.ID AND 
	product.edition_id = editions.ISBN AND editions.editor_name = 'EditorName';

	--A For authors
	-- authorBooks is re-used as it was created in the second querty
	--Sum all the sold products' price of an edition from a book of the given author

SELECT SUM(sales.price) FROM sales, product, editions WHERE 
	sales.product_id = product.ID AND 
	product.edition_id = editions.ISBN AND
	editions.book_id IN (SELECT * FROM authorBooks);

--5 WORKING
	--Count all the sales where the buyer was registered
SELECT COUNT(ID) FROM sales WHERE user_id IS NOT NULL;

--6 WORKING
	--Count the distinct user id from the sold products of English editions
SELECT COUNT(DISTINCT(sales.user_id)) FROM sales, product, editions WHERE 
	sales.user_id IS NOT NULL AND sales.product_id = product.ID AND 
	product.edition_id = editions.ISBN AND editions.language_ISO='EN';

--7 WORKING
	--Sum the price of all sales of Englisgh editions
SELECT SUM(sales.price) FROM sales, product, editions WHERE 
	sales.product_id = product.ID AND 
	product.edition_id = editions.ISBN AND editions.language_ISO = 'FR';

--8 WORKING
	--Get the start and end date from discounts where the publisher
	-- of the edition was Adelpi
SELECT start_date, end_date FROM discounts WHERE 
	discounts.ISBN IN 
	(SELECT ISBN FROM editions WHERE editions.editorial_name = 'Adelpi');

--9 WORKING
	--Get all the registered users except the ones which have ever bought
	-- a paperback book
SELECT users.ID FROM users 
EXCEPT
SELECT sales.user_id FROM sales WHERE sales.product_id IN
	(SELECT product.id FROM editions, product WHERE product.edition_id=editions.isbn 
	AND edition.info LIKE '%PAPERBACK%');
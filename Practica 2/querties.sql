--Javier Delgado del Cerro
--Javier López Cano

--1
	--Take the different ISBNs from a book  and count its editions and different languages
SELECT COUNT(ISBN) as NumEditions, COUNT(DISTINCT(language)) as NumLanguages FROM editions WHERE book_id IN (SELECT ID FROM books WHERE title = 'El Club Dumas');

--2
	--First, we create a view that contains all the books from an author
	--	This view is used in the following 3 exercises
CREATE VIEW authorBooks AS SELECT books.id FROM (authors JOIN relations ON (authors.id = relations.author_id))
				JOIN books ON (books.id = relations.book_id)
				WHERE authors.name = 'Arturo Perez-Reverte';

	--Take all the sold products of an edition from a book of the given author
SELECT COUNT(sales.id) AS soldBooks FROM sales, editions WHERE 
	sales.ISBN = editions.ISBN AND
	editions.book_id IN (SELECT * FROM authorBooks);

--3
	-- authorBooks is re-used as it was created in the previous querty

	--This is basically the previous query but with the condition that 
	--it has a discount (sales.discount_id!=NULL)

SELECT COUNT(sales.id) AS soldBooksDiscount FROM sales, editions WHERE 
	sales.discount_id IS NOT NULL AND
	sales.ISBN = editions.ISBN AND
	editions.book_id IN (SELECT * FROM authorBooks);

--4
	--A For editors
	--Sum the price of all the sales of a product from a given editior
SELECT SUM(sales.price) AS totalSalePrice FROM sales, editions WHERE 
	sales.ISBN = editions.ISBN AND editions.editorial_name = 'Wiley india Pvt. Ltd ';

	--A For authors
	-- authorBooks is re-used as it was created in the second querty
	--Sum all the sold products' price of an edition from a book of the given author

SELECT SUM(sales.price) FROM sales, editions WHERE 
	sales.ISBN = editions.ISBN AND
	editions.book_id IN (SELECT * FROM authorBooks);

--5
	--Count all the sales where the buyer was registered
SELECT COUNT(ID) FROM sales WHERE user_id IS NOT NULL;

--6
	--Count the distinct user id from the sold products of English editions
SELECT COUNT(DISTINCT(sales.user_id)) AS englishBuyerUsers FROM sales, editions WHERE 
	sales.user_id IS NOT NULL AND sales.ISBN = editions.ISBN AND 
	editions.language='Inglés';

--7
	--Sum the price of all sales of Englisgh editions
SELECT SUM(sales.price) AS englishBooksSoldPrice FROM sales, editions WHERE 
	sales.ISBN = editions.ISBN AND editions.language = 'Inglés';

--8
	--Get the start and end date from discounts where the publisher
	-- of the edition was Adelpi
SELECT start_date, end_date FROM discounts WHERE 
	discounts.ISBN IN 
	(SELECT ISBN FROM editions WHERE editions.editorial_name = 'Adelpi');

--9
	--Get all the registered users except the ones which have ever bought
	-- a paperback book
SELECT DISTINCT users.ID FROM users 
EXCEPT
SELECT sales.user_id FROM sales WHERE sales.ISBN IN
	(SELECT editions.ISBN FROM editions WHERE editions.info LIKE '%Libro de bolsillo%');
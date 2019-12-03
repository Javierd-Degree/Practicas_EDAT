import sqlite3
from datetime import date, datetime, timedelta
from random import randint, random, uniform, choice
import string
sql_createTables = [
""" CREATE TABLE books(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
title VARCHAR(512) NOT NULL,
publication_date DATE
);""",
"""
CREATE TABLE editions(
ISBN CHAR(13) PRIMARY KEY,
book_id INTEGER REFERENCES books(ID),
name VARCHAR(512) NOT NULL,
editor_name VARCHAR(256),
language_ISO VARCHAR(2),
price REAL NOT NULL,
edition_date DATE,
info VARCHAR(1024)
);""",
"""
CREATE TABLE authors(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
name VARCHAR(128),
surname VARCHAR(256)
);
""",
"""
CREATE TABLE relations(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
book_id INTEGER REFERENCES books(ID) NOT NULL,
author_id INTEGER REFERENCES authors(ID) NOT NULL
);
""",
"""
CREATE TABLE product(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
edition_id CHAR(13) REFERENCES editions(ISBN) NOT NULL
);
""",
"""
CREATE TABLE discounts(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
ISBN CHAR(13) REFERENCES editions(ISBN),
discount REAL NOT NULL,
start_date DATE,
end_date DATE
);
""",
"""
CREATE TABLE users(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
name VARCHAR(128),
card CHAR(19),
expenses REAL
);
""",
"""
CREATE TABLE sales(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
user_id INTEGER REFERENCES users(ID),
product_id INTEGER REFERENCES product(ID),
price REAL NOT NULL,
discount_id INTEGER REFERENCES discounts(ID),
payment_method INTEGER REFERENCES paymentMethods(ID),
sale_date DATE NOT NULL
);
""",
"""
CREATE TABLE paymentMethods(
ID INTEGER PRIMARY KEY AUTOINCREMENT,
description VARCHAR(128)
);
"""]

languages = ["ES", "EN", "FR", "OS", "AM", "KK"]
editionsISBN = []

def randomString():
	return ''.join(choice(string.ascii_uppercase + string.digits) for _ in range(12))


conn = sqlite3.connect('database.db')
conn.execute("PRAGMA foreign_keys = 1")
c = conn.cursor()

#Create the tables
for i in sql_createTables:
	c.execute(i)

#Set up the different writers
for i in range(0, 10):
	c.execute("INSERT INTO authors VALUES(?, ?, ?)", (i, "Author"+str(i), "Surname"+str(i)))

#Set up the different books
for i in range(0, 20):
	#Generate a random date for each book
	days = randint(1000, 10000)
	cDate = date.today() - timedelta(days)
	c.execute("INSERT INTO books VALUES(?, ?, ?)", (i, "Book"+str(i), cDate.strftime('%d-%m-%Y')))

#Set up some editions for each book
for i in range(0, 20):
	#Generate random language, price, edname and eddate
	#Random string returns 12 char str
	days = randint(0, 1000)
	edDate = date.today() - timedelta(days)
	lang = languages[randint(0, len(languages)-1)] 
	price = uniform(8.5, 54.3)
	edName = randomString()
	info = randomString()
	ISBN = randomString()+str(i)
	#Store the ISBNs in order to set up the discounts later
	editionsISBN.append(ISBN)
	t = (ISBN, i, "Edtion book"+str(i), edName, lang, price, edDate.strftime('%d-%m-%Y'), info) 
	c.execute("INSERT INTO editions VALUES(?, ?, ?, ?, ?, ?, ?, ?)", t)

#Set up products
for i in range(0, 40):
	#Just one product per edition
	c.execute("INSERT INTO product VALUES(?, ?)", (i, editionsISBN[(int)(i/2)]))

#Set up relations
for i in range(0, 20):
	#One random author for each book
	authorID = randint(0, 9)
	c.execute("INSERT INTO relations VALUES(?, ?, ?)", (i, i, authorID))

#Set up payment methods
c.execute("INSERT INTO paymentMethods VALUES(0, 'Card'), (1, 'Cash'), (2, 'Paypal'), (3, 'Bitcoin')")

#Set up users
for i in range(0, 15):
	#Make a long name
	expenses = uniform(45.7, 1415.2)
	name = randomString() + randomString()
	t = (i, name, randomString(), expenses)
	c.execute("INSERT INTO users VALUES (?, ?, ?, ?)", t)

#Set up discounts
for i in range(0, 4):
	ISBN = editionsISBN[randint(0, 20)]
	discounts = uniform(5, 38.5)
	sDays = randint(10, 30)
	start_date = date.today() - timedelta(sDays)
	eDays = randint(-9, 12)
	end_date = date.today() + timedelta(eDays)
	t = (i, ISBN, discounts, start_date.strftime('%d-%m-%Y'), end_date.strftime('%d-%m-%Y'))
	c.execute("INSERT INTO discounts VALUES (?, ?, ?, ?, ?)", t)

#Set up sales
#TODO NULL NOT ALLOWED!!!
for i in range(0, 50):
	#Use some null users and discounts
	user = randint(0, 12)
	if user > 9:
		user = None
	product_id = randint(0, 39)
	price = uniform(8.5, 54.3)
	discount_id = randint(0, 5)
	if discount_id > 3:
		discount_id = None
	paymentMethods = randint(0, 3)
	days = randint(0, 10000)
	sale_date = date.today() + timedelta(days)
	t = (i, user, product_id, price, discount_id, paymentMethods, sale_date.strftime('%d-%m-%Y'))
	c.execute("INSERT INTO sales VALUES (?, ?, ?, ?, ?, ?, ?)", t)

conn.commit()
conn.close()

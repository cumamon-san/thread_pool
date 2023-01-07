#!/bin/python

import sqlite3

with sqlite3.connect("test.db") as db:

    c = db.cursor()

    c.execute("""CREATE TABLE IF NOT EXISTS articles(
        title text NO NULL,
        author text NO NULL,
        score INT
    )""")

    c.execute("""INSERT INTO articles VALUES('Title_1', 'Author_1', 10)""")
    c.execute("""INSERT INTO articles VALUES('Title_2', 'Author_2', 20)""")

    c.execute("UPDATE articles SET title = 'New title' WHERE rowid = 2")

    db.commit()

    for item in c.execute("SELECT rowid, * FROM articles"):
        print("ID = ", item[0], ", ", item[1], sep='')

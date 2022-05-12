# -*- encoding: utf-8 -*-

import os
import sys
import re
import argparse

import sqlite3
from sqlite3 import Error


parser = argparse.ArgumentParser(description='Upload data to server')
parser.add_argument('response_db', type=str, help='Set the response database file')
args = parser.parse_args()


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    print(f'Program: {os.path.basename(os.path.splitext(__file__)[0])}')

    response_db = args.response_db

    if not os.path.exists(response_db):
        print(f'{response_db} NOT exists!')
        sys.exit(-1)

    conn = None
    try:
        conn = sqlite3.connect(response_db)
        print(f'Database: {response_db}')
        print(f'SQLite3: {sqlite3.version}')

    except Error as e:
        print(e)
    finally:
        if conn:
            conn.close()

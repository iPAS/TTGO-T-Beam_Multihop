# -*- encoding: utf-8 -*-

import os
import sys
import re
import argparse

import sqlite3
from sqlite3 import Error
import sqlcmd

import requests
import json


# -----------------------------------------------------------------------------
def get_arguments():
    parser = argparse.ArgumentParser(description='Upload data from log file to server')
    parser.add_argument('--db', type=str, help='Set the response database file',
                        default='extracted_data.sqlite3', required=False)
    parser.add_argument('--url', type=str, help='Request URL to send data',
                        default='http://agritronics.nstda.or.th/webpost/test09.php', required=False)
    return parser.parse_args()


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    print(f'Program: {os.path.basename(os.path.splitext(__file__)[0])}')

    args = get_arguments()
    response_db = args.db
    server_url = args.url

    print(f'Database: {response_db}')
    print(f'SQLite3: {sqlite3.version}')

    if not os.path.exists(response_db):
        print(f'{response_db} NOT exists!')
        sys.exit(-1)

    conn = None
    try:
        conn = sqlite3.connect(response_db)

        ## Read record where yet uploaded & mark it
        cur = conn.cursor()
        cur.execute(sqlcmd.query_select_if_not_uploaded)
        rows = cur.fetchall()

        ## Send each data
        success_count = 0
        for row in rows:
            response_id = row[0]
            data = json.loads(row[1].translate(str.maketrans('\'', '"')))

            try:
                # print(response_id, data)  # DEBUG:
                request_msg = f'{server_url}?data1=1000,{data["date"]},{data["time"]},1'
                for i in range(1, 18):
                    request_msg += f',{data.get(str(i), "0")}'
                resp = requests.get(request_msg)

                print(request_msg)  # DEBUG:
                print(f'>>> {resp}\n')
            except Exception as e:
                print(e)
            else:
                success_count += 1
                cur.execute(sqlcmd.query_update_uploaded_on_id, (response_id,))
        conn.commit()

        ## Remove the uploaded record that older than 30 days
        # TODO:
        conn.commit()


    except Error as e:
        print(e)
    finally:
        if conn:
            conn.close()

    print(f'Uploaded: {success_count}')

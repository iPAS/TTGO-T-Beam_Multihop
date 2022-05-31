# -*- encoding: utf-8 -*-

import os
import sys
import re
import argparse
from datetime import datetime as dt
import csv

import sqlite3
from sqlite3 import Error
import sqlcmd

import requests
import json


# -----------------------------------------------------------------------------
def get_arguments():
    parser = argparse.ArgumentParser(description='Upload data from log file to server')
    parser.add_argument('--db', type=str, help='Set the response database file',
                        default='extracted_data.sqlite3', required=False, metavar='<*.sqlite3>')
    parser.add_argument('--url', type=str, help='Request URL to send data',
                        default='http://agritronics.nstda.or.th/webpost/test09.php', required=False)
    parser.add_argument('--no-send', help='Generate REST URI only, no sending data', metavar='',
                        action='append_const', const=1)
    parser.add_argument('--end-points', help='Target on multiple end-points for testing', metavar='',
                        action='append_const', const=1)
    parser.add_argument('-v', '--verbose', help='Verbosity', metavar='',
                        action='append_const', const=1)
    return parser.parse_args()


# -----------------------------------------------------------------------------
def read_end_points_info(filename):
    # http://agritronics.nstda.or.th/webpost/landlora01.php
    # http://agritronics.nstda.or.th/webpost/landlora02.php
    # http://agritronics.nstda.or.th/webpost/landlora03.php
    # http://agritronics.nstda.or.th/webpost/landlora04.php
    # http://agritronics.nstda.or.th/webpost/landlora05.php
    # http://agritronics.nstda.or.th/webpost/landlora06.php

    with open(filename, 'r') as datfile:
        end_points = { sid: f'http://agritronics.nstda.or.th/webpost/landlora{sno}.php' for
            (sid, name, sno) in csv.reader(datfile, delimiter = ' ', quotechar = '"') }
    return end_points


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    print(f'Program: {os.path.basename(os.path.splitext(__file__)[0])}')

    args = get_arguments()
    response_db = args.db
    server_url = args.url
    verbose_level = 0 if args.verbose is None else len(args.verbose)
    do_send = True if args.no_send is None else False
    do_send_end_points = False if args.end_points is None else True

    print(f'Database: {response_db}')
    print(f'SQLite3: {sqlite3.version}')

    if not os.path.exists(response_db):
        print(f'{response_db} NOT exists!')
        sys.exit(-1)

    if do_send_end_points:
        end_points = read_end_points_info('api_end-points.txt')

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
                if verbose_level > 1:
                    print(response_id, data)  # DEBUG:

                origin_addr = data['origin']
                recv_date = data["date"]
                recv_date = dt.strptime(recv_date, '%Y-%m-%d').date().strftime('%y/%m/%d')  # YYYY-mm-dd
                recv_time = data["time"]

                end_point = server_url
                station_addr = f'{int(origin_addr):02X}'
                if do_send_end_points:
                    end_point = end_points.get(origin_addr, server_url)
                    station_addr = '1000'

                start_io = 1
                end_io = 18
                request_msg = f'{end_point}?data1={station_addr},{recv_date},{recv_time},{start_io}'
                for i in range(start_io, end_io):
                    request_msg += f',{data.get(str(i), "0")}'

                if do_send:
                    resp = requests.get(request_msg)

                if verbose_level > 0:
                    print(f'<<< {request_msg}')  # DEBUG:
                    if do_send:
                        print(f'>>> {resp}\n')
            except Exception as e:
                print(e)
            else:
                if do_send:
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

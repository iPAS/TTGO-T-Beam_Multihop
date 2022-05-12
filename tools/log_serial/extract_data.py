# -*- encoding: utf-8 -*-

import os
import sys
import re
import argparse
import csv

import hashlib
import sqlite3
from sqlite3 import Error


parser = argparse.ArgumentParser(description='Extract data from log file')
parser.add_argument('log_dir', type=str, help='Set the directory of log files')
parser.add_argument('response_db', type=str, help='Set the response database file')
args = parser.parse_args()


# -----------------------------------------------------------------------------
def findall_tags_in_file(path, tag_reg):
    '''
    Extract
    '''
    with open(path, 'r') as f:
        matches = re.findall(tag_reg, f.read(), re.MULTILINE + re.DOTALL)
    return matches


# -----------------------------------------------------------------------------
def cleanse_tags(matches, open_tag):
    '''
    Cleanse
    '''
    checker = re.compile(open_tag, re.MULTILINE + re.DOTALL)
    for i, m in enumerate(matches):  # If multiple opening-tag is found, something is wrong -> discard the match!
        founds = checker.findall(m)
        # print (founds)
        if len(founds) != 1:
            print(f'Tag error at item #{i} >>>')
            print(m + '\n')
            matches.pop(i)
    return matches


# -----------------------------------------------------------------------------
def extract_log(log_dir):
    matches = []
    log_files = sorted(os.listdir(log_dir))
    for log_file in log_files:
        # print(log_file.split('_'))
        matches += findall_tags_in_file(os.path.join(log_dir, log_file), r'\[[0-9:\.\- ]+\] \[D\].*?\[/D\]')

    # print(len(matches))                 # DEBUG: test cleanse_tags()
    # matches[0] = '[D]' + matches[0]     # DEBUG: test cleanse_tags()
    matches = cleanse_tags(matches, r'\[D\]')
    # print(len(matches))                 # DEBUG: test cleanse_tags()
    return matches


# -----------------------------------------------------------------------------
def read_stations_info(filename):
    with open(filename, 'r') as datfile:
        stations = { sid: name for (sid, name) in csv.reader(datfile, delimiter = ' ', quotechar = '"') }
    return stations


# -----------------------------------------------------------------------------
def name_stations(data, stations):
    new_data = []
    re_addr = re.compile(r' @([0-9]+) ')

    for d in data:
        addresses = re_addr.findall(d)
        if addresses is not None:
            for a in addresses:
                name = stations.get(a, None)
                if name is not None:
                    d = re.sub(f' @{a} ', f' @{a}({name}) ', d)
        new_data.append(d)
    return new_data


# -----------------------------------------------------------------------------
def extract_response(resp):
    """Extract the response message to be sent to server.

    >>> extract_response('[2022-05-02 12:27:54.807055 0.001959] 1000 i2c interface on ok')
    >>> extract_response('[2022-05-02 11:35:33.812434 0.003139] 1000 i2c 27.7,65.0')
    {'1': 27.7, '2': 65.0}

    >>> extract_response('[2022-05-02 12:58:27.046442 0.002686] 1000 wind set ok')
    >>> extract_response('[2022-05-02 12:38:37.413748 0.001749] 1000 wind 301,9.7')
    {'4': 301, '5': 9.7}

    >>> extract_response('[2022-05-02 12:38:28.791464 0.003035] 1000 rain set ok')
    >>> extract_response('[2022-05-02 12:38:28.788429 0.003645] 22: $ rain set 0 0.2')
    >>> extract_response('[2022-05-02 12:44:21.620966 0.002682] 1000 rain 3.1')
    {'6': 3.1}

    >>> extract_response('[2022-05-02 12:38:53.709259 0.005014] 44: $ landsld site_name   048')
    >>> extract_response('[2022-05-02 12:38:53.716303 0.007044] 1000 landsld set site name ok')
    >>> extract_response('[2022-05-02 12:52:29.286857 0.000964] 1000 landsld 36.6 0 12.1 0')
    {'7': 36.6}

    >>> extract_response('[2022-05-02 12:27:49.115454 0.003755] 3: $ uc20 start')
    >>> extract_response('[2022-05-02 12:27:49.117865 0.002411] 1000 uc20 start ok')
    >>> extract_response('[2022-05-02 12:52:40.978333 0.005108] 1000 uc20 99,,,99')
    {'8': 99}

    >>> extract_response('[2022-05-02 12:46:56.581830 0.001369] 1000 charger 13.5,54,13.6,0.0,19.7')
    {'10': 13.5}

    >>> extract_response('[2022-05-02 12:44:33.530255 0.002398] 1000 atod 0.131')
    {'16': 0.131}

    >>> extract_response('[2022-05-02 13:22:27.527841 0.002508] 1000 atod 18.286')
    {'17': 18.286}
    """
    m = re.search(r'\[.*?\] +[0-9]+ +(?P<cmd>[0-9a-z]+) +(?P<data>.+) *', resp)
    if m is None:
        return None
    extracted = m.groupdict()
    # print(extracted)  # DEBUG:

    cmd = extracted['cmd']
    data = extracted['data']

    extractors = {
        'i2c': lambda: {
            '1': float(data.split(',')[0]), # Temperature
            '2': float(data.split(',')[1])  # Humidity
        },

        'wind': lambda: {
            '4': int(data.split(',')[0]),   # Wind direction
            '5': float(data.split(',')[1])  # Wind speed
        },

        'rain': lambda: {
            '6': float(data)
        },

        'landsld': lambda: {
            '7': float(data.split(' ')[0])  # Rain in 24 hours
        },

        'uc20': lambda: {
            '8': int(data.split(',')[0])  # GSM quality
        },

        'charger': lambda: {
            '10': float(data.split(',')[0])  # Battery
        },

        'atod': lambda: {
            '16' if float(data) < 5.0 else '17': float(data)
        },

    }

    try:
        return extractors[cmd]()
    except:
        return None


# -----------------------------------------------------------------------------
def extract_data_lines(data):
    pattern_data = re.compile(r'\[[0-9:\.\- ]+\] 1000 [0-9a-z]+ .*')
    pattern_station = re.compile(r'\[[0-9:\.\- ]+\] \[D\] .*')
    pattern_meta = re.compile(r'\[\s*(?P<date>[0-9-]+)\s+(?P<time>[0-9:]+).*\]' +
                              r'\s+' + r'\[D\]' +
                              r'\s+' + r'@(?P<addr>[0-9]+)'     + r'[^0-9 ]*' +
                              r'\s+' + r'recv:(?P<recv>[0-9]+)' +
                              r'\s+' + r'.*' +
                              r'\s+' + r'@(?P<origin>[0-9]+)'   + r'[^0-9 ]*' +
                              r'\s+' + r'#(?P<order>[0-9]+)' +
                              r'\s+' + r'\^(?P<hop>[0-9]+)' +
                              '')
    extracted_data = []

    for d in data:
        ## Data of the packet only
        matches_data = pattern_data.findall(d)

        if matches_data:
            open_line = pattern_station.search(d).group(0)
            meta = pattern_meta.search(open_line).groupdict()

            extracted_infos = {}
            for matched in matches_data:
                filtered = extract_response(matched)
                if filtered is not None:
                    extracted_infos.update(filtered)

            if extracted_infos:
                # print(f'\n>>> {open_line} <<<')  # DEBUG:
                # print(meta)  # DEBUG:
                # print(extracted_infos)  # DEBUG:
                extracted_infos.update(meta)
                extracted_data.append(extracted_infos)

    return extracted_data


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    log_dir = args.log_dir
    response_db = args.response_db

    if not os.path.exists(log_dir):
        print(f'{log_dir} NOT exists!')
        sys.exit(-1)

    ## Data extraction
    data = extract_log(log_dir)  # Extract all log files

    # stations = read_stations_info('stations.txt')  # Read stations' names
    # data = name_stations(data, stations)  # Name the stations

    data = extract_data_lines(data)

    ## Data saving in SQLite
    if data:
        conn = None
        try:
            conn = sqlite3.connect(response_db)
            print(f'Database: {response_db}\nSQLite3: {sqlite3.version}')

            cur = conn.cursor()
            sql = '''
            CREATE TABLE IF NOT EXISTS response (
                response_id INTEGER PRIMARY KEY,
                hash TEXT UNIQUE NOT NULL,
                extracted_data TEXT NOT NULL,
                datetime_created INTEGER NOT NULL,
                uploaded BOOLEAN NOT NULL DEFAULT FALSE
                )
            '''
            cur.execute(sql)
            conn.commit()

            for d in data:
                # hash_value = hash(frozenset(d.items()))  # Never be the same on different runtime
                hash_value = hashlib.md5(str(sorted( d.items() )).encode()).hexdigest()
                # d['hash'] = hash_value
                # d['uploaded'] = False
                # print(f'[{hash_value}]\n', d, '\n')  # DEBUG:

                ## Insert if not exists
                # https://www.geeksforgeeks.org/python-mysql-insert-record-if-not-exists-in-table/
                sql = '''
                INSERT OR IGNORE INTO response(hash, extracted_data, datetime_created)
                VALUES(?, ?, strftime("%s", "now"))
                '''
                cur.execute(sql, (hash_value, str(d)))

                # sql = '''
                # INSERT INTO response(hash, extracted_data)
                # SELECT * FROM (SELECT {h}, "{d}") as val
                # WHERE NOT EXISTS(
                #     SELECT hash
                #     FROM response
                #     WHERE hash = {h}
                #     )
                # '''.format(h = hash_value, d = str(d))
                # cur.execute(sql)
            conn.commit()

        except Error as e:
            print(e)
        finally:
            if conn:
                conn.close()

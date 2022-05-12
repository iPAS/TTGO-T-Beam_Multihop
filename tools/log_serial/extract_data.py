# -*- encoding: utf-8 -*-

import os
import sys
import re
import argparse
import csv


parser = argparse.ArgumentParser(description='Extract data from log file')
parser.add_argument('log_dir', type=str)
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
def analyze_data_and_send_server(data):
    pattern_data = re.compile(r'\[[0-9:\.\- ]+\] 1000 [0-9a-z]+ .*')
    pattern_station = re.compile(r'\[[0-9:\.\- ]+\] \[D\] .*')
    pattern_meta = re.compile(r'\[\s*(?P<date>[0-9-]+)\s+(?P<time>[0-9:]+).*\]' +
                              r'\s+' + r'\[D\]' +
                              r'\s+' + r'@(?P<addr>[0-9]+)'     + r'.*' +
                              r'\s+' + r'recv:(?P<recv>[0-9]+)' +
                              r'\s+' + r'.*' +
                              r'\s+' + r'@(?P<origin>[0-9]+)'   + r'.*' +
                              r'\s+' + r'#(?P<order>[0-9]+)' +
                              r'\s+' + r'\^(?P<hop>[0-9]+)' +
                              '')
    for d in data:
        ## Data packet only
        matches_data = pattern_data.findall(d)
        if matches_data:
            station_info = pattern_station.search(d).group(0)
            print(station_info)

            meta = pattern_meta.search(station_info).groupdict()
            print(meta)

            for m in matches_data:
                print(f'>>> {m} <<<')

            break  # XXX: for debugging


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    log_dir = args.log_dir

    if not os.path.exists(log_dir):
        print(f'{log_dir} NOT exists!')
        sys.exit(-1)

    stations = read_stations_info('stations.txt')  # Read stations' names
    data = extract_log(log_dir)  # Extract all log files

    data = name_stations(data, stations)  # Name the stations

    analyze_data_and_send_server(data)

# -*- encoding: utf-8 -*-

import os
import sys
import re
import argparse


parser = argparse.ArgumentParser(description='Extract data from log file')
parser.add_argument('log_dir', type=str)
args = parser.parse_args()


def findall_tags_in_file(path, tag_reg):
    '''
    Extract
    '''
    with open(path, 'r') as f:
        matches = re.findall(tag_reg, f.read(), re.MULTILINE + re.DOTALL)
    return matches


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


if __name__ == '__main__':
    log_dir = args.log_dir

    if not os.path.exists(log_dir):
        print(f'{log_dir} NOT exists!')
        sys.exit(-1)

    matches = []
    log_files = sorted(os.listdir(log_dir))
    for log_file in log_files:
        # print(log_file.split('_'))
        matches += findall_tags_in_file(os.path.join(log_dir, log_file), r'\[D\].*?\[/D\]')

    # print(len(matches))                 # DEBUG: test cleanse_tags()
    # matches[0] = '[D]' + matches[0]     # DEBUG: test cleanse_tags()
    matches = cleanse_tags(matches, r'\[D\]')
    # print(len(matches))                 # DEBUG: test cleanse_tags()

    for m in matches:
        print(m)

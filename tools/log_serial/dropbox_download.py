# -*- coding: utf-8 -*-
################################################################
# DOWNLOAD ENTIRE FOLDER STRUCTURE FROM DROPBOX TO LOCAL DRIVE #
################################################################

# Instructions:
# (1) install dropbox API using pip
# > pip install dropbox

# (2) Create application to make requests to the Dropbox API
# - Go to: https://dropbox.com/developers/apps
# - Register your own App - e.g., call it "personal access to research data"
# - Copy secret *access token* after registering your app (click on get token)
#   Paste that access token to a file called *token_dropbox.txt*.
#   Make sure you do not version this file on Git, as it would allow others
#   to obtain data from your Dropbox. For example, you can add that file name
#   to .gitignore.

# https://gist.github.com/hannesdatta/10422a6fbb584f245c83361245335741

'''
Created on Wed Mar  4 06:31:48 2020

@author: hdatta

@revised by iPAS
'''
import dropbox
import os
import shutil
import re


# read access token
#ACCESS_TOKEN = open('dropbox_token.conf', 'r').read()
#ACCESS_TOKEN = ACCESS_TOKEN.split('\n')[0]
ACCESS_TOKEN = os.environ['dropbox_token'] 
SYNC_DIR = 'log'
LOG_DIR = os.environ['log_dir']

if ACCESS_TOKEN == '' or SYNC_DIR == '':
    exit(-1)

#------------------------------ get_dropbox.py --------------------------------

# Find folder ID
def get_folders(dbx, folder):
    result = dbx.files_list_folder(folder, recursive=True)

    folders=[]

    def process_dirs(entries):
        for entry in entries:
            if isinstance(entry, dropbox.files.FolderMetadata):
                folders.append(entry.path_lower + '--> ' + entry.id)

    process_dirs(result.entries)

    while result.has_more:
        result = dbx.files_list_folder_continue(result.cursor)
        process_dirs(result.entries)

    return(folders)


def wipe_dir(download_dir):
    # wipe download dir
    try:
        shutil.rmtree(download_dir)
    except:
        1+1


def get_files(dbx, folder_id, download_dir):
    assert(folder_id.startswith('id:'))
    result = dbx.files_list_folder(folder_id, recursive=True)

    # determine highest common directory
    assert(result.entries[0].id==folder_id)
    common_dir = result.entries[0].path_lower

    file_list = []

    def process_entries(entries):
        for entry in entries:
            if isinstance(entry, dropbox.files.FileMetadata):
                file_list.append(entry.path_lower)

    process_entries(result.entries)

    while result.has_more:
        result = dbx.files_list_folder_continue(result.cursor)

        process_entries(result.entries)


    print('Downloading ' + str(len(file_list)) + ' files...')
    i=0
    for fn in file_list:
        i+=1
        printProgressBar(i, len(file_list))

        # print('download to:', remove_suffix(download_dir, '/'), remove_prefix(fn, common_dir))
        # path = remove_suffix(download_dir, '/') + remove_prefix(fn, common_dir)
        path = download_dir + remove_prefix(fn, common_dir)
        print(fn, ' --> ', path)

        # try:
        #     sync_dir = os.path.dirname(os.path.abspath(path))
        #     os.makedirs(sync_dir)
        # except:
        #     1+1

        dbx.files_download_to_file(path, fn)
        # dbx.files_download(fn)


# auxilary function to print iterations progress (from https://stackoverflow.com/questions/3173320/text-progress-bar-in-the-console)
def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 1, length = 100, fill = '█'):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix), end = '\r')
    # Print New Line on Complete
    if iteration == total:
        print()


# inspired by https://stackoverflow.com/questions/16891340/remove-a-prefix-from-a-string and
# https://stackoverflow.com/questions/1038824/how-do-i-remove-a-substring-from-the-end-of-a-string-in-python

def remove_prefix(text, prefix):
    return text[text.startswith(prefix) and len(prefix):]

def remove_suffix(text, suffix):
    return text[:-(text.endswith(suffix) and len(suffix))]

#------------------------------ get_dropbox.py --------------------------------
# from get_dropbox import get_folders, get_files, wipe_dir




#------------------------------------------------------------------------------
# Authenticate with Dropbox
print('Authenticating with Dropbox...')
dbx = dropbox.Dropbox(ACCESS_TOKEN)
print('...authenticated with Dropbox owned by ' + dbx.users_get_current_account().name.display_name)

# (3) Obtain ID of folder that needs to be downloaded
#   folders = get_folders(), which generates a list with ID numbers for each folder
#   in your Dropbox (may take some time!!!)
#   Specifiy a path (if you know that path) for a directory "close" to your target
#   directory. Otherwise, this script will loop through the *entire* file structure
#   of your Dropbox, which will take a lot of time.

folders=get_folders(dbx, '')  # @root

# Let's take a look at these folder IDs
for folder in folders:
    print(folder, type(folder))

# Find SYNC_DIR
for folder in folders:
    result = re.match(r'\/(.+)--> (.+)', folder)
    # print (result[1])
    if result[1] == SYNC_DIR:
        folder_id = result[2]
        break
    else:
        folder_id = None

# Select target folder and copy desired folder ID below
#folder_id = 'id:i34YqK3uj6IAAAAAAAJ3bQ'

if folder_id is None:
    print('folder_id is Null !!')
    exit()

# Set target download directory on your local computer; ends with (e.g., raw_data/)
download_dir = LOG_DIR


##################
# DOWNLOAD FILES #
##################

# obtain list of files of target dir
print('Obtaining list of files in target directory...')
get_files(dbx, folder_id, download_dir)

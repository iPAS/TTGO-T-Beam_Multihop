
## Create if not exists
create_table_if_not_exists = '''
CREATE TABLE IF NOT EXISTS response (
    response_id INTEGER PRIMARY KEY,
    hash TEXT UNIQUE NOT NULL,
    extracted_data TEXT NOT NULL,
    datetime_created INTEGER NOT NULL,
    uploaded BOOLEAN NOT NULL DEFAULT FALSE
    )
'''


## Insert if not exists
# https://www.geeksforgeeks.org/python-mysql-insert-record-if-not-exists-in-table/
insert_if_not_exists = '''
INSERT OR IGNORE INTO response(hash, extracted_data, datetime_created)
VALUES(?, ?, strftime("%s", "now"))
'''


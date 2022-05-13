
## Create if not exists
create_table_if_not_exists = '''
CREATE TABLE IF NOT EXISTS response_table (
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
INSERT OR IGNORE INTO response_table(hash, extracted_data, datetime_created)
VALUES(?, ?, strftime("%s", "now"))
'''


def get_row_count(conn):
    cur = conn.cursor()
    cur.execute('''
    SELECT response_id FROM response_table
    ''')
    return len(cur.fetchall())

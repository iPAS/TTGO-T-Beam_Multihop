
## Create if not exists
query_create_table_if_not_exists = '''
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
query_insert_if_not_exists = '''
INSERT OR IGNORE INTO response_table(hash, extracted_data, datetime_created)
VALUES(?, ?, strftime("%s", "now"))
'''

## Select if not uploaded
query_select_if_not_uploaded = '''
SELECT response_id, extracted_data, uploaded FROM response_table 
WHERE uploaded = FALSE
'''

## Update the upload status
query_update_uploaded_on_id = '''
UPDATE response_table SET uploaded = TRUE WHERE response_id = ?
'''


# -----------------------------------------------------------------------------
def get_row_count(conn):
    cur = conn.cursor()
    cur.execute('''
    SELECT response_id FROM response_table
    ''')
    return len(cur.fetchall())

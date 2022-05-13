stations=`cat stations.txt`

SAVED_IFS=$IFS          # Save current IFS (Internal Field Separator)
IFS=$'\n'               # Change IFS to newline char
stations=($stations)    # split the `names` string into an array by the same name
IFS=$SAVED_IFS          # Restore original IFS


log_dir=log
dropbox_dir=log
response_db=extracted_data.sqlite3

source /home/pi/.virtualenvs/deep/bin/activate

python_version=$(python --version 2>&1 | sed -r 's/.* ([0-9]).*/\1/')
[[ "${python_version}" == "2" ]] && echo "Please use Python 3.x" && exit 255

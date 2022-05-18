echo "[$(date +'%F %T')]"

stations=`cat stations.txt`

SAVED_IFS=$IFS          # Save current IFS (Internal Field Separator)
IFS=$'\n'               # Change IFS to newline char
stations=($stations)    # split the `names` string into an array by the same name
IFS=$SAVED_IFS          # Restore original IFS

here=$(dirname $(readlink -f "$0"))
export log_dir="${here}/log"
dropbox_dir="${here}/log"
dropbox_conf=/home/pi/.dropbox_uploader
# dropbox_token.conf contains the token without newline
export dropbox_token=$(cat "${here}/dropbox_token.conf")
response_db="${here}/extracted_data.sqlite3"

active_venv=/home/pi/.virtualenvs/deep/bin/activate
[[ -f "${active_venv}" ]] && source "${active_venv}"

python_version=$(python --version 2>&1 | sed -r 's/.* ([0-9]).*/\1/')
[[ "${python_version}" == "2" ]] && echo "Please use Python 3.x" && exit 255

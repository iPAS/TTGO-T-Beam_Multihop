stations=`cat stations.txt`

SAVED_IFS=$IFS          # Save current IFS (Internal Field Separator)
IFS=$'\n'               # Change IFS to newline char
stations=($stations)    # split the `names` string into an array by the same name
IFS=$SAVED_IFS          # Restore original IFS


log_dir=log
dropbox_dir=log

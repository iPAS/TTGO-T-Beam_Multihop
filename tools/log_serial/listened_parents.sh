#!/usr/bin/bash

log_dir=log

found_records=$(grep 'Change parent' -R --no-filename "${log_dir}" | sort -t' ' --version-sort -k13 -k1,3)
# echo  $"${found_records}"
# exit

nodes=( $(echo "${found_records}" | cut -d' ' -f13 | uniq | tr '\n' ' ') )
# echo ${nodes[@]}
# exit

echo "Found: ${#nodes[@]}"

for n in ${nodes[@]}; do
    # echo $n
    node_sorted_records=$(echo "${found_records}" | grep "origin $n" | sort -t' ' --version-sort -k1,3)
    # echo "$node_sorted_records"
    # exit

    node_latest_found=$(echo "${node_sorted_records}" | tail -n1)
    echo ${node_latest_found}
done

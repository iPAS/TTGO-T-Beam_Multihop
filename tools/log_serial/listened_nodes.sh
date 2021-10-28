#bin/bash

log_dir=log

found_records=$(grep '\[D\]' -r --no-filename "${log_dir}" | sort -t\  --version-sort -k8,8)

nodes=( $(echo "${found_records}" | cut -d\  -f 8 | uniq | sed 's/@//' | tr '\n' ' ') )

echo "Found: ${#nodes[@]}"

for n in ${nodes[@]}; do
    # echo $n
    node_sorted_records=$(echo "${found_records}" | grep @$n | sort -t\  --version-sort -k1,1 -k2,2 -k3,3)
    # echo "$node_sorted_records"
    node_latest_found=$(echo "${node_sorted_records}" | tail -n 1)
    echo ${node_latest_found}
done

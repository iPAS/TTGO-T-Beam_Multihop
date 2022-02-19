#!/usr/bin/bash

log_dir=log

found_records=$(grep '\[X\] Change parent' -R --no-filename "${log_dir}" | sort -t' ' --version-sort -k13 -k1,3)
# echo  $"${found_records}"
# exit

nodes=( $(echo "${found_records}" | cut -d' ' -f13 | uniq | tr '\n' ' ') )
# echo ${nodes[@]}
# exit

echo "Found: ${#nodes[@]}"

source ./stations.sh

for n in ${nodes[@]}; do
    # echo $n
    node_sorted_records=$(echo "${found_records}" | grep -E "origin @?$n" | sort -t' ' --version-sort -k1,3)
    # echo "$node_sorted_records"
    # exit

    node_latest_found=$(echo "${node_sorted_records}" | tail -n1)

    for station in "${stations[@]}"; do
        station=( $station )
        node_id=${station[0]}
        node_name=${station[1]}
        node_latest_found=$(echo ${node_latest_found} |  \
            sed -E "s/(origin @?${node_id})/\\1(${node_name})/" |  \
            sed -E "s/(to ${node_id})/\\1(${node_name})/")
        #echo "s/@${node_id}/${node_name}/"
    done

    echo ${node_latest_found}
done

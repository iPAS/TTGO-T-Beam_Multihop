#!/usr/bin/bash

found_records=$(grep '\[D\]' -R --no-filename "${log_dir}" | sort -t' ' --version-sort -k8 -k1,3)
#echo "$found_records"
#exit

nodes=( $(echo "${found_records}" | cut -d' ' -f8 | uniq | sed 's/@//' | tr '\n' ' ') )
#echo "${nodes[*]}"

echo "Found: ${#nodes[@]}"
#exit

source ./stations.sh
#for station in "${stations[@]}"; do
#	station=( $station )
#	echo ${station[0]}, ${station[1]}
#done
#exit

for n in ${nodes[@]}; do
    node_sorted_records=$(echo "${found_records}" | grep "@$n" | sort -t' ' --version-sort -k1,3)
    # echo "$node_sorted_records"
    node_latest_found=$(echo "${node_sorted_records}" | tail -n1)

	for station in "${stations[@]}"; do
		station=( $station )
		node_id=${station[0]}
		node_name=${station[1]}
		node_latest_found=$(echo ${node_latest_found} | sed -E "s/(@${node_id})/\\1(${node_name})/")
		#echo "s/@${node_id}/${node_name}/"
	done

    echo ${node_latest_found}
done

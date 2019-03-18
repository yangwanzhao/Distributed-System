#!/bin/bash

identity_file="~/Desktop/CSE403/AWS/AWS_test.pem.txt"

line=$(cat DHTConfig | awk 'NR==1')
IPstr=${line#*=}
IPaddr=(${IPstr//,/ })  # () means convert to a list
for node in ${IPaddr[@]}; do
	echo $node
	scp -i $identity_file *.* DHTConfig ubuntu@$node:~/Distributed-System
	# scp -i $identity_file client.cc ubuntu@$node:~/Distributed-System
	# scp -i $identity_file server.cc ubuntu@$node:~/Distributed-System
	# scp -i $identity_file hashlist.cc ubuntu@$node:~/Distributed-System
done


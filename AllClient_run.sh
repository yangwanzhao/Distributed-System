#!/bin/bash

nodes="nodes.txt"
identity_file="~/Desktop/CSE403/AWS/AWS_test.pem.txt"
counter=0
for server in $(cat $nodes); do
    ssh -i $identity_file ubuntu@$server "cd Distributed-System; ./client -t -c 10000 -l 5; exit $1 $2" > outputs/client$counter.txt &
    echo "Command sent $server"
    counter=$((counter+1))
done
echo "Run on all nodes"

#!/bin/bash
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <server_ip> <port> <filename>"
    echo "Example: $0 127.0.0.1 12345 test.txt"
    exit 1
fi

SERVER_IP="$1"
PORT="$2"
FILE="$3"

NUM_CALLS=10

if [ ! -f "$FILE" ]; then
    echo "Error: File '$FILE' not found"
    exit 1
fi

echo "Starting $NUM_CALLS client calls..."
for ((i=1; i<=NUM_CALLS; i++)); do
    ../build/client "$SERVER_IP" "$PORT" "$FILE" &
    echo "Started client call $i"
    sleep 1
done

wait

echo "All client calls completed!"
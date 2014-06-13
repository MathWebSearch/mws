#!/bin/sh -e

HOST="localhost"
PORT="9200"

if [ $# -lt 1 ]; then
cat << EOF
Expected at least 1 bulk file
Usage:
    $0 <bulk_file> ... <bulk_file>
EOF
    exit 1
fi

while [ $# -gt 0 ]; do
    curl -s -XPOST $HOST:$PORT/_bulk?index=tema\&type=doc --data-binary @$1
    echo
    shift
done

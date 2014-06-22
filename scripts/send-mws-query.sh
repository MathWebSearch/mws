#!/bin/sh -e

if [ "$#" -ne 4 ]; then
    cat << EOF
Usage: $0 <port> <from> <size> <query>
Example:
> $0 9090 0 5 "<mws:qvar/>"

This script sends a query to a MWS rest interface running locally.
If no query is provided, the default is <mws:qvar/> which should
return all indexed expressions (up to a limit).

EOF
    exit 1
fi

PORT="$1"
HOST="http://localhost:$PORT"
LIMITMIN="$2"
LIMITSIZE="$3"
QUERY="$4"

curl -s -S "http://localhost:$PORT" -d "
<mws:query
  xmlns:mws=\"http://www.mathweb.org/mws/ns\"
  xmlns:m=\"http://www.w3.org/1998/Math/MathML\"
  limitmin=\"$LIMITMIN\"
  answsize=\"$LIMITSIZE\"
  totalreq=\"yes\"
  >
  <mws:expr>
    $QUERY
  </mws:expr>
</mws:query>
"

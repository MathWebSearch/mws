#!/bin/sh -e

if [ "$#" -ne 3 ]; then
    cat << EOF
Usage: $0 <restport> <limitmin> <limitsize>
Example:
> $0 9090 0 5

This script sends a qvar query to a MWS rest interface running locally.
This query should return all indexed data, since <mws:qvar/> matches all
expressions. The total presented in the returned mws:answset should be
the total number of expressions indexed when mwsd was started.

EOF
    exit 1
fi

PORT="$1"
HOST="http://localhost:$PORT"
LIMITMIN="$2"
LIMITSIZE="$3"


curl "http://localhost:$PORT" -d "
<mws:query
  xmlns:mws=\"http://www.mathweb.org/mws/ns\"
  xmlns:m=\"http://www.w3.org/1998/Math/MathML\"
  limitmin=\"$LIMITMIN\"
  answsize=\"$LIMITSIZE\"
  totalreq=\"yes\"
  >
  <mws:expr>
    <mws:qvar>x</mws:qvar>
  </mws:expr>
</mws:query>
"

#!/bin/bash -e

#
# Copyright (C) 2010-2013 KWARC Group <kwarc.info>
#
# This file is part of MathWebSearch.
#
# MathWebSearch is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# MathWebSearch is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.
#

# Author: Corneliu Prodescu <cprodescu@gmail.com>
#
# Script used to deploy MathWebSearch
#
# - generate /etc/init.d/mwsd_*
#

print_usage() {
  cat << EOF

Usage: $0 <mwsd.conf>

EOF
}

# Reading config

if [ $# -ne 1 ]; then
    print_usage
    exit 1
fi

CONFIG_FILE="$1"
CONFIG_DIR="$(dirname $CONFIG_FILE)"


[ -r "$CONFIG_FILE" ] || {
    echo "Error: Config file \"$CONFIG_FILE\" not readable"
    exit 1
}

. $CONFIG_FILE

path_error() {
  echo "$1 is invalid"
  exit 1
}

get_path() {
    path="$1"

    dirname="$(cd "$CONFIG_DIR" && cd $(dirname "$path") && pwd -P)"
    basename="$(basename "$path")"

    canonical_path="$dirname/$basename"
    if [ -r "$canonical_path" ]; then
        echo "$canonical_path"
        return 0
    else
        return 1
    fi
}

# Process paths (make canonical and relative to config file directory)
MWS_DATA_PATH="$(get_path "$MWS_DATA_PATH")" || path_error "MWS_DATA_PATH"
MWS_HARVEST_PATH="$(get_path "$MWS_HARVEST_PATH")" || path_error "MWS_HARVEST_PATH"
MWS_BIN_PATH="$(get_path "$MWS_BIN_PATH")" || path_error "MWS_BIN_PATH"

# Generate files from stubs
STUBS_DIR="$(dirname $0)"
OUTPUT_DIR="/etc/init.d"

MWSD_STUB="$STUBS_DIR/mwsd.init.in"

MWSD_OUT="$OUTPUT_DIR/mwsd_$MWS_DEPLOY_NAME"

cat $MWSD_STUB | \
    sed "s#@MWS_DEPLOY_NAME@#$MWS_DEPLOY_NAME#g" | \
    sed "s#@MWS_PORT@#$MWS_PORT#g" | \
    sed "s#@MWS_REST_PORT@#$MWS_REST_PORT#g" | \
    sed "s#@MWS_DATA_PATH@#$MWS_DATA_PATH#g" | \
    sed "s#@MWS_BIN_PATH@#$MWS_BIN_PATH#g" | \
    sed "s#@MWS_DAEMON_EXTRA_ARGS@#$MWS_DAEMON_EXTRA_ARGS#g" | \
    sed "s#@MWS_HARVEST_PATH@#$MWS_HARVEST_PATH#g" > "$MWSD_OUT"
chmod 755 $MWSD_OUT
echo "Generated $MWSD_OUT"
update-rc.d -f mwsd_$MWS_DEPLOY_NAME remove
update-rc.d mwsd_$MWS_DEPLOY_NAME defaults
echo "Registered mwsd_$MWS_DEPLOY_NAME with SysV init"

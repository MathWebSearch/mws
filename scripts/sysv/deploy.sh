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
# - generate /etc/init.d/mwsd_* and /etc/init.d/restd_*
#

print_usage() {
  cat << EOF

Usage: $0 <mwsd.conf>

EOF
}

path_error() {
  echo "$1 is invalid"
  exit 1
}

# Reading config

if [ $# -ne 1 ]; then
    print_usage
    exit 1
fi

CONFIG_FILE="$1"

[ -r "$CONFIG_FILE" ] || {
    echo "Error: Config file \"$CONFIG_FILE\" not readable"
    exit 1
}

# Include config
. `readlink -f $CONFIG_FILE`

# Process paths (make canonical and relative to config file directory)
MWS_DATA_PATH="$(cd $(dirname "$CONFIG_FILE") && readlink -f "$MWS_DATA_PATH")" || path_error "MWS_DATA_PATH"
MWS_HARVEST_PATH="$(cd $(dirname "$CONFIG_FILE") && readlink -f "$MWS_HARVEST_PATH")" || path_error "MWS_HARVEST_PATH"
MWS_BIN_PATH="$(cd $(dirname "$CONFIG_FILE") && readlink -f "$MWS_BIN_PATH")" || path_error "MWS_BIN_PATH"

# Generate files from stubs
STUBS_DIR="$(dirname $0)"
#OUTPUT_DIR="$(dirname $0)"
OUTPUT_DIR="/etc/init.d"

MWSD_STUB="$STUBS_DIR/mwsd.init.in"
RESTD_STUB="$STUBS_DIR/restd.init.in"

MWSD_OUT="$OUTPUT_DIR/mwsd_$MWS_DEPLOY_NAME"
RESTD_OUT="$OUTPUT_DIR/restd_$MWS_DEPLOY_NAME"

cat $MWSD_STUB |    \
    sed "s#@MWS_DEPLOY_NAME@#$MWS_DEPLOY_NAME#g" | \
    sed "s#@MWS_PORT@#$MWS_PORT#g" | \
    sed "s#@MWS_REST_PORT@#$MWS_REST_PORT#g" | \
    sed "s#@MWS_DATA_PATH@#$MWS_DATA_PATH#g" | \
    sed "s#@MWS_BIN_PATH@#$MWS_BIN_PATH#g" | \
    sed "s#@MWS_HARVEST_PATH@#$MWS_HARVEST_PATH#g" > "$MWSD_OUT"
chmod 755 $MWSD_OUT
echo "Generated $MWSD_OUT"
update-rc.d -f mwsd_$MWS_DEPLOY_NAME remove
update-rc.d mwsd_$MWS_DEPLOY_NAME defaults
echo "Registered with Sysvinit"

cat $RESTD_STUB |    \
    sed "s#@MWS_DEPLOY_NAME@#$MWS_DEPLOY_NAME#g" | \
    sed "s#@MWS_PORT@#$MWS_PORT#g" | \
    sed "s#@MWS_REST_PORT@#$MWS_REST_PORT#g" | \
    sed "s#@MWS_DATA_PATH@#$MWS_DATA_PATH#g" | \
    sed "s#@MWS_BIN_PATH@#$MWS_BIN_PATH#g" | \
    sed "s#@MWS_HARVEST_PATH@#$MWS_HARVEST_PATH#g" > "$RESTD_OUT"

chmod 755 $RESTD_OUT
echo "Generated $RESTD_OUT"
update-rc.d -f restd_$MWS_DEPLOY_NAME remove
update-rc.d restd_$MWS_DEPLOY_NAME defaults
echo "Registered with Sysvinit"

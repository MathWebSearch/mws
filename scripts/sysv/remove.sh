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
# Script used to remove MathWebSearch SysV integration
#
# - remove /etc/init.d/mwsd_*
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

[ -r "$CONFIG_FILE" ] || {
    echo "Error: Config file \"$CONFIG_FILE\" not readable"
    exit 1
}

. "$CONFIG_FILE"

# Note that this does not stop mws if running
rm -f /etc/init.d/mwsd_$MWS_DEPLOY_NAME
update-rc.d -f mwsd_$MWS_DEPLOY_NAME remove

# Older versions used a RESTful interface along with mwsd.
# This ensures that is removed as well.
rm -f /etc/init.d/restd_$MWS_DEPLOY_NAME
update-rc.d -f restd_$MWS_DEPLOY_NAME remove

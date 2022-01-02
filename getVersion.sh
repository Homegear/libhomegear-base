#!/bin/sh

SCRIPTDIR="$( cd "$(dirname $0)" && pwd )"

REVISION_FILE=$SCRIPTDIR/revision.txt
REVISION=$(cat $REVISION_FILE)
echo "0.8.$(date '+%Y%m%d%H%M%S')-${REVISION}"

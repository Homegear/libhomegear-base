#!/bin/sh

SCRIPTDIR="$( cd "$(dirname $0)" && pwd )"

REVISION_FILE=$SCRIPTDIR/revision.txt
REVISION=$(cat $REVISION_FILE)
echo "0.8.0-$REVISION.$(date '+%Y%m%d%H%M%S')"

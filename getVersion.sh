#!/bin/sh

SCRIPTDIR="$( cd "$(dirname $0)" && pwd )"

REVISION_FILE=$SCRIPTDIR/revision.txt
REVISION=$(cat $REVISION_FILE)
echo 0.7.0-$REVISION

#!/bin/sh
revision=0
wgetCount=0
while [ $revision -eq 0 ] && [ $wgetCount -le 5 ]; do
	rm -f contributors*
	wget -q https://api.github.com/repos/Homegear/Homegear/stats/contributors
	lines=`grep -Po '"total":.*[0-9]' contributors`
	for l in $lines; do
		    if [ "$l" != "\"total\":" ]; then
		            revision=$(($revision + $l))
		    fi
	done
	wgetCount=$(($wgetCount + 1))
done
echo 0.6.0-$revision

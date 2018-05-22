#!/bin/sh

revision1=0
wgetCount=0
while [ $revision1 -eq 0 ] && [ $wgetCount -le 5 ]; do
	rm -f contributors*
	wget -q https://api.github.com/repos/Homegear/Homegear/stats/contributors
	lines=`grep -Po '"total":.*[0-9]' contributors`
	for l in $lines; do
		    if [ "$l" != "\"total\":" ]; then
		            revision1=$(($revision1 + $l))
		    fi
	done
	wgetCount=$(($wgetCount + 1))
	sleep 1
done
rm -f contributors*

revision2=0
wgetCount=0
while [ $revision2 -eq 0 ] && [ $wgetCount -le 5 ]; do
	rm -f contributors*
	wget -q https://api.github.com/repos/Homegear/libhomegear-base/stats/contributors
	lines=`grep -Po '"total":.*[0-9]' contributors`
	for l in $lines; do
		    if [ "$l" != "\"total\":" ]; then
		            revision2=$(($revision2 + $l))
		    fi
	done
	wgetCount=$(($wgetCount + 1))
	sleep 1
done
rm -f contributors*

revision=$(($revision1 + $revision2))
echo 0.7.18-$revision

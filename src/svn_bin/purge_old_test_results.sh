#!/bin/sh

#
# This script is designed to be runable at any time and
# clear out old test results to achieve a desired state
# of kept and removed results...
#    - all results going back from 0...30 days
#    - one result per week 30...180 days (0.5 years)
#    - one result per month 180...910 days (3 years)
#    - no results older than 3 years
#

root=/project/projectdirs/visit/www

#
# Function to find dirs in a given range, count them
# and if the count is greater than 1, remove all but one.
#
purge_range () {
    local min=$1
    local max=$2
    local dir=$3
    local dirs_in_range=$(find $dir -maxdepth 1 -type d -mtime +$min -mtime -$max)
    dir_count=$(echo $dirs_in_range | tr ' ' '\n' | wc -l)
    if [[ $dir_count -gt 1 ]]; then
        dirs_to_remove=$(echo $dirs_in_range | cut -d' ' -f2-)
        if [[ -n "$dirs_to_remove" ]]; then
            removals=$(expr $removals + $dir_count - 1)
            rm -rf $dirs_to_remove
        fi
    fi
}

#
# Loop to purge ranges of length 7 days (1 week)
# starting at 30 days through one half year back (180 days)
#
removals=0
i=30
len=7
while [[ $i -lt 180 ]]; do
   j=$(expr $i + $len)
   purge_range $i $j $root 
   i=$j
done 

#
# Loop to purge ranges of length 30 days (1 month)
# starting at 180 days though 2.5 years back
# (910 days)
#
i=180
len=30
while [[ $i -lt 910 ]]; do
   j=$(expr $i + $len)
   purge_range $i $j $root 
   i=$j
done 

#
# clear out anything older than 910 days
#
purge_range 910 9999 $root

if [[ $removals -gt 0 ]]; then
    echo "Removed $removals VisIt test results"
fi

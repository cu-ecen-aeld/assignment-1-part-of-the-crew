#!/bin/sh
# Author: Nikolai Dunichev 

set -e
#set -u

if [ ! -n "$1" ];
then
	echo "No the first argument(filesdir)"
	exit 1
fi

if [ ! -n "$2" ];
then
	echo "No the second argument(searchstr)"
	exit 1
fi

result_num_files=$(grep -lr "$2" "$1" | wc -w)

if [ "0" = "$result_num_files" ];
then
	echo "There are no files meeting the specified conditions"
	exit 1
fi

for i in $(grep -lr "$2" "$1"); do
    #echo $i
    result_num_lines=$(( $result_num_lines+$(grep -c "$2" "$i") ))
done

echo "The number of files are" $result_num_files "and the number of matching lines are" $result_num_lines


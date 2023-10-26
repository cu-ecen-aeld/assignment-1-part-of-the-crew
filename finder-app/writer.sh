#!/bin/sh
# Author: Nikolai Dunichev 

set -e
#set -u

if [ ! -n "$1" ];
then
	echo "No the first argument(writefile)"
	exit 1
fi

if [ ! -n "$2" ];
then
	echo "No the second argument(writestr)"
	exit 1
fi
name=$(echo "$1" | sed -r "s/(.+)\/.+/\1/")

#echo ${name} 
mkdir -p ${name} 
echo "$2" >> "$1"


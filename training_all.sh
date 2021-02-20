#!/bin/sh
for i in $(cat $1 | awk '{print $1}') ; do
	./fintrack -n $i &
done

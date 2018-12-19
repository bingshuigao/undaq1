#!/bin/sh

for f in `ls ../inc/*.h`
do
	ln -s "$f" .
done

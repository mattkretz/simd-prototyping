#!/bin/sh
read x <$1
x=$((x+1))
echo $x >$1
echo $x

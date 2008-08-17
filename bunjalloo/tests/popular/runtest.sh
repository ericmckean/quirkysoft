#!/bin/bash
cd $(dirname $0)

BUNJALLOO=../_build_/sdl/bunjalloo/bunjalloo
while read line
do
  cd ../.. >/dev/null
  $BUNJALLOO $line &
  sleep 10
  printf "5 seconds to kill... "
  sleep 5
  pkill bunjalloo
  printf "killed\n"
  cd - >/dev/null
done < input.txt

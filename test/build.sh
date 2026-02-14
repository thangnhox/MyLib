#!/bin/bash

src="$1"
base="$(basename "$src" .cpp)"

g++ -ggdb -std=c++23 "$src" -L../bin/lib -ltnclib -I../include -o "$base.exe"

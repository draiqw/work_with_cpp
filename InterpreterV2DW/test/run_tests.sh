#!/bin/bash
make clean
make
for prog in data/*; do
    echo "Running $prog"
    ./bin/interpreter "$prog"
    echo ""
done

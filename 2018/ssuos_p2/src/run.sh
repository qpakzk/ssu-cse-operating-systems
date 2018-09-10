#!/bin/bash
SHDIR=$(dirname $0)

pushd $SHDIR
make
make run
popd
reset

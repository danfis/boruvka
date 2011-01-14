#!/bin/bash


PYTHON=python
PYTHON2=python2
TESTS="vec2.py vec3.py vec4.py"

if [ "$1" != "" ]; then
    TESTS="$1"
fi;

for t in $TESTS; do
    echo -e "\033[0;33m    $t \033[0;0m"
    PYTHONPATH=.. $PYTHON "$t";
done;

$PYTHON2 ../../testsuites/cu/check-regressions regressions

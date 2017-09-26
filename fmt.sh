#!/bin/bash

MODULE="$1"
MODULE_CAP="$2"
TYPE="$3"
PREFIX="$4"

MODULE_UP=$(echo "$MODULE" | tr '[:lower:]' '[:upper:]')
PREFIX_UP=$(echo "$PREFIX" | tr '[:lower:]' '[:upper:]')

sed "s/TYPE/${TYPE}/g" \
    | sed "s/bor_${MODULE}/bor_${PREFIX}${MODULE}/g" \
    | sed "s/bor${MODULE_CAP}/bor${PREFIX_UP}${MODULE_CAP}/g" \
    | sed "s/BOR_${MODULE_UP}/BOR_${PREFIX_UP}${MODULE_UP}/g" \
    | sed "s/boruvka\/_${MODULE}\.h/boruvka\/${PREFIX}${MODULE}.h/g" \
    | sed "s/boruvka\/${MODULE}\.h/boruvka\/${PREFIX}${MODULE}.h/g"

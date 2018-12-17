#!/bin/bash

./test $@
TEST_RET=$?

./cu/cu-check-regressions --no-progress reg
REG_RET=$?

exit $(($TEST_RET + $REG_RET))

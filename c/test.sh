#!/bin/bash
set -o nounset
set -o errexit

for f in tests/*.conf
do
    f1=tests/$(basename $f .conf).txt
    EXPECT=$(cat $f1)
    OUTPUT=$(./lexer_test $f)
    if [[ "$EXPECT" == "$OUTPUT" ]]; then
        echo -e "\e[32mPASS\e[0m"
    else
        echo -e "Expected: \"\e[33m$EXPECT\e[0m\""
        echo -e "Got:      \"\e[33m$OUTPUT\e[0m\""
        echo -e "\e[31mFAIL\e[0m"
        exit -1
    fi
done

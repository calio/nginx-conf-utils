#!/bin/bash
set -o nounset
set -o errexit

for f in tests/*.conf
do
    name=$(basename $f .conf)
    f1=tests/$name.txt
    if [[ ! -e $f1 ]]; then
        echo "Can't find file \"$f1\""
        exit 1
    fi

    EXPECT=$(cat $f1)
    OUTPUT=$(./lexer_test $f)
    if [[ "$EXPECT" == "$OUTPUT" ]]; then
        echo -e "\e[32mPASS   $name\e[0m"
    else
        echo -e "Expected: \"\e[33m$EXPECT\e[0m\""
        echo -e "Got:      \"\e[33m$OUTPUT\e[0m\""
        echo -e "\e[31mFAIL   $name\e[0m"
        exit 1
    fi
done

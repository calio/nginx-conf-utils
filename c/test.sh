#!/bin/bash
set -o nounset
set -o errexit

check() {
    name=$1
    EXPECT=$2
    OUTPUT=$3

    if [[ "$EXPECT" == "$OUTPUT" ]]; then
        echo -e "\t\t\e[32mPASS\e[0m"
    else
        echo -e "\t\t\e[31mFAIL\e[0m"
        echo -e "Expected: \"\e[33m$EXPECT\e[0m\""
        echo -e "Got:      \"\e[33m$OUTPUT\e[0m\""
        #exit 1
    fi
}

echo "Lexer tests:"
LEXER_TESTS=tests/lexer
for f in $LEXER_TESTS/*.conf
do
    name=$(basename $f .conf)
    f1=$LEXER_TESTS/$name.txt
    if [[ ! -e $f1 ]]; then
        echo "Can't find file \"$f1\""
        exit 1
    fi

    echo -n "$name"

    EXPECT=$(cat $f1)
    OUTPUT=$(./lexer_test $f)

    check $name "$EXPECT" "$OUTPUT"
done

echo
echo "Parser tests:"
PARSER_TESTS=tests/parser
for f in $PARSER_TESTS/*.conf
do
    name=$(basename $f .conf)
    f1=$PARSER_TESTS/$name.txt
    if [[ ! -e $f1 ]]; then
        echo "Can't find file \"$f1\""
        exit 1
    fi

    echo -n "$name"

    EXPECT=$(cat $f1)
    OUTPUT=$(./parser_test $f|jq .)

    check $name "$EXPECT" "$OUTPUT"
done

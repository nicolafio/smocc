#!/bin/bash

sources_dir=./src
executable=out/smocc
process=0

get_sources_hash() {
    hash=""
    sources="Makefile $(find $sources_dir -type f)"
    for file in $sources
    do
        hash="$hash$(md5 -q $file)"
    done
    echo $hash
}

wait_for_change() {
    hash=$(get_sources_hash)
    while [ "$hash" == "$(get_sources_hash)" ]
    do sleep .1
    done
}

while true
do
    clear
    make clean
    make $executable

    if [ $process -ne 0 ]
    then kill $process
    fi

    $executable &
    process=$!

    wait_for_change
done

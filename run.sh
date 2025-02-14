#!/bin/bash

BUILD_DIR=build
COUNT_THREAD=-j$(nproc)

BUILD_FLAGS=""

FILE_CONFIG=../configs/config.yaml
PROGRAM=program
DEBUG_BUILD=""

mkdir -p $BUILD_DIR
cd $BUILD_DIR

while getopts "dcml:r:" OPTION; do
    case $OPTION in
    d)
        BUILD_FLAGS=-DCMAKE_BUILD_TYPE=Debug 
        echo ./$PROGRAM $TARGET $FILE_CONFIG
        DEBUG_BUILD="VERBOSE=1"
    ;;
	c)
        cmake $BUILD_FLAGS ..
    ;;
    m)
        rm ./$PROGRAM

        make $DEBUG_BUILD
        # make $COUNT_THREAD
    ;;
    l)
        FILE_CONFIG=${OPTARG}
    ;;
	r)
        TARGET=${OPTARG}
        if [[ $TARGET == "bss" ]]; then
            echo "BSS: set config"
            FILE_CONFIG=../configs/config_bss.yaml
        fi
        if [[ $TARGET == "ue" ]]; then
            echo "UE: set config"
            FILE_CONFIG=../configs/config_ue.yaml
        fi
        if [[ $TARGET == "monitor" ]]; then
            echo "MONITOR: set config"
            FILE_CONFIG=../configs/config_monitor.yaml
        fi
        if [[ $TARGET == "receiver" ]]; then
            echo "receiver: set config"
            FILE_CONFIG=../configs/config_recv.yaml
        fi
        echo ./$PROGRAM $TARGET $FILE_CONFIG
        ./$PROGRAM $TARGET $FILE_CONFIG
        exit;
	;;
	*)
		echo "Incorrect option"
	;;
	esac
done

if [[ $OPTIND == 1 ]]; then

    echo "Commands:"
    echo "d - DEBUG"
    echo "c - cmake (rebuild makefile)"
    echo "l - file config path"
    echo "m - make"
    echo "r - run project <target>"
fi






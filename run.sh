#!/bin/bash


BUILD_DIR=build
COUNT_THREAD=-j16

#DEBUG_BUILD=-D

BUILD_FLAGS=""

mkdir -p $BUILD_DIR
cd $BUILD_DIR

ADDRESS_DEV="ip:192.168.3.1"
SPECIFIC_OPTION="4"

while getopts "dcma:r:vo:" OPTION; do
    case $OPTION in
    d)
        BUILD_FLAGS=-DCMAKE_BUILD_TYPE=Debug 
        #echo $BUILD_FLAGS
    ;;
	c)
        #echo $BUILD_FLAGS
        cmake $BUILD_FLAGS ..
    ;;
    m)
        cd $BUILD_DIR
        make $COUNT_THREAD
        # exit;
    ;;
    a)
        ADDRESS_DEV=${OPTARG}
    ;;
    o)
        SPECIFIC_OPTION=${OPTARG}
    ;;
	r)
        TARGET=${OPTARG}
        ./program $ADDRESS_DEV ../data/data_test.txt $TARGET $SPECIFIC_OPTION
	;;
	v) 
        python3 ../visual_signal.py
	;;
	*)
		echo "Incorrect option"
	;;
	esac
done

if [[ $OPTIND == 1 ]]; then

    echo "Not found command, -dcmarv"
    echo "d - DEBUG"
    echo "c - cmake (rebuild makefile)"
    echo "m - make"
    echo "a - address device, example <ip:192.168.3.1>"
    echo "r - run project"
    echo "v - view result project"
    echo "o - specific option"

    #echo "Example: ./run.sh -cm -a 192.168.3.1 -o 4 -r model_practice"
fi


exit;




if [[ $1 == "test" ]]; then
    g++ ./fast_test/test1.cpp -o ./fast_test/test1
    ./fast_test/test1


    exit;
fi






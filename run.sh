#!/bin/bash


BUILD_DIR=build
COUNT_THREAD=-j16

#DEBUG_BUILD=-D

BUILD_FLAGS=""


cd $BUILD_DIR



while getopts "mpvcd" OPTION; do
    case $OPTION in
    d)
        BUILD_FLAGS=-DCMAKE_BUILD_TYPE=Debug 
        #echo $BUILD_FLAGS

    ;;
	c)
        mkdir -p $BUILD_DIR
        
        #echo $BUILD_FLAGS
        cmake $BUILD_FLAGS ..
        
    ;;
    m)
        # cd $BUILD_DIR
        make $COUNT_THREAD
        # exit;
    ;;
	p)
        ./program
	;;
	v) 
        python3 visual_signal.py
	;;
	*)
		echo "Incorrect option"
	;;
	esac
done

if [[ $OPTIND == 1 ]]; then

    echo "Not found command, -mpv"
fi


exit;




if [[ $1 == "test" ]]; then
    g++ ./fast_test/test1.cpp -o ./fast_test/test1
    ./fast_test/test1


    exit;
fi






#! /bin/bash

check () {
	echo "Checking $1"
	val=0
	if [ $2 = true ]
	then val=1
	else val=0
	fi
	"./$3" -q < "$1"
	if [ $? -eq $val ]
	then echo "$1: as expected"
	else echo "$1: * test failed!!!!!!!!!!!!!!!!! *"
	fi
	echo "---"
}

check data/buttazzo-1.txt true feasible
#check data/buttazzo-1.txt true onlfeas # slow
check data/buttazzo-1.txt true gfp
check data/buttazzo-1.txt true gedf

check data/buttazzo-2.txt false feasible
#check data/buttazzo-2.txt false onlfeas # slow
check data/buttazzo-2.txt false gfp
check data/buttazzo-2.txt false gedf
check data/buttazzo-2.txt false llf

check data/buttazzo-3.txt true gfp

check data/buttazzo-4.txt true gfp

check data/buttazzo-5.txt true gedf
check data/buttazzo-5.txt false gfp

check data/buttazzo-6.txt true gedf
check data/buttazzo-6.txt false gfp

check data/dbf-anomaly.txt false feasible

check data/edf-no-llf-yes.txt true llf
check data/edf-no-llf-yes.txt false gedf

check data/edf-nonopt.txt true onlfeas
check data/edf-nonopt.txt false gedf

check data/fp-no-edf-yes.txt true gedf
check data/fp-no-edf-yes.txt false gfp

check data/nonsync.txt false feasible


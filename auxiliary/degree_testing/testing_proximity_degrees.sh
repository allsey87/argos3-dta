#!/bin/bash

arrayRANGETEMP=(0.50 0.52 0.54 0.56 0.58 0.60 0.62 0.64 0.66 0.68 0.70 0.72 0.74 0.76 0.78 0.80 0.82 0.84 0.86 0.88 0.90 0.92 0.94 0.96 0.98 1.00 1.02 1.04 1.06 1.08 1.10 1.12 1.14 1.16 1.18 1.20 1.22 1.24 1.26 1.28 1.30 1.32 1.34 1.36 1.38 1.40 1.42 1.44 1.46 1.48 1.5 1.52 1.54 1.56 1.58 1.6 1.62 1.64 1.66 1.68 1.7 1.72 1.74 1.76 1.78 1.8 1.82 1.84 1.86 1.88 1.9 1.92 1.94 1.96 1.98 2 2.02 2.04 2.06 2.08 2.1 2.12 2.14 2.16 2.18 2.2 2.22 2.24 2.26 2.28 2.3 2.32 2.34 2.36 2.38 2.4 2.42 2.44 2.46 2.48 2.5 2.52 2.54 2.56 2.58 2.6 2.62 2.64 2.66 2.68);		
		

TOPOLOGY="proximity"

MIN_DEG=0;
MAX_DEG=$((${#arrayRANGETEMP[@]}-1));

SIZE=$1;

MIN_SEED=1;
MAX_SEED=30;
fullDir="/users/irausch/argos3-dta";

specDir=$TOPOLOGY;
cd $fullDir;
CODENAME="size"_"$SIZE"_"DEGREE_TESTING"
mkdir results/"$CODENAME";
mkdir results/"$CODENAME"/"$specDir";

for degree in `seq $MIN_DEG $MAX_DEG`;
	do mkdir "$fullDir"/results/"$CODENAME"/"$specDir"/"$degree";
	for seed in `seq $MIN_SEED $MAX_SEED`;
		do mkdir "$fullDir"/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"; 
		
		if [ "$TOPOLOGY" != "proximity" -a "$TOPOLOGY" != "no_comm" ]; then 
			cp $fullDir/argos_files/size_"$SIZE"/"$TOPOLOGY"_"$SIZE"/"$TOPOLOGY"_"$degree"_"$seed".argos.in $fullDir/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"/"$TOPOLOGY"_"$degree"_"$seed".argos.in;
		else
			cp $fullDir/argos_files/size_"$SIZE"/"$TOPOLOGY"_"$SIZE"/"$TOPOLOGY"_deg_test.argos.in $fullDir/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"/"$TOPOLOGY"_"$degree"_"$seed".argos.in;
		fi
	done;
done;

parallel --bar --delay 1.0 'TOPOL={2}; RANGE={3}; SEED={4}; SPECDIR={5}; CODENAME={6};
	fullPath={1}/results/"$CODENAME"/$SPECDIR/$RANGE/$SEED;
	argFile=$fullPath/{2}_{3}_{4}.argos.in;
	cd $fullPath;
		
	arrayRANGE=(0.50 0.52 0.54 0.56 0.58 0.60 0.62 0.64 0.66 0.68 0.70 0.72 0.74 0.76 0.78 0.80 0.82 0.84 0.86 0.88 0.90 0.92 0.94 0.96 0.98 1.00 1.02 1.04 1.06 1.08 1.10 1.12 1.14 1.16 1.18 1.20 1.22 1.24 1.26 1.28 1.30 1.32 1.34 1.36 1.38 1.40 1.42 1.44 1.46 1.48 1.5 1.52 1.54 1.56 1.58 1.6 1.62 1.64 1.66 1.68 1.7 1.72 1.74 1.76 1.78 1.8 1.82 1.84 1.86 1.88 1.9 1.92 1.94 1.96 1.98 2 2.02 2.04 2.06 2.08 2.1 2.12 2.14 2.16 2.18 2.2 2.22 2.24 2.26 2.28 2.3 2.32 2.34 2.36 2.38 2.4 2.42 2.44 2.46 2.48 2.5 2.52 2.54 2.56 2.58 2.6 2.62 2.64 2.66 2.68);		
		
		
	xmlstarlet ed --inplace -u 'argos-configuration//framework/experiment/@random_seed' -v $SEED $argFile; 
	xmlstarlet ed --inplace -u 'argos-configuration//controllers/lua_controller/actuators/wifi/@range' -v ${arrayRANGE["$RANGE"]} $argFile;
		
	argos3 -l $fullPath/log -e $fullPath/logerr -c $argFile' ::: "$fullDir" ::: "$TOPOLOGY" ::: `seq $MIN_DEG $MAX_DEG` ::: `seq $MIN_SEED $MAX_SEED` ::: "$specDir" ::: "$CODENAME"
	
cd $fullDir/results/"$CODENAME";
tar czf "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz "$specDir"
cp "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz /groups/wall2-ilabt-iminds-be/pl-compas/exp/results/task_allocation/

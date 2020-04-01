#!/bin/bash


TOPOLOGY=$1

MIN_DEG=$2;
MAX_DEG=$3;

SIZE=$4;

MIN_SEED=1;
MAX_SEED=30;
fullDir="/users/irausch/argos3-dta";

specDir=$TOPOLOGY;
cd $fullDir;
CODENAME="size"_"$SIZE"_"hetero_shading"
mkdir results/"$CODENAME";
mkdir results/"$CODENAME"/"$specDir";

for degree in `seq $MIN_DEG $MAX_DEG`;
	do mkdir "$fullDir"/results/"$CODENAME"/"$specDir"/"$degree";
	for seed in `seq $MIN_SEED $MAX_SEED`;
		do mkdir "$fullDir"/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"; 
		cp $fullDir/argos_files/size_"$SIZE"/"$TOPOLOGY"_"$SIZE"/"$TOPOLOGY"_"$degree"_"$seed".argos.in $fullDir/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"/"$TOPOLOGY"_"$degree"_"$seed".argos.in;
	done;
done;

parallel --delay 1.0 'TOPOL={2}; DEGREE={3}; SEED={4}; SPECDIR={5}; CODENAME={6};
	fullPath={1}/results/"$CODENAME"/$SPECDIR/$DEGREE/$SEED;
	argFile=$fullPath/{2}_{3}_{4}.argos.in;
	cd $fullPath;
	
	xmlstarlet ed --inplace -u 'argos-configuration//framework/experiment/@random_seed' -v $SEED $argFile; 
	xmlstarlet ed --inplace -u 'argos-configuration//framework/experiment/@length' -v 10000 $argFile;
		
	argos3 -l $fullPath/log -e $fullPath/logerr -c $argFile' ::: "$fullDir" ::: "$TOPOLOGY" ::: `seq $MIN_DEG $MAX_DEG` ::: `seq $MIN_SEED $MAX_SEED` ::: "$specDir" ::: "$CODENAME"
	
cd $fullDir/results/"$CODENAME";
tar czf "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz "$specDir"
cp "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz /groups/wall2-ilabt-iminds-be/pl-compas/exp/results/task_allocation/

#!/bin/bash

#~ arrayRANGETEMP=(0.0) # <------------------- CHOOSE THIS FOR THE "ZERO DIRECT COMMUNICATION" SCENARIO! 
#~ TOPOLOGY="no_comm" # <------------------- CHOOSE THIS FOR THE "ZERO DIRECT COMMUNICATION" SCENARIO! 
arrayRANGETEMP=(1.2 2.7)
TOPOLOGY="proximity"

MIN_DEG=0;
MAX_DEG=$((${#arrayRANGETEMP[@]}-1));

SIZE=$1;

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
		
		if [ "$TOPOLOGY" != "proximity" -a "$TOPOLOGY" != "no_comm" ]; then 
			cp $fullDir/argos_files/size_"$SIZE"/"$TOPOLOGY"_"$SIZE"/"$TOPOLOGY"_"$degree"_"$seed".argos.in $fullDir/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"/"$TOPOLOGY"_"$degree"_"$seed".argos.in;
		else
			cp $fullDir/argos_files/size_"$SIZE"/"$TOPOLOGY"_"$SIZE"/"$TOPOLOGY"_"$degree".argos.in $fullDir/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"/"$TOPOLOGY"_"$degree"_"$seed".argos.in;
		fi
	done;
done;

logs=~/logs;

#####################
#~ ATTENTION!
#~ If you changed arrayRANGETEMP above, then also update 
#~ arrayRANGE in the gnu parallel command below!
#####################

parallel --delay 1.0 'TOPOL={2}; DEGREE={3}; SEED={4}; SPECDIR={5}; CODENAME={6}; LOGS={7}
	fullPath={1}/results/"$CODENAME"/$SPECDIR/$DEGREE/$SEED;
	argFile=$fullPath/{2}_{3}_{4}.argos.in;
	cd $fullPath;
	
	arrayRANGE=(1.2 2.7)
	
	xmlstarlet ed --inplace -u 'argos-configuration//framework/experiment/@random_seed' -v $SEED $argFile; 
	xmlstarlet ed --inplace -u 'argos-configuration//framework/experiment/@length' -v 10000 $argFile;
	xmlstarlet ed --inplace -u 'argos-configuration//controllers/lua_controller/actuators/wifi/@range' -v ${arrayRANGE["$RANGE"]} $argFile;
		
	argos3 -l $LOGS/log -e $LOGS/logerr -c $argFile' ::: "$fullDir" ::: "$TOPOLOGY" ::: `seq $MIN_DEG $MAX_DEG` ::: `seq $MIN_SEED $MAX_SEED` ::: "$specDir" ::: "$CODENAME" ::: "$logs"
	
cd $fullDir/results/"$CODENAME";
tar czf "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz "$specDir"
cp "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz /groups/wall2-ilabt-iminds-be/pl-compas/exp/results/task_allocation/

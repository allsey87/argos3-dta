#!/bin/bash

SIZE=$1;

arrayRANGE=(0.0) # <------------------- CHOOSE THIS FOR THE "ZERO DIRECT COMMUNICATION" SCENARIO! 
#~ TOPOLOGY="no_comm" # <------------------- CHOOSE THIS FOR THE "ZERO DIRECT COMMUNICATION" SCENARIO! 
if [ "$SIZE" -eq "20" ]; then
	arrayRANGE=(2.17 2.80 3.4 4.08 4.62 5.56); # <------------------- For swarm size 20
elif [ "$SIZE" -eq "30" ]; then
	arrayRANGE=(1.54 1.94 2.36 2.70 3.04 3.38); # <------------------- For swarm size 30
elif [ "$SIZE" -eq "40" ]; then
	arrayRANGE=(1.28 1.60 1.94 2.22 2.44 2.70); # <------------------- For swarm size 40
elif [ "$SIZE" -eq "50" ]; then
	arrayRANGE=(1.09 1.36 1.62 1.82 2.06 2.26); # <------------------- For swarm size 50
else
	echo "UKNOWN SWARM SIZE!";
fi
TOPOLOGY="proximity"

MIN_DEG=0;
MAX_DEG=$((${#arrayRANGE[@]}-1));

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
			cp $fullDir/argos_files/size_"$SIZE"/"$TOPOLOGY"_"$SIZE"/"$TOPOLOGY"_deg_test.argos.in $fullDir/results/"$CODENAME"/"$specDir"/"$degree"/"$seed"/"$TOPOLOGY"_"$degree"_"$seed".argos.in;
		fi
	done;
done;


logs=~/logs;
mkdir "$logs";

#####################
#~ ATTENTION!
#~ If you changed arrayRANGETEMP above, then also update 
#~ arrayRANGE in the gnu parallel command below!
#####################

parallel --bar --delay 1.0 'TOPOL={2}; DEGREE={3}; RANGE={4}; SEED={5}; SPECDIR={6}; CODENAME={7}; LOGS={8}
	fullPath={1}/results/"$CODENAME"/$SPECDIR/$DEGREE/$SEED;
	argFile=$fullPath/{2}_{3}_{5}.argos.in;
	cd $fullPath;

	xmlstarlet ed --inplace -u 'argos-configuration//framework/experiment/@random_seed' -v $SEED $argFile; 
	xmlstarlet ed --inplace -u 'argos-configuration//framework/experiment/@length' -v 10000 $argFile;
	xmlstarlet ed --inplace -u 'argos-configuration//controllers/lua_controller/actuators/wifi/@range' -v $RANGE $argFile;
		
	argos3 -l $LOGS/log -e $LOGS/logerr -c $argFile' ::: "$fullDir" ::: "$TOPOLOGY" ::: `seq $MIN_DEG $MAX_DEG` :::+ ${arrayRANGE[*]} ::: `seq $MIN_SEED $MAX_SEED` ::: "$specDir" ::: "$CODENAME" ::: "$logs"
	
cd $fullDir/results/"$CODENAME";
tar czf "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz "$specDir"
cp "$specDir"_"$MIN_DEG"_"$MAX_DEG"_"$CODENAME".tar.gz /groups/wall2-ilabt-iminds-be/pl-compas/exp/results/task_allocation/

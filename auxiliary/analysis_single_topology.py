import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

#~ This code plots a figure with 4 panels, with each panel corresponding to one of the 4 swarm sizes [20, 30, 40, 50]

def average_over_seeds(size, network, deg):
#~ This function opens the files for 30 seeds and returns the mean tranjectories of the specified property
#~ The mean value is calculated over 45000 seconds, while the experiment duration is 50000 seconds.
#~ 		This is because, at each time step, the loop-functions write the features into the file iff there is at least one robot in the arena.
#~		While this is almost always the case, it is safer to leave a "margin of error". 
	feature='average estimate'
	if network != "proximity" and network != "no_comm":
		feature = 'building robots' # <---- This feature name actually belongs to 'average estimate'. For some reason pandas keeps messing it up the column names. For proximity and no_comm this issue is fixed.
	
	filename = '/users/irausch/analysis/results/hetero_shading/size_' + size + '/' + network + '/' + deg + '/'
	
	fname = filename + str(1) + '/data.csv'
	data = pd.read_csv(fname, sep='\t')
	av_est = np.array(data[feature])[:45000]
	
	for seed in range(2,31):
		fname = filename + str(seed) + '/data.csv'	
		data = pd.read_csv(fname, sep='\t')
		av_est = av_est + np.array(data[feature])[:45000] 

	av_est = av_est / 30
	time_array = np.arange(0,len(av_est),1)
	new_arr = np.array([av_est, time_array])

	return new_arr


#~ Possible network topologies are:
#~ no_comm, proximity, random, regular, watt-strogatz, scale-free
network=str(sys.argv[1]) # <------- This file takes a network topology as input

if network == "no_comm":
	degsId=["0"]
	degs=['0']
elif network == "proximity":
	degsId=[str(x) for x in range(0,6)]
	degs=['4','6','8','10', '12', '14']
else:
	degsId=[str(x) for x in range(2,6)]
	degs=['8','12','16','20']

sizes=['20', '30', '40', '50']
y_axis_range=[0,0.2]
y_axis_label="Avg Estimate"
outFileName='avg_est_network_'

fig, ax = plt.subplots(nrows=2, ncols=2, sharex=True, sharey=True)

i=0
size = sizes[i]
print("Drawing size %s ..." % size)

for d in range(0,len(degs)):
	seed_averaged_data = average_over_seeds(size, network, degsId[d])
	ax[0,0].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

ax[0,0].set_ylim(y_axis_range)
ax[0,0].set_ylabel(y_axis_label)
ax[0,0].set_title(size)
ax[0,0].legend(loc='upper center', ncol=2)
start, end = ax[0,0].get_xlim()
ax[0,0].xaxis.set_ticks(np.arange(start, end, 10000))

i=1
size = sizes[i]
print("Drawing size %s ..." % size)

for d in range(0,len(degs)):
	seed_averaged_data = average_over_seeds(size, network, degsId[d])
	ax[0,1].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

ax[0,1].set_ylim(y_axis_range)
ax[0,1].set_title(size)
ax[0,1].legend(loc='upper center', ncol=2)
start, end = ax[0,1].get_xlim()
ax[0,1].xaxis.set_ticks(np.arange(start, end, 10000))


i=2
size = sizes[i]
print("Drawing size %s ..." % size)

for d in range(0,len(degs)):
	seed_averaged_data = average_over_seeds(size, network, degsId[d])
	ax[1,0].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

ax[1,0].set_ylim(y_axis_range)
ax[1,0].set_ylabel(y_axis_label)
ax[1,0].set_xlabel("Time")
ax[1,0].set_title(size)
ax[1,0].legend(loc='upper center', ncol=2)
start, end = ax[1,0].get_xlim()
ax[1,0].xaxis.set_ticks(np.arange(start, end, 10000))

i=3
size = sizes[i]
print("Drawing size %s ..." % size)

for d in range(0,len(degs)):
	seed_averaged_data = average_over_seeds(size, network, degsId[d])
	ax[1,1].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

ax[1,1].set_ylim(y_axis_range)
ax[1,1].set_xlabel("Time")
ax[1,1].set_title(size)
ax[1,1].legend(loc='upper center', ncol=2)
start, end = ax[1,1].get_xlim()
ax[1,1].xaxis.set_ticks(np.arange(start, end, 10000))


outfname=outFileName + network + '.png'
fig.savefig(outfname, dpi=fig.dpi)
	
print("DONE.")

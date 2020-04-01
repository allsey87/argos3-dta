import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

#~ This code plots, for each swarm size, a figure with 4 panels. Each panel corresponds to one of the 4 topologies [random, regular, watt-strogatz, scale-free]

def average_over_seeds(size, network, deg):
#~ This function opens the files for 30 seeds and returns the mean tranjectories of the specified property
#~ The mean value is calculated over 45000 seconds, while the experiment duration is 50000 seconds.
#~ 		This is because, at each time step, the loop-functions write the features into the file iff there is at least one robot in the arena.
#~		While this is almost always the case, it is safer to leave a "margin of error". 
	feature='building robots' # <---- This feature name actually belongs to 'average estimate'. For some reason pandas keeps messing it up the column names.
	
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


degsId=['2','3','4','5']
degs=['8','12','16','20']
sizes=['20', '30', '40', '50']
y_axis_range=[0,0.2]
y_axis_label="Avg Estimate"
outFileName='avg_est_swarmSize_'

for size in sizes:
	fig, ax = plt.subplots(nrows=2, ncols=2, sharex=True, sharey=True)
	network='random'
	print("Drawing %s ..." % network)

	for d in range(0,len(degs)):
		seed_averaged_data = average_over_seeds(size, network, degsId[d])
		ax[0,0].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[0,0].set_ylim(y_axis_range)
	ax[0,0].set_ylabel(y_axis_label)
	panel_title = "topology " + network + ", size " + size
	ax[0,0].set_title(panel_title)
	ax[0,0].legend(loc='upper center', ncol=2)
	start, end = ax[0,0].get_xlim()
	ax[0,0].xaxis.set_ticks(np.arange(start, end, 10000))

	network='regular'
	print("Drawing %s ..." % network)

	for d in range(0,len(degs)):
		seed_averaged_data = average_over_seeds(size, network, degsId[d])
		ax[0,1].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[0,1].set_ylim(y_axis_range)
	panel_title = "topology " + network + ", size " + size
	ax[0,1].set_title(panel_title)
	ax[0,1].legend(loc='upper center', ncol=2)
	start, end = ax[0,1].get_xlim()
	ax[0,1].xaxis.set_ticks(np.arange(start, end, 10000))


	network='watts-strogatz'
	print("Drawing %s ..." % network)

	for d in range(0,len(degs)):
		seed_averaged_data = average_over_seeds(size, network, degsId[d])
		ax[1,0].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[1,0].set_ylim(y_axis_range)
	ax[1,0].set_ylabel(y_axis_label)
	ax[1,0].set_xlabel("Time")
	panel_title = "topology " + network + ", size " + size
	ax[1,0].set_title(panel_title)
	ax[1,0].legend(loc='upper center', ncol=2)
	start, end = ax[1,0].get_xlim()
	ax[1,0].xaxis.set_ticks(np.arange(start, end, 10000))

	network='scale-free'
	print("Drawing %s ..." % network)

	for d in range(0,len(degs)):
		seed_averaged_data = average_over_seeds(size, network, degsId[d])
		ax[1,1].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[1,1].set_ylim(y_axis_range)
	ax[1,1].set_xlabel("Time")
	panel_title = "topology " + network + ", size " + size
	ax[1,1].set_title(panel_title)
	ax[1,1].legend(loc='upper center', ncol=2)
	start, end = ax[1,1].get_xlim()
	ax[1,1].xaxis.set_ticks(np.arange(start, end, 10000))


	outfname=outFileName + size + '.png'
	fig.savefig(outfname, dpi=fig.dpi)
	
print("DONE.")

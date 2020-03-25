import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def average_over_seeds(filename):
	fname = filename + str(1) + '/data.csv'
	#~ columns = ['foraging robots', 'building robots', 'average estimate', 'average gradient', 'construction events', 'blocks in cache']
	#~ data = pd.DataFrame(columns=columns)
	#~ data = data.append(pd.read_csv(fname, sep='\t'), ignore_index = True)
	data = pd.read_csv(fname, sep='\t')
	av_est = np.array(data['building robots'])[:45000] # <---- This column has the wrong name but the right data which actually belongs to 'average estimate'. For some reason pandas keeps messing it up.
	#~ av_est = np.array(data['blocks in cache'])[:45000]
	for seed in range(2,31):
		fname = filename + str(seed) + '/data.csv'	
		data = pd.read_csv(fname, sep='\t')
		av_est = av_est + np.array(data['building robots'])[:45000] # <---- This column has the wrong name but the right data which actually belongs to 'average estimate'. For some reason pandas keeps messing it up.
		#~ av_est = av_est + np.array(data['blocks in cache'])[:45000]

	av_est = av_est / 30
	time_array = np.arange(0,len(av_est),1)
	new_arr = np.array([av_est, time_array])

	return new_arr



sizes=['20', '30', '40', '50']
#~ sizes=[]
for size in sizes:
	fig, ax = plt.subplots(nrows=2, ncols=2, sharex=True, sharey=True)
	network='random'
	print("Drawing %s ..." % network)
	degsId=['2','3','4','5']
	degs=['8','12','16','20']

	for d in range(0,len(degs)):
		filename = '/users/irausch/analysis/results/hetero_shading/size_' + size + '/' + network + '/' + degsId[d] + '/'
		seed_averaged_data = average_over_seeds(filename)
		ax[0,0].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[0,0].set_ylim([0,0.2])
	ax[0,0].set_ylabel("Avg Estimate")
	ax[0,0].set_label("Time")
	ax[0,0].set_title(network)
	ax[0,0].legend(loc='upper center', ncol=2)
	start, end = ax[0,0].get_xlim()
	ax[0,0].xaxis.set_ticks(np.arange(start, end, 10000))

	network='regular'
	print("Drawing %s ..." % network)
	degsId=['2','3','4','5']
	degs=['8','12','16','20']

	for d in range(0,len(degs)):
		filename = '/users/irausch/analysis/results/hetero_shading/size_' + size + '/' + network + '/' + degsId[d] + '/'
		seed_averaged_data = average_over_seeds(filename)
		ax[0,1].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[0,1].set_ylim([0,0.2])
	ax[0,1].set_label("Avg Estimate")
	ax[0,1].set_label("Time")
	ax[0,1].set_title(network)
	ax[0,1].legend(loc='upper center', ncol=2)
	start, end = ax[0,1].get_xlim()
	ax[0,1].xaxis.set_ticks(np.arange(start, end, 10000))


	network='watts-strogatz'
	print("Drawing %s ..." % network)
	degsId=['2','3','4','5']
	degs=['8','12','16','20']

	for d in range(0,len(degs)):
		filename = '/users/irausch/analysis/results/hetero_shading/size_' + size + '/' + network + '/' + degsId[d] + '/'
		seed_averaged_data = average_over_seeds(filename)
		ax[1,0].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[1,0].set_ylim([0,0.2])
	ax[1,0].set_ylabel("Avg Estimate")
	ax[1,0].set_xlabel("Time")
	ax[1,0].set_title(network)
	ax[1,0].legend(loc='upper center', ncol=2)
	start, end = ax[1,0].get_xlim()
	ax[1,0].xaxis.set_ticks(np.arange(start, end, 10000))

	network='scale-free'
	print("Drawing %s ..." % network)
	degsId=['2','3','4','5']
	degs=['8','12','16','20']

	for d in range(0,len(degs)):
		filename = '/users/irausch/analysis/results/hetero_shading/size_' + size + '/' + network + '/' + degsId[d] + '/'
		seed_averaged_data = average_over_seeds(filename)
		ax[1,1].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

	ax[1,1].set_ylim([0,0.2])
	ax[1,1].set_ylabel("Avg Estimate")
	ax[1,1].set_xlabel("Time")
	ax[1,1].set_title(network)
	ax[1,1].legend(loc='upper center', ncol=2)
	start, end = ax[1,1].get_xlim()
	ax[1,1].xaxis.set_ticks(np.arange(start, end, 10000))


	outfname='hetero_shading/avg_bic_swarmSize_' + size + '.png'
	fig.savefig(outfname, dpi=fig.dpi)
	
print("DONE.")

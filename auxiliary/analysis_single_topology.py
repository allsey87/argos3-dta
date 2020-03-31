import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

def average_over_seeds(size, network, deg):
	filename = '/users/irausch/analysis/results/hetero_shading/size_' + size + '/' + network + '/' + deg + '/'
	fname = filename + str(1) + '/data.csv'
	data = pd.read_csv(fname, sep='\t')
	column='average estimate'
	if network != "proximity" and network != "no_comm":
		column = 'building robots'
	av_est = np.array(data[column])[:45000] 
	for seed in range(2,31):
		fname = filename + str(seed) + '/data.csv'	
		data = pd.read_csv(fname, sep='\t')
		av_est = av_est + np.array(data[column])[:45000] 

	av_est = av_est / 30
	time_array = np.arange(0,len(av_est),1)
	new_arr = np.array([av_est, time_array])

	return new_arr


network=str(sys.argv[1])

if network == "no_comm":
	degsId=["0"]
	degs=['0']
elif network == "proximity":
	degsId=[str(x) for x in range(0,6)]
	degs=['4','6','8','10', '12', '14']
else:
	degsId=[str(x) for x in range(2,6)]
	degs=['8','12','16','20']

#~ sizes=['50', '50', '50', '50']
sizes=['20', '30', '40', '50']

fig, ax = plt.subplots(nrows=2, ncols=2, sharex=True, sharey=True)

i=0
size = sizes[i]
print("Drawing size %s ..." % size)

for d in range(0,len(degs)):
	seed_averaged_data = average_over_seeds(size, network, degsId[d])
	ax[0,0].plot(seed_averaged_data[1], seed_averaged_data[0], label=degs[d])

ax[0,0].set_ylim([0,1.0])
ax[0,0].set_ylabel("Avg Estimate")
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

ax[0,1].set_ylim([0,1.0])
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

ax[1,0].set_ylim([0,1.0])
ax[1,0].set_ylabel("Avg Estimate")
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

ax[1,1].set_ylim([0,1.0])
ax[1,1].set_xlabel("Time")
ax[1,1].set_title(size)
ax[1,1].legend(loc='upper center', ncol=2)
start, end = ax[1,1].get_xlim()
ax[1,1].xaxis.set_ticks(np.arange(start, end, 10000))


#~ outfname='hetero_shading/avg_bic_swarmSize_' + size + '.png'
outfname='hetero_shading/avg_est_' + network + '.png'
fig.savefig(outfname, dpi=fig.dpi)
	
print("DONE.")

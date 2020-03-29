import os
import pandas as pd
import numpy as np

def average_over_seeds(filename):
	fname = filename + str(1) + '/data.csv'
	data = pd.read_csv(fname, sep='\t')[38:48]
	av_est = np.mean(np.array(data['average degree'])) 
	for seed in range(2,31):
		fname = filename + str(1) + '/data.csv'	
		data = pd.read_csv(fname, sep='\t')[38:48]
		av_est = av_est + np.mean(np.array(data['average degree'])) 
		
	av_est = av_est / 30

	return av_est



#~ sizes=['20', '30', '40', '50']
sizes=['50']
for size in sizes:
	network='proximity'
	
	#~ ranges=[x for x in range(0.5, 25.00, 0.02)]
	ranges=[i*0.02+0.5 for i in range(0, int(24.5/0.02)) ]
	
	with open('degree_testing_output.csv', 'w+') as outFile:
		#~ for d in range(0,len(ranges)):
		for d in range(566,573):
			filename = '/users/irausch/argos3-dta/results/size_' + size + '_DEGREE_TESTING/' + network + '/' + str(d) + '/'
			seed_average = average_over_seeds(filename)
			
			print("%d\t%f\t%f" % (d, ranges[d], seed_average))
			outFile.write("%f\t%f" % (ranges[d], seed_average))
	
print("DONE.")

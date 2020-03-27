import os
import pandas as pd
import numpy as np

def average_over_seeds(filename):
	fname = filename + str(1) + '/data.csv'
	data = pd.read_csv(fname, sep='\t')
	av_est = np.mean(np.array(data['average degree'])) 
	for seed in range(2,31):
		fname = filename + str(seed) + '/data.csv'	
		data = pd.read_csv(fname, sep='\t')
		av_est = av_est + np.mean(np.array(data['average degree'])) 
		
	av_est = av_est / 30

	return av_est



#~ sizes=['20', '30', '40', '50']
sizes=['50']
for size in sizes:
	network='proximity'
	
	ranges=[0.50, 0.52, 0.54, 0.56, 0.58, 0.60, 0.62, 0.64, 0.66, 0.68, 0.70, 0.72, 0.74, 0.76, 0.78, 0.80, 0.82, 0.84, 0.86, 0.88, 0.90, 0.92, 0.94, 0.96, 0.98, 1.00, 1.02, 1.04, 1.06, 1.08, 1.10, 1.12, 1.14, 1.16, 1.18, 1.20, 1.22, 1.24, 1.26, 1.28, 1.30, 1.32, 1.34, 1.36, 1.38, 1.40, 1.42, 1.44, 1.46, 1.48, 1.5, 1.52, 1.54, 1.56, 1.58, 1.6, 1.62, 1.64, 1.66, 1.68, 1.7, 1.72, 1.74, 1.76, 1.78, 1.8, 1.82, 1.84, 1.86, 1.88, 1.9, 1.92, 1.94, 1.96, 1.98, 2, 2.02, 2.04, 2.06, 2.08, 2.1, 2.12, 2.14, 2.16, 2.18, 2.2, 2.22, 2.24, 2.26, 2.28, 2.3, 2.32, 2.34, 2.36, 2.38, 2.4, 2.42, 2.44, 2.46, 2.48, 2.5, 2.52, 2.54, 2.56, 2.58, 2.6, 2.62, 2.64, 2.66, 2.68]		
	
	with open('degree_testing_output.csv', 'w+') as outFile:
		for d in range(0,len(ranges)):
		#~ for d in range(0,10):
			filename = '/users/irausch/argos3-dta/results/size_' + size + '_DEGREE_TESTING/' + network + '/' + str(d) + '/'
			seed_average = average_over_seeds(filename)
			
			print("%f\t%f" % (ranges[d], seed_average))
			outFile.write("%f\t%f" % (ranges[d], seed_average))
	
print("DONE.")

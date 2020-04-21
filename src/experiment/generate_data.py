import xml.etree.ElementTree
import tempfile
import threading
import subprocess
import tables
import pandas

from copy import deepcopy
from time import sleep

# create a lock for writing to the terminal
terminal_lock = threading.Lock()

# define an ARGoS job (subclass of Thread)
class ARGoSJob(threading.Thread):
   def __init__(self, desc, config, seed, dataset):
      self.desc = desc
      self.config = deepcopy(config)
      self.seed = seed
      self.dataset = dataset
      with self.dataset['lock']:
         self.dataset['jobs'] += 1
      # create an output file
      self.output_file = tempfile.NamedTemporaryFile(mode='r', suffix='.csv', delete=False)
      # set the output file
      self.config.find('./loop_functions').attrib['output'] = self.output_file.name
      # set the seed
      self.config.find('./framework/experiment').attrib['random_seed'] = str(self.seed)
      # create the configuration file
      self.config_file = tempfile.NamedTemporaryFile(mode='w+b', suffix='.argos', delete=False)
      self.config_file.write(xml.etree.ElementTree.tostring(self.config))
      self.config_file.flush()
      # call the super class constructor
      super().__init__()
      
   def run(self):
      with terminal_lock:
         print('starting job: %s (seed = %d)' % (self.desc, self.seed))
         print('  command: argos3 -c %s' % self.config_file.name)
         print('  output file: %s' % self.output_file.name)
      subprocess.run(['argos3', '-c', self.config_file.name], capture_output=True)
      data = pandas.read_csv(self.output_file)
      data.insert(0, 'seed', self.seed)
      # acquire the lock
      with self.dataset['lock']:
         self.dataset['data'] = self.dataset['data'].append(data)
         self.dataset['jobs'] -= 1
         # if we were the last job
         if self.dataset['jobs'] == 0:
            with terminal_lock:
               print('writing data to %s.hdf' % self.dataset['name'])
            self.dataset['data'].to_hdf('%s.hdf' % self.dataset['name'], 'table', append=False)

def run_argos_jobs(jobs, threads):
   active_jobs = {}
   # transfer thread jobs to the active job dictionary and start them
   for thread in range(0, threads):
      if jobs:
         active_jobs[thread] = jobs.pop()
         active_jobs[thread].start()
         sleep(0.1)
   while active_jobs:
      for thread, active_job in active_jobs.items():
         if active_job.is_alive():
            continue
         else:
            if jobs:
               active_jobs[thread] = jobs.pop()
               active_jobs[thread].start()
               sleep(0.1)
            else:
               active_jobs.pop(thread)
               break;

# list of jobs for ARGoS
jobs = []

# open the template configuration file
config = xml.etree.ElementTree.parse('@CMAKE_BINARY_DIR@/experiment/template.argos').getroot()
framework = config.find('./framework')
experiment = framework.find('./experiment')
visualization = config.find('./visualization')
loop_functions = config.find('./loop_functions')
parameters = loop_functions.find('./parameters')
# remove the qtopengl visualization
if visualization.find('./qt-opengl') is not None:
   visualization.remove(visualization.find('./qt-opengl'))

# set the loop function parameters
parameters.attrib['enable_foraging'] = 'false'
parameters.attrib['mean_foraging_duration_initial'] = '0'
parameters.attrib['mean_foraging_duration_gradient'] = '0'
parameters.attrib['construction_limit'] = '0'

# experiment parameters
experiment.attrib['length'] = '960'

# define jobs
biased_dataset = {
   'name': 'biased',
   'lock': threading.Lock(),
   'data': pandas.DataFrame(),
   'jobs': 0,
}
for run in range(0,5):
   seed = run + 1
   parameters.attrib['shading_distribution'] = 'biased'
   # desc, config, seed, dataset
   job = ARGoSJob('biased, seed = %s' % seed, config, seed, biased_dataset)
   jobs.append(job)

uniform_dataset = {
   'name': 'uniform',
   'lock': threading.Lock(),
   'data': pandas.DataFrame(),
   'jobs': 0,
}
for run in range(0,5):
   seed = run + 1
   parameters.attrib['shading_distribution'] = 'uniform'
   # desc, config, seed, dataset
   job = ARGoSJob('biased, seed = %s' % seed, config, seed, uniform_dataset)
   jobs.append(job)

# execute all jobs
run_argos_jobs(jobs, 8)

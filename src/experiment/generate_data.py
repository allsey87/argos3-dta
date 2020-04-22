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

# run the argos jobs in parallel
def run_argos_jobs(jobs, threads):
   active_jobs = {}
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

# create a new dataset
def create_dataset(name):
   return {
      'name': name,
      'lock': threading.Lock(),
      'data': pandas.DataFrame(),
      'jobs': 0,
   }

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

# set the default loop function parameters
parameters.attrib['enable_foraging'] = 'true'
parameters.attrib['mean_foraging_duration_initial'] = '1'
parameters.attrib['mean_foraging_duration_gradient'] = '0'
parameters.attrib['construction_limit'] = '5'
parameters.attrib['shading_distribution'] = 'biased'

# experiment parameters
experiment.attrib['length'] = '750'

# pipuck controller parameters
pipuck_params = config.find('./controllers/lua_controller/params')
pipuck_wifi_actuator = config.find('./controllers/lua_controller/actuators/wifi')

for wifi_range in ['1']:
   pipuck_wifi_actuator.attrib['range'] = wifi_range
   for ttl_value in ['4']:
      pipuck_params.attrib['ttl'] = ttl_value
      for acc_len in ['100']:
         pipuck_params.attrib['accumulator_length'] = acc_len
         for target_density in ['0.2', '0.3', '0.4']:
            pipuck_params.attrib['target_density'] = target_density
            dataset = create_dataset('wr%s_ttl%s_al%s_td%s' % (wifi_range, ttl_value, acc_len, target_density))
            for run in range(0,5):
               seed = run + 1
               desc = ('[wifi range: %s, ttl: %s, accumulator length: %s, target density: %s]' % (wifi_range, ttl_value, acc_len, target_density))
               job = ARGoSJob(desc, config, seed, dataset)
               jobs.append(job)

# execute all jobs
run_argos_jobs(jobs, 8)

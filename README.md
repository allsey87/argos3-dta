# Dynamic task allocation for swarm robotics construction

This repository contains the ARGoS controllers and loop functions for investigating dynamic task allocation for swarm robotics construction.

## Dependencies
* [ARGoS3 (beta56 or higher)](https://www.argos-sim.info/core.php)
* [ARGoS3-SRoCS (commit #7350b82
or later)](https://github.com/allsey87/argos3-srocs)

## Installation and compilation
1. Install ARGoS3 from the package or compile from source and install as follows:
```bash
git clone https://github.com/ilpincy/argos3.git
cd argos3
mkdir build
cd build
cmake -DARGOS_DOCUMENTATION=OFF -DCMAKE_BUILD_TYPE=Release ../src
make
sudo make install
```

2.  Compile from source and install ARGoS3-SRoCS as follows:
```bash
git clone https://github.com/allsey87/argos3-srocs.git
cd argos3-srocs
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../src
make
sudo make install
```

3. Compile the loop functions and configure the experiments
```bash
# clone this repository
git clone https://github.com/allsey87/argos3-dta.git
# configure and compile
cd argos-dta
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../src
make
# run the abstract experiment
cd experiments
argos3 -c abstract.argos
```

## Limitations and notes
When using the QtOpenGL visualization for the abstract configuration, do not select the robots while the simulator is running. The QtOpenGL visualisation can not handle the removal of robots from the simulator and will crash with a segmentation fault.

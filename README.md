# Dynamic task allocation for swarm robotics construction

This repository contains the ARGoS controllers and loop functions for investigating dynamic task allocation for swarm robotics construction.

## Dependencies
* [ARGoS3 (beta56 or higher)](https://www.argos-sim.info/core.php)
* [ARGoS3-SRoCS (commit #7350b82
or later)](https://github.com/allsey87/argos3-srocs)

## Compilation and installation
1. It is recommended that you uninstall older versions of ARGoS that may be installed on your system and to remove any local configuration that you may have for ARGoS as follows.

```bash
# on Linux
rm $HOME/.config/Iridia-ULB/ARGoS.conf
# on mac
defaults write info.argos-sim.ARGoS
```

2. Install ARGoS3 from the package or compile from source and install as follows:
```bash
git clone https://github.com/ilpincy/argos3.git
cd argos3
mkdir build
cd build
cmake -DARGOS_DOCUMENTATION=OFF -DCMAKE_BUILD_TYPE=Release ../src
make
sudo make install
```

3.  Compile from source and install ARGoS3-SRoCS as follows:
```bash
git clone https://github.com/allsey87/argos3-srocs.git
cd argos3-srocs
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../src
make
sudo make install
```

4. Compile the loop functions and configure the experiments
```bash
# clone this repository (and its submodules)
git clone --recursive https://github.com/allsey87/argos3-dta.git
# configure and compile
cd argos-dta
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../src
make
```

## Running experiments
The ARGoS configuration files for running experiments are configured automatically by CMake and placed in `build/experiments`. These configuration files will be updated everytime cmake or make is executed and the input configuration (e.g., `src/experiments/abstract.argos.in`) has been modified. To this end, you may want to consider modifying `src/experiments/abstract.argos.in` instead of `build/experiments/abstract.argos` to avoid your changes being overwritten.

The Lua controllers for both the abstract and the SRoCS configurations use the behavior trees to implement a robot's behavior. Support for these behavior trees is provided via by (luabt)[https://github.com/allsey87/luabt]. This module is cloned recursively into this repository.

In order to find the luabt module, it is recommended that you run the experiments from the same directory that the configuration file is located in. That is:

```bash
cd build/experiments
argos3 -c abstract.argos
```

## Limitations
The abstract loop functions work in part by adding and removing robots from the simulation. The QtOpenGL visualization can not handle the removal of robots from the simulator while they are **selected** and will cause ARGoS to crash with a segmentation fault. It recommended that you do not step or run the simulation while robots are selected in the visualization.

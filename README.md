# Dynamic task allocation for swarm robotics construction

This repository contains the ARGoS controllers and loop functions for investigating dynamic task allocation for swarm robotics construction.

## Dependencies
To build on this work, you will need the following versions of ARGoS and the ARGoS-SRoCS plugin. For recommended instructions on how to install these, see the [compilation and installation](#compilation-and-installation) section.
* [ARGoS3 (beta57 or higher)](https://www.argos-sim.info/core.php)
* [ARGoS3-SRoCS (commit #45b00d9 or later)](https://github.com/allsey87/argos3-srocs)

## Running experiments
The ARGoS configuration files for running experiments in a small, medium, and large arena are created automatically for your system by CMake and placed in `build/experiments`. The files in the `build/experiments` are templates that need to be configured with respect to the experiment you want to run. By default, you will only get an arena with four walls since no robots have been added to the configuration.

### Configuring experiment templates
1. Under the node, `/controllers/lua_controller[pipuck]/actuators/wifi`, you have a choice of two wifi implementations, `dta_proximity` and `dta_abstract`. With `dta_proximity`, you need to give the communication range in meters, for example, `<wifi implementation="dta_proximity" range="0.75">`. Note that the range can also be inf, which would simulate a fully connected network. With `dta_abstract`, the robots are always in communication with their specified neighbours regardless of their physical proximity to each other in the simulation. These neighbours are specified later in the configuration file.
2. To add robots to the experiment, you need to add a node for each robot. For example, `<pipuck id="pipuck0" controller="pipuck" can_send_to="pipuck1,pipuck3,pipuck5">`, where:

   * `id` must be a unique id for each robot
   * `controller` must be the id of a controller declared in the controllers section
   * `can_send_to` is a list of robot ids that this robot can communicate to (unidirectional), this list is only respected when the `dta_abstract` implementation of the wifi actuator is being used.
3. The `output` attribute of the loop function node, should be a path (relative or absolute) to an output file for recording results. If not provided, the loop function will output to `std::cout`.
4. The parameters of the loop function are defined as follows:
   
   * `grid_layout` is the number of tiles in the X and Y directions (only square tiles, where X == Y, have been tested)
   * `shading_distribution` must be `uniform` or `biased`, in the uniform case new tiles are distributed uniformly, for the biased case, the tiles are distributed following a gaussian distribution in the X and Y directions with a mean in the bottom right quadrant of the arena and with a standard deviation such that only cells in that quadrant will be shaded.
   * `mean_foraging_duration_initial` is the initial mean amount of time a robot spends doing the foraging task, this value is incremented by `mean_foraging_duration_gradient` each time step, and is the input to a Poisson arrival process.
   * `construction_limit` is the maximum number of tiles that can be unshaded (used for construction) every second

### Output of experiment
The output of the loop functions is tab seperated values. The values are defined as follows:
   * foraging robots: number of robots currently foraging blocks
   * building robots: number of robots currently in the cache (capable of observing the local density of tiles)
   * average estimate: TODO
   * average deviation: TODO
   * construction events: TODO
   * blocks in cache: TODO
   * average degree: TODO

## Limitations
The loop functions work in part by adding and removing robots from the simulation. The QtOpenGL visualization can not handle the removal of robots from the simulator while they are **selected** and this will cause ARGoS to crash with a segmentation fault. It recommended that you do not step or run the simulation while robots are selected in the visualization.

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

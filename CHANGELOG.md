# Changelog

## v0.1.26:
### Added:
* Simulation config: synapse_replay input files must be .h5 (#351)
* SimulationConfig: Adds "represents_physical_electrode" field in "inputs" section (#354)

### Fixed:
* Update HighFive SHA to `2.9.0`. (#343)
* Make documentation available for Python API (#345)
* Build static macos and linux wheels (#355)

## v0.1.25:

### Added:
 - Implement fall-back for `make_collective_reader`. (#326)
 - Inject dataset reading via `Hdf5Reader`. (#307)
 - Use `using Range = std::array<Value, 2>`. (#319)

### Fixed:
 - SimulationConfig: `spike_threshold` is float (#340)
 - Check more variants of CMake invocations. (#333)
 - Remove `CMAKE_SKIP_INSTALL_ALL_DEPENDENCY`. (#336)
 - add SimulationConfig to `__all__` (#339)
 - Make sure defaults for condition blocks and others are applied (#338)
 - Allow `sonata::sonata_{shared,static}`. (#335)
 - Update `pybind11==2.11.1`. (#332)
 - Refactor `edge_index::resolve`. (#314)
 - Refactor `_readSelection`. (#315)
 - Prevent pytest from picking up pybind11 tests. (#320)
 - update to require using cmake 3.16 (#321)
 - test for completely empty datasets (#281)
 - Update HighFive to v2.8.0. (#318)
 - be neighborly, do not use `using` in headers (#316)
 - work around brew erroring in github actions (#317)
 - Move `Selection` to its own header. (#306)
 - Simplify HighFive usage. (#304)
 - Simplify single group check. (#303)
 - Optimize loading `?fferent_edges`. (#298)
 - Allow disabling coverage an 'Debug'. (#299)
 - Simplify `Selection::Selection`. (#300)
 - Use `RawIndex = std::vector<std::array<.., 2>>`. (#294)

## v0.1.24:
 - small cosmetic fixes/py 3.12 wheels (#290)
 - Fix toJSON incorrect empty, node_id and compound problems (#289)
 - NodeSet `update`: merge NodeSet objects together (#283)

## v0.1.23:
 - Create non-tuple Spike Report access (#285)
 - Add 'lfp' report type and 'electrodes_file' field (#271)
 - Add missing reversal property of shotnoise family (#284)
 - Move std::vector methods above scalar methods in bindings (#280)

## v0.1.22:
 - bump HighFive to v2.7.1 (#278)
 - Extend metadata type to be int, float, bool and string (#277)
 - Prevent node_set materialization for `@libary` with integers (#276)

## v0.1.21:
 - SimulationConfig: set the default value of random_seed in inputs to None (#270)
 - Allow empty arrays in nodesets (#272)
 - implement spec for population properties for vasculature, astrocytes and endfeet (#275)

## v0.1.19:
 - switch to having `connection_overrides` being a list (#263)
 - expose circuit config status (#264)
 - Don't read the full node sets file before parsing (#261)

## v0.1.18:
 - speed up nested compound nodesets

## v0.1.17:

### Added:
 - Allow for partial circuit configs (#241)
 - Added spatial index accessors for segment and synapse indices (#250)

### Fixed:
 - Add include(Catch) to CMakeLists.txt. (#248)
 - Enable setting the C++ standard. (#247)
 - SONATA sim config updates (#245)
 - make `chemical` the default connection type (#242)
 - add check for duplicate populations (#239)
 - refactor python tests so they are not in a monolithic file (#238)
 - Fix `get_population_names` in README.rst (#235)
 - update github actions

## v0.1.16:

### Fixed:
 - Fix #226 for empty selections (#232)

## v0.1.15:

### Added:
 - Parse `metadata` and `beta_features` sections in SimulationConfig (#224)
 - SimulationConfig parser: add modifications properties in conditions section (#228)
 - make nodes/edges paths files available through PopulationProperties (#230)

### Fixed:
 - Updates on the SimulationConfig parser (#227)
 - don't make many small reads via HighFive/hdf5 (#226)
 - Bump pybind11 from v2.5.0 to v2.10.0; supports python 3.11 (#223)
 - fix report files not respecting the `output` path (#222)

## v0.1.14:

### Fixed:
 - Parse synapse mod variables in conditions/mechanisms section (#217)

## v0.1.13:

### Added:
 - Parse SimulationConfig #197, #203, #207, #208, #210, others

### Fixed:
 - throw if biophysical_neuron_models_dir is missing for biophysical population (#206)
 - for `biophysical` node populations, don't raise error if alternate_morphologies exists (#209)

## v0.1.12:

### Added:
 - Add a check when feeding a directory path to readFile (#188)
 - Add support for IO-blocks (#183)

### Fixed:
 - Bump HighFive to v2.4.0 (#190); fixes errors on archs: aarch64, s390x and ppc64le
 - Update hdf5 to 1.12.1 in linux wheel building (#191)
 - Add `-Wp,-D_GLIBCXX_ASSERTIONS` to more likely find out of bounds memory access (#182)
 - Improve memory operations + NodeID-ElementID mapping bindings (#179)

## v0.1.11:

### Fixed:
 - Prefer building Release mode for python wheels, can control with SONATA_BUILD_TYPE (#176)
 - Only do a single read when getting a report frame (#174)

## v0.1.10:

### Added:
 - Have the shim module document functionality (#162)
 - added simulation config implementation (#158)
 - reporting getNodeIdElementIdMapping function(#168)
 - getTimes inside SpikeReader::Population (#169)

### Fixed:
 - move to C++14 (#159)
 - Exclude musllinux wheels building
 - Update HighFive submodule to latest 2.3.1 (#156)
 - update to new pybind11::mkdocs method (#173)
 - switch away from nose for test discovery (#172)

## v0.1.9: Thu 01 Jul 2021

### Added:
 - circuit_config.json implementation (#142)
 - allow for simple expressions (#147) in node_sets as proposed to the
   offical SONATA spec in https://github.com/AllenInstitute/sonata/pull/129

### Fixed:
 - upgrade Catch2 to 2.13.6; autodetect test names (#148)
 - move to fmt 7.1.2 (#145)

## v0.1.8: Wed 10 Feb 2021

### Fixed:
 - moved extlib/nlohmann/json.hpp one level deaper so that building with
   non-versioned nlohmann json is easier

## v0.1.7: Thu Jan 21 2021

### Added:
 - Implemented `node_sets` functionality (#107)
 - Handle pathlib.Path for paths (#113)
 - Add __repr__ to python Selection binding, aswell as the (Node|Edge)Population (#114)

### Fixed:
 - Fix python bindings for Selection with negative values (#112)
 - Fix regression from move to GitHub actions

## v0.1.4 Wed Jul 22 2020

 - Use common docstrings between C++ and Python
 - Better documentation
 - Report readers documentation
 - Bump HighFive and pybind11
 - Numpy arrays for the report outputs

### Fixed:

 - Fixed python sdist distribution by disabling test requirements
 - Fix report_reader to have several time the same index

## v0.1.3: Fri Mar 13 2020

### Added:

 - convenience method to create a Selection of all members of a Population (#60)
 - make bbp::sonata::Selection comparable (#62)
 - comparison for node attributes to values (#64)
 - intersection and union for selections (#67)
 - output reader for spikes, somas and elements (#71)

### Fixed:

 - Mac python wheels (#77)

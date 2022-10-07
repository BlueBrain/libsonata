# Changelog

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

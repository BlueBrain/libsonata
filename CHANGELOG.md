# Changelog

## v0.1.9: Thu 01 Jul 2021

### Added:
    * circuit_config.json implementation (#142)
    * allow for simple expressions (#147) in node_sets as proposed to the 
      offical SONATA spec in https://github.com/AllenInstitute/sonata/pull/129

### Fixed:
    * upgrade Catch2 to 2.13.6; autodetect test names (#148)
    * move to fmt 7.1.2 (#145)

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

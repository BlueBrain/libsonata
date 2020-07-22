# Changelog

## Development: v0.1.5

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

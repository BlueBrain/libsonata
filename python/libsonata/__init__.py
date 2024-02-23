"""Python reader for SONATA circuit files
"""
# Needed so that mac wheels properly delocate:
#  https://github.com/matthew-brett/delocate/issues/22

from libsonata._libsonata import (
    CircuitConfig,
    CircuitConfigStatus,
    SimulationConfig,
    EdgePopulation,
    EdgeStorage,
    ElementDataFrame,
    ElementReportPopulation,
    ElementReportReader,
    NodePopulation,
    NodeSets,
    NodeStorage,
    Selection,
    SomaDataFrame,
    SomaReportPopulation,
    SomaReportReader,
    SonataError,
    SpikePopulation,
    SpikeReader,
    version,
    Hdf5Reader,
)


__all__ = [
    "CircuitConfig",
    "CircuitConfigStatus",
    "SimulationConfig",
    "EdgePopulation",
    "EdgeStorage",
    "ElementDataFrame",
    "ElementReportPopulation",
    "ElementReportReader",
    "NodePopulation",
    "NodeSets",
    "NodeStorage",
    "Selection",
    "SomaDataFrame",
    "SomaReportPopulation",
    "SomaReportReader",
    "SonataError",
    "SpikePopulation",
    "SpikeReader",
    "version",
    "Hdf5Reader",
]

def make_collective_reader(comm, collective_metadata, collective_transfer):
    """Create an Hdf5Reader for collective I/O.

    If MPI support is available via `libsonata_mpi`, then it will return
    an Hdf5Reader for collective I/O on the given communicator.

    This requires that the application uses libsonata in a collective way, e.g.
    all ranks must read the same attributes in the same order.

    If `libsonata_mpi` hasn't been installed, the function returns the default
    Hdf5Reader.
    """
    try:
        import libsonata_mpi
        return libsonata_mpi.make_collective_reader(comm, collective_metadata, collective_transfer)

    except ModuleNotFoundError:
        return Hdf5Reader()

C++ / Python reader for SONATA circuit files:
https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md

# Installation

## Building the C++ library

```
git clone git@github.com:BlueBrain/libsonata.git --recursive
cd libsonata
mkdir build && cd build
cmake  -DCMAKE_BUILD_TYPE=Release  -DEXTLIB_FROM_SUBMODULES=ON ..
make -j
```

## Installing as a Python package

```
git clone git@github.com:BlueBrain/libsonata.git --recursive
cd libsonata
pip install .
```


# Usage (Python)

## Nodes

### NodeStorage

```
>> from libsonata import NodeStorage

>> nodes = NodeStorage(<path to H5 file>)

# list populations
>> nodes.population_names

# open population
>> population = nodes.open_population(<name>)
```

### NodePopulation

```
# total number of nodes in the population
>> population.size

# attribute names
>> population.attribute_names

# get attribute value for single node
>> population.get_attribute('mtype', 42)

# ...or Selection of nodes (see below) => returns NumPy array with corresponding values
>> population.get_attribute('mtype', selection)
```

### Selection

List of element IDs where adjacent IDs are grouped for the sake of efficient HDF5 file access.
For instance, `{1, 2, 3, 5}` sequence becomes `{[1, 4), [5, 6)}`.

`Selection` can be instantiated from:
 - a sequence of scalar values (works for NumPy arrays as well)
 - a sequence of pairs (interpreted as ranges above, works for N x 2 NumPy arrays as well)

`EdgePopulation` connectivity queries (see below) return `Selection`s as well.

```
>> import numpy as np
>> from libsonata import Selection

>> selection = Selection(np.asarray([1, 2, 3, 5]))
>> selection.ranges
[(1, 4), (5, 6)]
```

```
>> selection = Selection([(1, 4), (5, 6)])
>> selection.flatten()
[1, 2, 3, 5]
>> selection.flat_size
4
>> bool(selection)
True
```

```
>> selection = Selection([])
>> selection.ranges
[]
>> selection.flatten()
[]
>> selection.flat_size
0
>> bool(selection)
False
```


## Edges

### EdgeStorage

Analogous to `NodeStorage`.

```
>> from libsonata import EdgeStorage

>> edges = EdgeStorage(<path to H5 file>)

# list populations
>> edges.population_names

# open population
>> population = edges.open_population(<name>)
```

### EdgePopulation

Analogous to `NodePopulation`...

```
# total number of edges in the population
>> population.size

# attribute names
>> population.attribute_names

# get attribute value for single edge
>> population.get_attribute('delay', 42)

# ...or Selection of edges => returns NumPy array with corresponding values
>> population.get_attribute('delay', selection)
```

...with additional methods for querying connectivity

```
# source / target node ID(s)
>> population.source_node(42)
>> population.target_node(42)

# ...or their vectorized analogues
>> population.source_nodes([0, 1])
>> population.target_nodes([0, 1])

# ...(works for NumPy arrays as well)
>> import numpy as np
>> population.source_nodes(np.asarray([0, 1]))
>> population.target_nodes(np.asarray([0, 1]))

# query connectivity (result is Selection object)
>> selection = population.afferent_edges(1)
>> selection = population.efferent_edges(1)
>> selection = population.connecting_edges(1, 2)

# ...or their vectorized analogues
>> selection = population.afferent_edges([1, 2, 3])
>> selection = population.efferent_edges([1, 2, 3])
>> selection = population.connecting_edges([1, 2, 3], [4, 5, 6])

# ...(works for NumPy arrays as well)
>> import numpy as np
>> selection = population.afferent_edges(np.asarray([1, 2, 3]))
```

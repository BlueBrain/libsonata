|banner|

|license| |coverage| |docs|

libsonata
=========

C++ / Python reader for SONATA circuit files:
`SONATA guide <https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md>`__

Installation
------------

Installing from PyPI
~~~~~~~~~~~~~~~~~~~~

.. code-block:: shell

   pip install libsonata

Installing as a Python package, directly from GitHub
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: shell

   pip install git+https://github.com/BlueBrain/libsonata

Building the C++ library
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: shell

   git clone git@github.com:BlueBrain/libsonata.git --recursive
   cd libsonata
   mkdir build && cd build
   cmake  -DCMAKE_BUILD_TYPE=Release  -DEXTLIB_FROM_SUBMODULES=ON ..
   make -j

Usage (Python)
--------------

Nodes
~~~~~

NodeStorage
+++++++++++

.. code-block:: pycon

   >>> import libsonata

   >>> nodes = libsonata.NodeStorage('path/to/H5/file')

   # list populations
   >>> nodes.population_names

   # open population
   >>> population = nodes.open_population(<name>)


NodePopulation
++++++++++++++

.. code-block:: pycon

   # total number of nodes in the population
   >>> population.size

   # attribute names
   >>> population.attribute_names

   # get attribute value for single node, say 42
   >>> population.get_attribute('mtype', 42)

   # ...or Selection of nodes (see below) => returns NumPy array with corresponding values
   >>> selection = libsonata.Selection(values=[1, 5, 9, 42])  # nodes 1, 5, 9, 42
   >>> mtypes = population.get_attribute('mtype', selection)
   >>> list(zip(selection.flatten(), mtypes))
   [(1, u'mtype_of_1'), (5, u'mtype_of_5'), (9, u'mtype_of_9'), (42, u'mtype_of_42')]


Selection
+++++++++

List of element IDs (either `node_id`, or `edge_id`) where adjacent IDs are grouped for the sake of efficient HDF5 file access.
For instance, `{1, 2, 3, 5}` sequence becomes `{[1, 4), [5, 6)}`.

`Selection` can be instantiated from:
 - a sequence of scalar values (works for NumPy arrays as well)
 - a sequence of pairs (interpreted as ranges above, works for N x 2 NumPy arrays as well)

`EdgePopulation` connectivity queries (see below) return ``Selection``\ s as well.

.. code-block:: pycon

   >>> selection = libsonata.Selection([1, 2, 3, 5])
   >>> selection.ranges
   [(1, 4), (5, 6)]


.. code-block:: pycon

   >>> selection = libsonata.Selection([(1, 4), (5, 6)])
   >>> selection.flatten()
   [1, 2, 3, 5]
   >>> selection.flat_size
   4
   >>> bool(selection)
   True


Edges
~~~~~

EdgeStorage
+++++++++++

Population handling for `EdgeStorage` is analogous to `NodeStorage`:

.. code-block:: pycon

   >>> edges = libsonata.EdgeStorage('path/to/H5/file')

   # list populations
   >>> edges.population_names

   # open population
   >>> population = edges.open_population(<name>)


EdgePopulation
++++++++++++++

.. code-block:: pycon

   # total number of edges in the population
   >>> population.size

   # attribute names
   >>> population.attribute_names

   # get attribute value for single edge, say 123
   >>> population.get_attribute('delay', 123)

   # ...or Selection of edges => returns NumPy array with corresponding values
   >>> selection = libsonata.Selection([1, 5, 9])
   >>> population.get_attribute('delay', selection) # returns delays for edges 1, 5, 9


...with additional methods for querying connectivity, where the results are selections that can be applied like above

.. code-block:: pycon

   # get source / target node ID for the 42nd edge:
   >>> population.source_node(42)
   >>> population.target_node(42)

   # query connectivity (result is Selection object)
   >>> selection_to_1 = population.afferent_edges(1)  # all edges with target node_id 1
   >>> population.target_nodes(selection_to_1)  # since selection only contains edges
                                                # targeting node_id 1 the result will be a
                                                # numpy array of all 1's
   >>> selection_from_2 = population.efferent_edges(2)  # all edges sourced from node_id 2
   >>> selection = population.connecting_edges(2, 1)  # this selection is all edges from
                                                      # node_id 2 to node_id 1

   # ...or their vectorized analogues
   >>> selection = population.afferent_edges([1, 2, 3])
   >>> selection = population.efferent_edges([1, 2, 3])
   >>> selection = population.connecting_edges([1, 2, 3], [4, 5, 6])


Acknowledgements
----------------

This project/research has received funding from the European Union’s Horizon 2020 Framework Programme for Research and Innovation under the Specific Grant Agreement No. 785907 (Human Brain Project SGA2).


License
-------

libsonata is distributed under the terms of the GNU Lesser General Public License version 3,
unless noted otherwise, for example, for external dependencies.
Refer to `COPYING.LESSER` and `COPYING` files for details.

Copyright (C) 2018-2020, Blue Brain Project/EPFL and contributors.

libsonata is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License version 3
as published by the Free Software Foundation.

libsonata is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libsonata.  If not, see <https://www.gnu.org/licenses/>.


.. |license| image:: https://img.shields.io/pypi/l/libsonata
                :target: https://github.com/BlueBrain/libsonata/blob/master/COPYING.LESSER

.. |coverage| image:: https://coveralls.io/repos/github/BlueBrain/libsonata/badge.svg
                 :target: https://coveralls.io/github/BlueBrain/libsonata

.. |docs| image:: https://readthedocs.org/projects/libsonata/badge/?version=latest
             :target: https://libsonata.readthedocs.io/
             :alt: documentation status

.. substitutions
.. |banner| image:: docs/source/_images/libSonataLogo.jpg

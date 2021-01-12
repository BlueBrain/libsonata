import os
import pathlib
import unittest

import numpy as np

from libsonata import (EdgeStorage, NodeStorage,
                       Selection, SonataError,
                       SpikeReader, SpikePopulation,
                       SomaReportReader, SomaReportPopulation,
                       ElementReportReader, ElementReportPopulation,
                       NodeSets,
                       )


PATH = os.path.dirname(os.path.realpath(__file__))
PATH = os.path.join(PATH, '../../tests/data')


class TestSelection(unittest.TestCase):
    def test_basic(self):
        ranges = [(3, 5), (0, 3)]
        selection = Selection(ranges)
        self.assertTrue(selection)
        self.assertEqual(selection.ranges, ranges)
        self.assertEqual(selection.flat_size, 5)
        self.assertEqual(selection.flatten().tolist(), [3, 4, 0, 1, 2])

    def test_dtypes(self):
        for dtype in (np.uint, np.uint16, np.uint32, np.uint64, ):
            self.assertEqual(Selection(np.array([1, 0, 1, 2], dtype=dtype)).flatten().tolist(),
                             [1, 0, 1, 2])

        # these work due to forcecast
        for dtype in (np.int, np.int16, np.int32, np.int64, ):
            self.assertEqual(Selection(np.array([1, 0, 1, 2], dtype=dtype)).flatten().tolist(),
                             [1, 0, 1, 2])

        # these fail due to being negative node ids
        self.assertRaises(SonataError, Selection, [-1, 0, 1, 2])
        self.assertRaises(SonataError, Selection, [1, 0, -1, 2])
        Selection(((0, 1), (1, 2)))
        self.assertRaises(SonataError, Selection, ((-2, -1), (1, 2)))

        self.assertRaises(SonataError, Selection, [[-1, 1], [3, 4], [5, 6]])

    def test_from_values(self):
        values = [
            [1, 3, 4, 1],
            [0, 0, 0, 0],
        ]
        expected = [(1, 2), (3, 5), (1, 2)]
        self.assertEqual(Selection(values[0]).ranges, expected)
        self.assertEqual(Selection(np.array(values, dtype=np.uint64, order='C')[0]).ranges, expected)
        self.assertEqual(Selection(np.array(values, dtype=np.uint32, order='C')[0]).ranges, expected)
        self.assertEqual(Selection(np.array(values, dtype=np.uint64, order='F')[0]).ranges, expected)

        self.assertRaises(ValueError, Selection, np.zeros((3, 3, 3), dtype=np.uint64))

        self.assertRaises(SonataError, Selection, ((2, 1), (1, 2)))

    def test_from_ranges(self):
        Selection(((0, 1), (1, 2)))
        self.assertRaises(SonataError, Selection, ((2, 1), (1, 2)))

    def test_from_values_empty(self):
        self.assertFalse(Selection([]))
        self.assertFalse(Selection(np.array([], dtype=np.uint64)))

    def test_comparison(self):
        empty = Selection([])
        range_selection = Selection(((0, 2), (3, 4)))
        values_selection = Selection([1, 3, 4, 1])

        self.assertEqual(empty, empty)
        self.assertNotEqual(empty, range_selection)
        self.assertNotEqual(empty, values_selection)

        self.assertEqual(range_selection, range_selection)
        self.assertNotEqual(range_selection, values_selection)

        self.assertEqual(values_selection, values_selection)

        values_selection1 = Selection([1, 3, 4, 1])
        self.assertEqual(values_selection, values_selection1)

    def test_union_intersection(self):
        empty = Selection([])
        self.assertEqual(empty, empty & empty)
        self.assertEqual(empty, empty | empty)

        even = Selection(list(range(0, 10, 2)))
        odd = Selection(list(range(1, 10, 2)))
        self.assertEqual(empty, empty & even)
        self.assertEqual(even, empty | even)
        self.assertEqual(empty, odd & even)
        self.assertEqual(Selection(list(range(10))), odd | even)


class TestNodePopulation(unittest.TestCase):
    def setUp(self):
        path = os.path.join(PATH, 'nodes1.h5')
        self.test_obj = NodeStorage(path).open_population('nodes-A')

    def test_name(self):
        self.assertEqual(self.test_obj.name, "nodes-A")

    def test_size(self):
        self.assertEqual(self.test_obj.size, 6)
        self.assertEqual(len(self.test_obj), 6)

    def test_attribute_names(self):
        self.assertEqual(
            self.test_obj.attribute_names,
            {
                'attr-X', 'attr-Y', 'attr-Z',
                'A-int8', 'A-int16', 'A-int32', 'A-int64',
                'A-uint8', 'A-uint16', 'A-uint32', 'A-uint64',
                'A-float', 'A-double',
                'A-string',
                'A-enum',
                'E-mapping-good',
                'E-mapping-bad'
            }
        )

    def test_get_attribute(self):
        self.assertEqual(self.test_obj.get_attribute('attr-X', 0), 11.)
        self.assertEqual(self.test_obj.get_attribute('attr-X', Selection([0, 5])).tolist(), [11., 16.])

        # different dtypes
        self.assertEqual(self.test_obj.get_attribute('attr-Y', 0), 21)
        self.assertEqual(self.test_obj.get_attribute('attr-Z', 0), 'aa')

        # default value
        self.assertEqual(self.test_obj.get_attribute('attr-X', Selection([0, 5]), 42.).tolist(), [11., 16.])

        self.assertRaises(SonataError, self.test_obj.get_attribute, 'no-such-attribute', 0)

    def test_get_dynamics_attribute(self):
        self.assertEqual(self.test_obj.get_dynamics_attribute('dparam-X', 0), 1011.)
        self.assertEqual(self.test_obj.get_dynamics_attribute('dparam-X', Selection([0, 5])).tolist(), [1011., 1016.])

        # different dtypes
        self.assertEqual(self.test_obj.get_dynamics_attribute('dparam-Y', 0), 1021)
        self.assertEqual(self.test_obj.get_dynamics_attribute('dparam-Z', 0), 'd-aa')

        # default value
        self.assertEqual(self.test_obj.get_dynamics_attribute('dparam-X', Selection([0, 5]), 42.).tolist(), [1011., 1016.])

        self.assertRaises(SonataError, self.test_obj.get_dynamics_attribute, 'no-such-attribute', 0)

    def test_enumeration_names(self):
        self.assertEqual(
            self.test_obj.enumeration_names,
            {
                'E-mapping-good',
                'E-mapping-bad'
            }
        )

    def test_enumeration_values(self):
        self.assertEqual(
            self.test_obj.get_attribute(
                "E-mapping-good",
                Selection([(0, 1), (2, 3)])
            ).tolist(),
            ["C", "C"]
        )

        self.assertEqual(
            self.test_obj.get_attribute(
                "E-mapping-good",
                Selection([(0, 1), (2, 3)])
            ).tolist(),
            ["C", "C"]
        )

        self.assertEqual(
            self.test_obj.get_enumeration(
                "E-mapping-good",
                Selection([(0, 1), (2, 3)])
            ).tolist(),
            [2, 2]
        )

        self.assertEqual(
            self.test_obj.enumeration_values("E-mapping-good"),
            ["A", "B", "C"]
        )

        self.assertRaises(
            SonataError,
            self.test_obj.get_attribute,
            "E-mapping-bad",
            1
        )

        self.assertRaises(
            SonataError,
            self.test_obj.get_enumeration,
            "attr-X",
            0
        )

    def test_select_all(self):
        self.assertEqual(self.test_obj.select_all().flat_size, 6)

    def test_match_values(self):
        # string
        self.assertEqual(self.test_obj.match_values("attr-Z", "bb").flatten().tolist(),
                         [1])

        # enum
        self.assertEqual(self.test_obj.match_values("E-mapping-good", "C").flatten().tolist(),
                         [0, 2, 4, 5])

        # int
        self.assertEqual(self.test_obj.match_values("attr-Y", 23).flatten().tolist(),
                         [2, ])

        # float
        self.assertRaises(TypeError, self.test_obj.match_values, "attr-Y", 23.)


class TestEdgePopulation(unittest.TestCase):
    def setUp(self):
        path = os.path.join(PATH, 'edges1.h5')
        self.test_obj = EdgeStorage(path).open_population('edges-AB')

    def test_source(self):
        self.assertEqual(self.test_obj.source, 'nodes-A')

    def test_target(self):
        self.assertEqual(self.test_obj.target, 'nodes-B')

    def test_source_nodes(self):
        self.assertEqual(self.test_obj.source_node(1), 1)
        self.assertEqual(self.test_obj.source_nodes(Selection([0, 1, 2, 4])).tolist(), [1, 1, 2, 3])

    def test_target_nodes(self):
        self.assertEqual(self.test_obj.target_node(1), 2)
        self.assertEqual(self.test_obj.target_nodes(Selection([0, 1, 2, 4])).tolist(), [1, 2, 1, 0])

    def test_afferent_edges(self):
        self.assertEqual(self.test_obj.afferent_edges([1, 2]).ranges, [(0, 4), (5, 6)])
        self.assertEqual(self.test_obj.afferent_edges(1).ranges, [(0, 1), (2, 4)])

    def test_efferent_edges(self):
        self.assertEqual(self.test_obj.efferent_edges([1, 2]).ranges, [(0, 4)])
        self.assertEqual(self.test_obj.efferent_edges(0).ranges, [])

    def test_connecting_edges(self):
        self.assertEqual(self.test_obj.connecting_edges([1, 2], [1, 2]).ranges, [(0, 4)])
        self.assertEqual(self.test_obj.connecting_edges(1, 1).ranges, [(0, 1)])

    def test_select_all(self):
        self.assertEqual(self.test_obj.select_all().flat_size, 6)


class TestSpikePopulation(unittest.TestCase):
    def setUp(self):
        path = os.path.join(PATH, "spikes.h5")
        self.test_obj = SpikeReader(path)

    def test_get_all_populations(self):
        self.assertEqual(self.test_obj.get_population_names(), ['All', 'empty', 'spikes1', 'spikes2'])

    def test_get_population(self):
        self.assertTrue(isinstance(self.test_obj['spikes1'], SpikePopulation))

    def test_get_inexistant_population(self):
        self.assertRaises(RuntimeError, self.test_obj.__getitem__, 'foobar')

    def test_get_spikes_from_population(self):
        self.assertEqual(self.test_obj['All'].get(), [(5, 0.1), (2, 0.2), (3, 0.3), (2, 0.7), (3, 1.3)])
        self.assertEqual(self.test_obj['All'].get(tstart=0.2, tstop=1.0), [(2, 0.2), (3, 0.3), (2, 0.7)])
        self.assertEqual(self.test_obj['spikes2'].get(tstart=0.2, tstop=1.0), [(3, 0.3), (2, 0.2), (2, 0.7)])
        self.assertEqual(self.test_obj['spikes1'].get((3,)), [(3, 0.3), (3, 1.3)])
        self.assertEqual(self.test_obj['spikes2'].get((3,)), [(3, 0.3), (3, 1.3)])
        self.assertEqual(self.test_obj['spikes2'].get((10,)), [])
        self.assertEqual(self.test_obj['spikes2'].get((2,), 0., 0.5), [(2, 0.2)])
        self.assertEqual(self.test_obj['spikes1'].get((2, 5)), [(2, 0.2), (2, 0.7), (5, 0.1)])
        self.assertEqual(self.test_obj['spikes2'].get((2, 5)), [(5, 0.1), (2, 0.2), (2, 0.7)])
        self.assertEqual(self.test_obj['All'].sorting, "by_time")
        self.assertEqual(self.test_obj['spikes1'].sorting, "by_id")
        self.assertEqual(self.test_obj['spikes2'].sorting, "none")
        self.assertEqual(self.test_obj['empty'].get(), [])

        self.assertEqual(len(self.test_obj['All'].get(node_ids=[])), 0)

class TestSomaReportPopulation(unittest.TestCase):
    def setUp(self):
        path = os.path.join(PATH, "somas.h5")
        self.test_obj = SomaReportReader(path)

    def test_get_all_population(self):
        self.assertEqual(self.test_obj.get_population_names(), ['All', 'soma1', 'soma2'])

    def test_get_population(self):
        self.assertTrue(isinstance(self.test_obj['All'], SomaReportPopulation))

    def test_get_inexistant_population(self):
        self.assertRaises(RuntimeError, self.test_obj.__getitem__, 'foobar')

    def test_get_reports_from_population(self):
        self.assertEqual(self.test_obj['All'].times, (0., 1., 0.1))
        self.assertEqual(self.test_obj['All'].time_units, 'ms')
        self.assertEqual(self.test_obj['All'].data_units, 'mV')
        self.assertFalse(self.test_obj['All'].sorted)
        self.assertEqual(len(self.test_obj['All'].get().ids), 20)  # Number of nodes
        self.assertEqual(len(self.test_obj['All'].get().times), 10)  # number of times
        self.assertEqual(len(self.test_obj['All'].get().data), 10)  # should be the same

        sel = self.test_obj['All'].get(node_ids=[13, 14], tstart=0.8, tstop=1.0)
        self.assertEqual(len(sel.times), 2)  # Number of timestamp (0.8 and 0.9)
        self.assertEqual(list(sel.ids), [13, 14])
        np.testing.assert_allclose(sel.data, [[13.8, 14.8], [13.9, 14.9]])

        sel_all = self.test_obj['All'].get()
        self.assertEqual(sel_all.ids, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20])

        sel_empty = self.test_obj['All'].get(node_ids=[])
        np.testing.assert_allclose(sel_empty.data, np.empty(shape=(0, 0)))


class TestElementReportPopulation(unittest.TestCase):
    def setUp(self):
        path = os.path.join(PATH, "elements.h5")
        self.test_obj = ElementReportReader(path)

    def test_get_all_population(self):
        self.assertEqual(self.test_obj.get_population_names(), ['All', 'element1', 'element42'])

    def test_get_population(self):
        self.assertTrue(isinstance(self.test_obj['All'], ElementReportPopulation))

    def test_get_inexistant_population(self):
        self.assertRaises(RuntimeError, self.test_obj.__getitem__, 'foobar')

    def test_get_node_ids(self):
        self.assertEqual(self.test_obj['All'].get_node_ids(), [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20])

    def test_get_reports_from_population(self):
        self.assertEqual(self.test_obj['All'].times, (0., 4., 0.2))
        # check following calls succeed (no memory destroyed)
        self.assertEqual(self.test_obj['All'].times, (0., 4., 0.2))

        self.assertEqual(self.test_obj['All'].time_units, 'ms')
        self.assertEqual(self.test_obj['All'].data_units, 'mV')
        self.assertTrue(self.test_obj['All'].sorted)
        self.assertEqual(len(self.test_obj['All'].get(tstride=2).data), 10)  # Number of times in this range
        self.assertEqual(len(self.test_obj['All'].get(tstride=2).times), 10)  # Should be the same
        self.assertEqual(len(self.test_obj['All'].get().ids), 100)
        sel = self.test_obj['All'].get(node_ids=[13, 14], tstart=0.8, tstop=1.2)
        keys = list(sel.ids)
        keys.sort()
        self.assertEqual(keys, [(13, 30), (13, 30), (13, 31), (13, 31), (13, 32), (14, 32), (14, 33), (14, 33), (14, 34), (14, 34)])

        self.assertEqual(len(self.test_obj['All'].get(node_ids=[]).data), 0)
        self.assertEqual(len(self.test_obj['All'].get(node_ids=[]).times), 0)
        self.assertEqual(len(self.test_obj['All'].get(node_ids=[]).ids), 0)

        self.assertEqual(len(sel.times), 3)  # Number of timestamp (0.8, 1.0 and 1.2)
        with self.assertRaises(SonataError):
            self.test_obj['All'].get(tstart=5.)  # tstart out of range

        # tstart should be <= tstop
        np.testing.assert_allclose(self.test_obj['All'].get(node_ids=[1, 2], tstart=3., tstop=3.).data[0],
                                   [150.0, 150.1, 150.2, 150.3, 150.4, 150.5, 150.6, 150.7, 150.8, 150.9])
        # check following calls succeed (no memory destroyed)
        np.testing.assert_allclose(self.test_obj['All'].get(node_ids=[1, 2], tstart=3., tstop=3.).data[0],
                                   [150.0, 150.1, 150.2, 150.3, 150.4, 150.5, 150.6, 150.7, 150.8, 150.9])
        np.testing.assert_allclose(self.test_obj['All'].get(node_ids=[3, 4], tstart=0.2, tstop=0.4).data[0],
                                   [11.0, 11.1, 11.2, 11.3, 11.4, 11.5, 11.6, 11.7, 11.8, 11.9], 1e-6, 0)
        np.testing.assert_allclose(self.test_obj['All'].get(node_ids=[3, 4], tstride=4).data[2],
                                   [81.0, 81.1, 81.2, 81.3, 81.4, 81.5, 81.6, 81.7, 81.8, 81.9], 1e-6, 0)


class TestNodePopulationFailure(unittest.TestCase):
    def test_CorrectStructure(self):
        self.assertRaises(SonataError, NodeSets, "1")
        self.assertRaises(SonataError, NodeSets, "[1]")

    def test_BasicScalarFailFloat(self):
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": { "node_id": 1.5 } }')

    def test_BasicScalarFailNegativeNodeIds(self):
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": { "node_id": -1 } }')
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": { "node_id": [1, -1] } }')

    def test_BasicScalarFailPopulation(self):
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": { "population": 1 } }')
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": { "population": [1, 2] } }')

    def test_FailCompoundWithInt(self):
        self.assertRaises(SonataError, NodeSets, '{"compound": ["foo", 1] }')

    def test_FaileRecursiveCompound(self):
        self.assertRaises(SonataError, NodeSets, '{"compound": ["compound"] }')

    def test_MissingBasic(self):
        self.assertRaises(SonataError, NodeSets, '{"compound": ["missing"] }')


class TestNodePopulationNodeSet(unittest.TestCase):
    def setUp(self):
        self.population = NodeStorage(os.path.join(PATH, 'nodes1.h5')).open_population('nodes-A')

    def test_BasicScalarInt(self):
        sel = NodeSets('{ "NodeSet0": { "attr-Y": 21 } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection(((0, 1),)))

    def test_BasicScalarString(self):
        sel = NodeSets('{ "NodeSet0": { "attr-Z": ["aa", "cc"] } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection(((0, 1), (2, 3))))

    def test_BasicScalarEnum(self):
        sel = NodeSets('{ "NodeSet0": { "E-mapping-good": "C" } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection(((0, 1), (2, 3), (4, 6),)))

    def test_BasicScalarAnded(self):
        j = '''{"NodeSet0": {"E-mapping-good": "C",
                             "attr-Y": [21, 22]
                            }
            }'''
        sel = NodeSets(j).materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection(((0, 1), )))

    def test_BasicScalarNodeId(self):
        sel = NodeSets('{ "NodeSet0": { "node_id": [1, 3, 5] } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection(((1, 2), (3, 4), (5, 6))))

    def test_BasicScalarPopulation(self):
        sel = NodeSets('{ "NodeSet0": { "population": "nodes-A" } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, self.population.select_all())

        sel = NodeSets('{ "NodeSet0": { "population": ["nodes-A",  "FAKE"] } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, self.population.select_all())

        sel = NodeSets('{ "NodeSet0": { "population": "NOT_A_POP" } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection([]))

    def test_NodeSetCompound(self):
        j = '''{"NodeSet0": { "node_id": [1] },
                "NodeSet1": { "node_id": [2] },
                "NodeSetCompound0": ["NodeSet0", "NodeSet1"],
                "NodeSetCompound1": ["NodeSetCompound0", "NodeSet2"],
                "NodeSet2": { "node_id": [3] }
        }'''
        sel = NodeSets(j).materialize("NodeSetCompound1", self.population)
        self.assertEqual(sel, Selection(((1, 4), )))

    def test_NodeSet_toJSON(self):
        j = '''
        {"bio_layer45": {
              "model_type": "biophysical",
              "location": ["layer4", "layer5"]
          },
          "V1_point_prime": {
              "population": "biophysical",
              "model_type": "point",
              "node_id": [1, 2, 3, 5, 7, 9]
          },
          "combined": ["bio_layer45", "V1_point_prime"]
        }'''
        new = NodeSets(j).toJSON()
        ns1 = NodeSets(new)
        self.assertEqual(new, ns1.toJSON())

        ns = NodeSets.from_file(os.path.join(PATH, 'node_sets.json'))
        self.assertEqual(new, ns.toJSON())


def test_path_ctor():
    #  make sure constructors that take file paths can use pathlib.Path
    path = pathlib.Path(PATH)

    NodeStorage(path / 'nodes1.h5')
    EdgeStorage(path / 'edges1.h5')
    SpikeReader(path / 'spikes.h5')
    SomaReportReader(path / 'somas.h5')
    ElementReportReader(path / 'elements.h5')


if __name__ == '__main__':
    unittest.main()

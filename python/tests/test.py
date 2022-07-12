import json
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
                       CircuitConfig, SimulationConfig
                       )

from libsonata._libsonata import Report, Output, Run

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

    def test_getTimes_from_population(self):
        self.assertEqual(self.test_obj['All'].times, (0.1, 1.3))


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
        keys = sel.ids
        keys_ref = np.asarray([13, 14])
        self.assertTrue((keys == keys_ref).all())
        np.testing.assert_allclose(sel.data, [[13.8, 14.8], [13.9, 14.9]])

        sel_all = self.test_obj['All'].get()
        keys_all = sel_all.ids
        keys_all_ref = np.asarray([10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertTrue((keys_all == keys_all_ref).all())

        data_begin = sel_all.data[0][:10]
        data_begin_ref = np.asarray([10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0], dtype=np.float32)
        self.assertTrue((data_begin == data_begin_ref).all())

        data_end = sel_all.data[-1][-10:]
        data_end_ref = np.asarray([20.9, 1.9, 2.9, 3.9, 4.9, 5.9, 6.9, 7.9, 8.9, 9.9], dtype=np.float32)
        self.assertTrue((data_end == data_end_ref).all())

        sel_empty = self.test_obj['All'].get(node_ids=[])
        np.testing.assert_allclose(sel_empty.data, np.empty(shape=(0, 0)))

    def test_get_node_id_element_id_mapping(self):
        ids_mapping = self.test_obj['All'].get_node_id_element_id_mapping([[3, 5]])
        ids_mapping_ref = np.asarray([3, 4])
        self.assertTrue((ids_mapping == ids_mapping_ref).all())

    def test_block_gap_limit(self):
        ids_mapping = self.test_obj['All'].get_node_id_element_id_mapping([[3, 5]], block_gap_limit=4194304) # >= 1 x GPFS block
        ids_mapping_ref = np.asarray([3, 4])
        self.assertTrue((ids_mapping == ids_mapping_ref).all())

        with self.assertRaises(SonataError):
            self.test_obj['All'].get_node_id_element_id_mapping([[3, 5]], block_gap_limit=4194303) # < 1 x GPFS block


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
        keys = sel.ids
        keys_ref = np.asarray([(13, 30), (13, 30), (13, 31), (13, 31), (13, 32), (14, 32), (14, 33), (14, 33), (14, 34), (14, 34)])
        self.assertTrue((keys == keys_ref).all())
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

    def test_get_node_id_element_id_mapping(self):
        ids_mapping = self.test_obj['All'].get_node_id_element_id_mapping([[3, 5]])
        ids_mapping_ref = np.asarray([[3, 5], [3, 5], [3, 6], [3, 6], [3, 7], [4, 7], [4, 8], [4, 8], [4, 9], [4, 9]])
        self.assertTrue((ids_mapping == ids_mapping_ref).all())

    def test_block_gap_limit(self):
        ids_mapping = self.test_obj['All'].get_node_id_element_id_mapping([[3, 5]], block_gap_limit=4194304) # >= 1 x GPFS block
        ids_mapping_ref = np.asarray([[3, 5], [3, 5], [3, 6], [3, 6], [3, 7], [4, 7], [4, 8], [4, 8], [4, 9], [4, 9]])
        self.assertTrue((ids_mapping == ids_mapping_ref).all())

        with self.assertRaises(SonataError):
            self.test_obj['All'].get_node_id_element_id_mapping([[3, 5]], block_gap_limit=4194303) # < 1 x GPFS block


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
          "power_number_test": {
              "numeric_attribute_gt": { "$gt": 3 },
              "numeric_attribute_lt": { "$lt": 3 },
              "numeric_attribute_gte": { "$gte": 3 },
              "numeric_attribute_lte": { "$lte": 3 }
          },
          "power_regex_test": {
              "string_attr": { "$regex": "^[s][o]me value$" }
          },
          "combined": ["bio_layer45", "V1_point_prime"]
        }'''
        new = NodeSets(j).toJSON()
        ns1 = NodeSets(new)
        self.assertEqual(new, ns1.toJSON())

        ns = NodeSets.from_file(os.path.join(PATH, 'node_sets.json'))
        self.assertEqual(new, ns.toJSON())


class TestMisc(unittest.TestCase):
    def test_path_ctor(self):
        #  make sure constructors that take file paths can use pathlib.Path
        path = pathlib.Path(PATH)

        NodeStorage(path / 'nodes1.h5')
        EdgeStorage(path / 'edges1.h5')
        SpikeReader(path / 'spikes.h5')
        SomaReportReader(path / 'somas.h5')
        ElementReportReader(path / 'elements.h5')
        NodeSets.from_file(path / 'node_sets.json')
        CircuitConfig.from_file(path / 'config/circuit_config.json')


class TestCircuitConfig(unittest.TestCase):
    def setUp(self):
        self.config = CircuitConfig.from_file(os.path.join(PATH, 'config/circuit_config.json'))

    def test_basic(self):
        self.assertEqual(self.config.node_sets_path,
                         os.path.abspath(os.path.join(PATH, 'config/node_sets.json')))

        self.assertEqual(self.config.node_populations,
                         {'nodes-A', 'nodes-B'})
        self.assertEqual(self.config.node_population('nodes-A').name, 'nodes-A')

        self.assertEqual(self.config.edge_populations,
                         {'edges-AB', 'edges-AC'})
        self.assertEqual(self.config.edge_population('edges-AB').name, 'edges-AB')

    def test_expanded_json(self):
        config = json.loads(self.config.expanded_json)
        self.assertEqual(config['components']['biophysical_neuron_models_dir'],
                         'biophysical_neuron_models')
        self.assertEqual(config['networks']['nodes'][0]['node_types_file'],
                         None)
        self.assertEqual(config['networks']['nodes'][0]['nodes_file'],
                         '../nodes1.h5')

    def test_get_population_properties(self):
        node_prop = self.config.node_population_properties('nodes-A')
        self.assertEqual(node_prop.type, 'biophysical')
        self.assertTrue(node_prop.morphologies_dir.endswith('morphologies'))
        self.assertTrue(node_prop.biophysical_neuron_models_dir.endswith('biophysical_neuron_models'))
        self.assertEqual(node_prop.alternate_morphology_formats, {})

        edge_prop = self.config.edge_population_properties('edges-AC')
        self.assertEqual(edge_prop.type, 'chemical_synapse')
        self.assertTrue(edge_prop.morphologies_dir.endswith('morphologies'))
        self.assertTrue(edge_prop.biophysical_neuron_models_dir.endswith('biophysical_neuron_models'))
        self.assertEqual(edge_prop.alternate_morphology_formats, {})


    def test_biophysical_properties_raises(self):
        contents = {
            "manifest": {
                "$BASE_DIR": "./",
                },
            "components": {
                "morphologies_dir": "some/morph/dir",
            },
            "networks": {
                "nodes": [
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": {
                            "nodes-A": { }  # nothing overridden;
                                            # missing biophysical_neuron_models_dir
                        }
                    }],
                "edges": []
                }
            }
        self.assertRaises(SonataError, CircuitConfig, json.dumps(contents), PATH)


    def test_shadowing_morphs(self):
        contents = {
            "manifest": {
                "$BASE_DIR": "./",
                },
            "components": {
                "biophysical_neuron_models_dir": "/biophysical_neuron_models",
                "alternate_morphologies": {
                    "h5v1": "/morphologies/h5"
                    }
                },
            "networks": {
                "nodes": [
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": {
                            "nodes-A": {
                                "morphologies_dir": "/my/custom/morphologies/dir"
                                }
                            }
                        }
                    ],
                "edges": []
                }
            }
        cc = CircuitConfig(json.dumps(contents), PATH)
        pp = cc.node_population_properties('nodes-A')
        assert pp.alternate_morphology_formats == {'h5v1': '/morphologies/h5'}
        assert pp.biophysical_neuron_models_dir == "/biophysical_neuron_models"
        assert pp.morphologies_dir == "/my/custom/morphologies/dir"
        assert pp.alternate_morphology_formats == {'h5v1': '/morphologies/h5'}

        contents = {
            "manifest": {
                "$BASE_DIR": "./",
                },
            "networks": {
                "nodes": [
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": {
                            "nodes-A": {
                                "biophysical_neuron_models_dir": "/some/dir",
                                "morphologies_dir": "/my/custom/morphologies/dir"
                                }
                            }
                        }
                    ],
                "edges": []
                }
            }
        cc = CircuitConfig(json.dumps(contents), PATH)
        pp = cc.node_population_properties('nodes-A')
        assert pp.morphologies_dir == "/my/custom/morphologies/dir"

        contents = {
            "manifest": {
                "$BASE_DIR": "./",
                },
            "networks": {
                "nodes": [
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": {
                            "nodes-A": {
                                "type": "biophysical",
                                "biophysical_neuron_models_dir": "/some/dir",
                                "alternate_morphologies": {
                                    "h5v1": "/my/custom/morphologies/dir",
                                    "neurolucida-asc": "/my/custom/morphologies/dir"
                                    }
                                }
                            }
                        }
                    ],
                "edges": []
                }
            }
        cc = CircuitConfig(json.dumps(contents), PATH)
        pp = cc.node_population_properties('nodes-A')
        assert pp.morphologies_dir == ""
        assert pp.alternate_morphology_formats == {
            'neurolucida-asc': '/my/custom/morphologies/dir',
            'h5v1': '/my/custom/morphologies/dir',
            }


class TestSimulationConfig(unittest.TestCase):
    def setUp(self):
        self.config = SimulationConfig.from_file(
                        os.path.join(PATH, 'config', 'simulation_config.json'))

    def test_basic(self):
        self.assertEqual(self.config.base_path, os.path.abspath(os.path.join(PATH, 'config')))

        self.assertEqual(self.config.run.tstop, 1000)
        self.assertEqual(self.config.run.dt, 0.025)
        self.assertEqual(self.config.run.random_seed, 201506)
        self.assertEqual(self.config.run.spike_threshold, -30)
        self.assertEqual(self.config.run.spike_location, Run.SpikeLocation.AIS)
        self.assertEqual(self.config.run.integration_method, Run.IntegrationMethod.nicholson_ion)
        self.assertEqual(self.config.run.forward_skip, 500)

        self.assertEqual(self.config.output.output_dir,
                         os.path.abspath(os.path.join(PATH, 'config/some/path/output')))
        self.assertEqual(self.config.output.spikes_file, 'out.h5')
        self.assertEqual(self.config.output.log_file, '')
        self.assertEqual(self.config.output.spikes_sort_order, Output.SpikesSortOrder.by_id)

        self.assertEqual(self.config.conditions.celsius, 35.0)
        self.assertEqual(self.config.conditions.v_init, -80)
        self.assertEqual(self.config.conditions.synapses_init_depleted, False)
        self.assertEqual(self.config.conditions.extracellular_calcium, None)
        self.assertEqual(self.config.conditions.minis_single_vesicle, False)
        self.assertEqual(self.config.conditions.randomize_gaba_rise_time, False)
        self.assertEqual(self.config.conditions.mechanisms, {'ProbAMPANMDA_EMS': {'property2': -1,
                                                                                  'property1': False},
                                                             'GluSynapse': {'property4': 'test',
                                                                            'property3': 0.025}})

        self.assertEqual(self.config.list_report_names,
                         { "axonal_comp_centers", "cell_imembrane", "compartment", "soma" })

        self.assertEqual(self.config.report('soma').cells, 'Column')
        self.assertEqual(self.config.report('soma').type, Report.Type.compartment)
        self.assertEqual(self.config.report('soma').compartments, Report.Compartments.center)
        self.assertEqual(self.config.report('soma').enabled, True)
        self.assertEqual(self.config.report('compartment').dt, 0.1)
        self.assertEqual(self.config.report('compartment').sections, Report.Sections.all)
        self.assertEqual(self.config.report('compartment').compartments, Report.Compartments.all)
        self.assertEqual(self.config.report('compartment').enabled, False)
        self.assertEqual(self.config.report('axonal_comp_centers').start_time, 0)
        self.assertEqual(self.config.report('axonal_comp_centers').compartments, Report.Compartments.center)
        self.assertEqual(self.config.report('axonal_comp_centers').scaling, Report.Scaling.none)
        self.assertEqual(self.config.report('axonal_comp_centers').file_name,
                         os.path.abspath(os.path.join(PATH, 'config/axon_centers.h5')))
        self.assertEqual(self.config.report('cell_imembrane').end_time, 500)
        self.assertEqual(self.config.report('cell_imembrane').type.name, 'summation')
        self.assertEqual(self.config.report('cell_imembrane').variable_name, 'i_membrane, IClamp')

        self.assertEqual(self.config.network,
                         os.path.abspath(os.path.join(PATH, 'config/circuit_config.json')))
        self.assertEqual(self.config.target_simulator.name, 'CORENEURON');
        circuit_conf = CircuitConfig.from_file(self.config.network);
        self.assertEqual(self.config.node_sets_file, circuit_conf.node_sets_path);
        self.assertEqual(self.config.node_set, 'Column');

        self.assertEqual(self.config.list_input_names,
                         {"ex_abs_shotnoise",
                          "ex_extracellular_stimulation",
                          "ex_hyperpolarizing",
                          "ex_linear",
                          "ex_noise_mean",
                          "ex_noise_meanpercent",
                          "ex_OU",
                          "ex_pulse",
                          "ex_rel_linear",
                          "ex_rel_OU",
                          "ex_rel_shotnoise",
                          "ex_replay",
                          "ex_seclamp",
                          "ex_shotnoise",
                          "ex_subthreshold"
                          })

        self.assertEqual(self.config.input('ex_linear').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_linear').module.name, 'linear')
        self.assertEqual(self.config.input('ex_linear').delay, 0)
        self.assertEqual(self.config.input('ex_linear').duration, 15)
        self.assertEqual(self.config.input('ex_linear').node_set, "Column")
        self.assertEqual(self.config.input('ex_linear').amp_start, 0.15)
        self.assertEqual(self.config.input('ex_linear').amp_end, 0.15)

        self.assertEqual(self.config.input('ex_rel_linear').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_rel_linear').module.name, 'relative_linear')
        self.assertEqual(self.config.input('ex_rel_linear').delay, 0)
        self.assertEqual(self.config.input('ex_rel_linear').duration, 1000)
        self.assertEqual(self.config.input('ex_rel_linear').node_set, "Column")
        self.assertEqual(self.config.input('ex_rel_linear').percent_start, 80)
        self.assertEqual(self.config.input('ex_rel_linear').percent_end, 20)

        self.assertEqual(self.config.input('ex_pulse').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_pulse').module.name, 'pulse')
        self.assertEqual(self.config.input('ex_pulse').delay, 10)
        self.assertEqual(self.config.input('ex_pulse').duration, 80)
        self.assertEqual(self.config.input('ex_pulse').node_set, "Mosaic")
        self.assertEqual(self.config.input('ex_pulse').width, 1)
        self.assertEqual(self.config.input('ex_pulse').frequency, 80)

        self.assertEqual(self.config.input('ex_noise_meanpercent').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_noise_meanpercent').module.name, 'noise')
        self.assertEqual(self.config.input('ex_noise_meanpercent').delay, 0)
        self.assertEqual(self.config.input('ex_noise_meanpercent').duration, 5000)
        self.assertEqual(self.config.input('ex_noise_meanpercent').node_set, "Rt_RC")

        self.assertEqual(self.config.input('ex_subthreshold').percent_less, 80)
        self.assertEqual(self.config.input('ex_shotnoise').rise_time, 0.4)
        self.assertEqual(self.config.input('ex_shotnoise').amp_mean, 70)

        self.assertEqual(self.config.input('ex_hyperpolarizing').duration, 1000)

        self.assertEqual(self.config.input('ex_noise_meanpercent').mean_percent, 0.01)
        self.assertEqual(self.config.input('ex_noise_meanpercent').mean, None)

        self.assertEqual(self.config.input('ex_noise_mean').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_noise_mean').module.name, 'noise')
        self.assertEqual(self.config.input('ex_noise_mean').delay, 0)
        self.assertEqual(self.config.input('ex_noise_mean').duration, 5000)
        self.assertEqual(self.config.input('ex_noise_mean').node_set, "Rt_RC")
        self.assertEqual(self.config.input('ex_noise_mean').mean, 0)
        self.assertEqual(self.config.input('ex_noise_mean').mean_percent, None)

        self.assertEqual(self.config.input('ex_rel_shotnoise').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_rel_shotnoise').module.name, 'relative_shot_noise')
        self.assertEqual(self.config.input('ex_rel_shotnoise').delay, 0)
        self.assertEqual(self.config.input('ex_rel_shotnoise').duration, 1000)
        self.assertEqual(self.config.input('ex_rel_shotnoise').node_set, "L5E")
        self.assertEqual(self.config.input('ex_rel_shotnoise').random_seed, self.config.run.random_seed)
        self.assertEqual(self.config.input('ex_rel_shotnoise').dt, 0.25)

        self.assertEqual(self.config.input('ex_replay').input_type.name, 'spikes')
        self.assertEqual(self.config.input('ex_replay').module.name, 'synapse_replay')
        self.assertEqual(self.config.input('ex_replay').delay, 0)
        self.assertEqual(self.config.input('ex_replay').duration, 40000)
        self.assertEqual(self.config.input('ex_replay').node_set, "Column")
        self.assertEqual(self.config.input('ex_replay').spike_file,
                         os.path.abspath(os.path.join(PATH, 'config/replay.dat')))
        self.assertEqual(self.config.input('ex_replay').source, "ML_afferents")
        self.assertEqual(self.config.input('ex_extracellular_stimulation').node_set, 'Column')

        self.assertEqual(self.config.input('ex_abs_shotnoise').input_type.name, "conductance")
        self.assertEqual(self.config.input('ex_abs_shotnoise').amp_cv, 0.63)
        self.assertEqual(self.config.input('ex_abs_shotnoise').mean, 50)
        self.assertEqual(self.config.input('ex_abs_shotnoise').sigma, 5)

        self.assertEqual(self.config.input('ex_OU').module.name, "ornstein_uhlenbeck")
        self.assertEqual(self.config.input('ex_OU').input_type.name, "conductance")
        self.assertEqual(self.config.input('ex_OU').tau, 2.8)
        self.assertEqual(self.config.input('ex_OU').reversal, 10)
        self.assertEqual(self.config.input('ex_OU').mean, 50)
        self.assertEqual(self.config.input('ex_OU').sigma, 5)

        self.assertEqual(self.config.input('ex_rel_OU').input_type.name, "current_clamp")
        self.assertEqual(self.config.input('ex_rel_OU').tau, 2.8)
        self.assertEqual(self.config.input('ex_rel_OU').reversal, 0)
        self.assertEqual(self.config.input('ex_rel_OU').mean_percent, 70)
        self.assertEqual(self.config.input('ex_rel_OU').sd_percent, 10)

        self.assertEqual(self.config.list_connection_override_names, {"ConL3Exc-Uni", "GABAB_erev"})
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').source, 'Excitatory')
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').target, 'Mosaic')
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').weight, 1)
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').spont_minis, 0.01)
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').modoverride, 'GluSynapse')
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').delay, 0.5)
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').synapse_delay_override, None)
        self.assertEqual(self.config.connection_override('ConL3Exc-Uni').synapse_configure, None)
        self.assertEqual(self.config.connection_override('GABAB_erev').spont_minis, None)
        self.assertEqual(self.config.connection_override('GABAB_erev').synapse_delay_override, 0.5)
        self.assertEqual(self.config.connection_override('GABAB_erev').delay, 0)
        self.assertEqual(self.config.connection_override('GABAB_erev').modoverride, None)
        self.assertEqual(self.config.connection_override('GABAB_erev').synapse_configure, '%s.e_GABAA = -82.0 tau_d_GABAB_ProbGABAAB_EMS = 77')

    def test_expanded_json(self):
        config = json.loads(self.config.expanded_json)
        self.assertEqual(config['output']['output_dir'], 'some/path/output')


if __name__ == '__main__':
    unittest.main()

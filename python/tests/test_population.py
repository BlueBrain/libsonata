import os
import pathlib
import unittest

import numpy as np

from libsonata import (CircuitConfig,
                       ElementReportReader,
                       NodeSets,
                       NodeStorage,
                       Selection,
                       SimulationConfig,
                       SomaReportReader,
                       SonataError,
                       SpikeReader,
                       EdgeStorage,
                       )


PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                    '../../tests/data')


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
        for dtype in (int, np.int16, np.int32, np.int64, ):
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
                Selection([])
            ).tolist(),
            []
        )

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
        self.assertEqual(self.test_obj.source_nodes(Selection([])).tolist(), [])
        self.assertEqual(self.test_obj.source_node(1), 1)
        self.assertEqual(self.test_obj.source_nodes(Selection([0, 1, 2, 4])).tolist(), [1, 1, 2, 3])

    def test_target_nodes(self):
        self.assertEqual(self.test_obj.target_node(1), 2)
        self.assertEqual(self.test_obj.target_nodes(Selection([0, 1, 2, 4])).tolist(), [1, 2, 1, 0])
        self.assertEqual(self.test_obj.target_nodes(Selection([])).tolist(), [])

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

    def test_library_enumeration(self):
        self.assertEqual(
            self.test_obj.get_attribute("E-mapping-good", Selection([(0, 1), (2, 3)])).tolist(),
            ["C", "C"]
        )

        self.assertEqual(
            self.test_obj.get_enumeration( "E-mapping-good", Selection([(0, 1), (2, 3)])).tolist(),
            [2, 2]
        )


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
        SimulationConfig.from_file(path / 'config/simulation_config.json')

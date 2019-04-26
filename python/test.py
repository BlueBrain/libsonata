import os
import unittest

from libsonata import *

PATH = os.path.dirname(os.path.realpath(__file__))
PATH = os.path.join(PATH, '../tests/data')

class TestSelection(unittest.TestCase):
    def test_basic(self):
        ranges = [(3, 5), (0, 3)]
        selection = Selection(ranges)
        self.assertTrue(selection)
        self.assertEqual(selection.ranges, ranges)
        self.assertEqual(selection.flat_size, 5)
        self.assertEqual(selection.flatten(), [3, 4, 0, 1, 2])

    def test_from_values(self):
        selection = Selection([1, 3, 4, 1])
        self.assertEqual(selection.ranges, [(1, 2), (3, 5), (1, 2)])


class TestNodePopulation(unittest.TestCase):
    def setUp(self):
        path = os.path.join(PATH, 'nodes1.h5')
        self.test_obj = NodeStorage(path).open_population('nodes-A')

    def test_name(self):
        self.assertEqual(self.test_obj.name, "nodes-A")

    def test_size(self):
        self.assertEqual(self.test_obj.size, 6)

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


if __name__ == '__main__':
    unittest.main()

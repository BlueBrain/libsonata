import os
import unittest

import numpy as np

from libsonata import (ElementReportPopulation,
                       ElementReportReader,
                       SomaReportPopulation,
                       SomaReportReader,
                       SonataError,
                       SpikePopulation,
                       SpikeReader,
                       )


PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                    '../../tests/data')


class TestSpikeReader(unittest.TestCase):
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
        self.assertEqual(self.test_obj['spikes2'].time_units, 'ms')
        self.assertEqual(self.test_obj['empty'].get(), [])

        self.assertEqual(len(self.test_obj['All'].get(node_ids=[])), 0)

        dict_data = self.test_obj['All'].get_dict()
        self.assertTrue((dict_data["node_ids"] == np.asarray([5, 2, 3, 2, 3])).all())
        self.assertTrue((dict_data["timestamps"] == np.asarray([0.1, 0.2, 0.3, 0.7, 1.3])).all())
        dict_data_filtered = self.test_obj['All'].get_dict(node_ids=[5, 3], tstart=0.2)
        self.assertTrue((dict_data_filtered["node_ids"] == np.asarray([3, 3])).all())
        self.assertTrue((dict_data_filtered["timestamps"] == np.asarray([0.3, 1.3])).all())
        dict_data_filtered_time = self.test_obj['All'].get_dict(tstart=0.2, tstop=1.0)
        self.assertTrue((dict_data_filtered_time["node_ids"] == np.asarray([2, 3, 2])).all())
        self.assertTrue((dict_data_filtered_time["timestamps"] == np.asarray([0.2, 0.3, 0.7])).all())
        dict_data_filtered_nodes = self.test_obj['All'].get_dict(node_ids=[5, 2])
        self.assertTrue((dict_data_filtered_nodes["node_ids"] == np.asarray([5, 2, 2])).all())
        self.assertTrue((dict_data_filtered_nodes["timestamps"] == np.asarray([0.1, 0.2, 0.7])).all())

    def test_getTimes_from_population(self):
        self.assertEqual(self.test_obj['All'].times, (0.1, 1.3))


class TestSomaReportReader(unittest.TestCase):
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


class TestElementReportReader(unittest.TestCase):
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

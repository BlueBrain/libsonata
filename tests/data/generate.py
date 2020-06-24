#!/usr/bin/env python

import sys
import numpy as np
import h5py

def get_vlen_str_type():
    if sys.version_info[0] == 2:
        vlen_str_type = unicode
    elif sys.version_info[0] == 3:
        vlen_str_type = str
    else:
        raise 'Only python2/3 supported'
    return vlen_str_type

def write_population_one_group(pop, prefix):
    string_dtype = h5py.special_dtype(vlen=get_vlen_str_type())

    pop.create_dataset('%s_group_id' % prefix, data=np.zeros(6), dtype=np.uint8)
    pop.create_dataset('%s_group_index' % prefix, data=np.arange(6), dtype=np.uint16)
    pop.create_dataset('%s_type_id' % prefix, data=np.full(6, -1), dtype=np.int32)

    attrs = pop.create_group('0')
    attrs.create_dataset('attr-X', data=np.arange(11., 17.), dtype=np.float64)
    attrs.create_dataset('attr-Y', data=np.arange(21, 27), dtype=np.int64)
    attrs.create_dataset('attr-Z', data=[(2 * x).encode('utf-8') for x in 'abcdef'],
                         dtype=string_dtype)

    # All supported dtypes
    attrs.create_dataset('A-double', dtype=np.float64)
    attrs.create_dataset('A-float', dtype=np.float32)
    attrs.create_dataset('A-int64', dtype=np.int64)
    attrs.create_dataset('A-int32', dtype=np.int32)
    attrs.create_dataset('A-int16', dtype=np.int16)
    attrs.create_dataset('A-int8', dtype=np.int8)
    attrs.create_dataset('A-uint64', dtype=np.uint64)
    attrs.create_dataset('A-uint32', dtype=np.uint32)
    attrs.create_dataset('A-uint16', dtype=np.uint16)
    attrs.create_dataset('A-uint8', dtype=np.uint8)
    attrs.create_dataset('A-string', dtype=string_dtype)

    # Unsupported dtype
    enum_dtype = h5py.special_dtype(enum=('i', {"RED": 0, "GREEN": 1}))
    attrs.create_dataset('A-enum', data=[0, 1], dtype=enum_dtype)

    attrs.create_dataset('E-mapping-good', data=[2, 1, 2, 0, 2, 2], dtype=np.int64)
    attrs.create_dataset('E-mapping-bad', data=[2, 3, 1, 0, -1, 2], dtype=np.int64)

    mapping = [s.encode('utf-8') for s in ("A", "B", "C")]
    library = attrs.create_group('@library')
    library.create_dataset('E-mapping-good', data=mapping, dtype=string_dtype)
    library.create_dataset('E-mapping-bad', data=mapping, dtype=string_dtype)
    library.create_dataset('E-mapping-ugly', data=mapping, dtype=string_dtype)

    dparams = attrs.create_group('dynamics_params')
    dparams.create_dataset('dparam-X', data=np.arange(1011., 1017.), dtype=np.float64)
    dparams.create_dataset('dparam-Y', data=np.arange(1021, 1027), dtype=np.int64)
    dparams.create_dataset('dparam-Z', data=[('d-' + 2 * x).encode('utf-8') for x in 'abcdef'],
                           dtype=string_dtype)


def write_population_two_groups(pop):
    pop.create_group('0')
    pop.create_group('1')


def write_nodes(filepath):
    with h5py.File(filepath, 'w') as h5f:
        root = h5f.create_group('nodes')
        write_population_one_group(
            root.create_group('nodes-A'),
            prefix='node'
        )
        write_population_two_groups(
            root.create_group('nodes-B')
        )


def group_ranges(values):
    result = []
    a, b = 0, 0
    for v in values:
        if v == b:
            b += 1
        else:
            if a < b:
                result.append((a, b))
            a, b = v, v + 1
    if a < b:
        result.append((a, b))
    return result


def write_edge_index_one_side(ids, N, out):
    index1, index2 = [], []
    for _id in range(N):
        ranges = group_ranges(np.where(ids == _id)[0])
        index1.append((len(index2), len(index2) + len(ranges)))
        index2.extend(ranges)
    out.create_dataset('node_id_to_ranges', data=index1, dtype=np.uint64)
    out.create_dataset('range_to_edge_id', data=index2, dtype=np.uint64)


def write_edge_index(pop, source_N, target_N):
    index_group = pop.create_group('indices')
    write_edge_index_one_side(
        pop['source_node_id'][:],
        source_N,
        index_group.create_group('source_to_target')
    )
    write_edge_index_one_side(
        pop['target_node_id'][:],
        target_N,
        index_group.create_group('target_to_source')
    )


def write_edges(filepath):
    with h5py.File(filepath, 'w') as h5f:
        root = h5f.create_group('edges')
        edges_AB = root.create_group('edges-AB')
        write_population_one_group(edges_AB, prefix='edge')
        edges_AB.create_dataset(
            'source_node_id', data=[1, 1, 2, 2, 3, 3], dtype=np.uint64
        ).attrs['node_population'] = u'nodes-A'
        edges_AB.create_dataset(
            'target_node_id', data=[1, 2, 1, 1, 0, 2], dtype=np.uint64
        ).attrs['node_population'] = u'nodes-B'
        write_edge_index(edges_AB, 4, 4)
        write_population_two_groups(
            root.create_group('edges-AC')
        )


def write_soma_report(filepath):
    population_names = ['All', 'soma1', 'soma2']
    node_ids = np.arange(1, 21)
    index_pointers = np.arange(0, 21)
    element_ids = np.zeros(20)
    times = (0.0, 1.0, 0.1)
    data = [node_ids + j*0.1 for j in range(10)]
    string_dtype = h5py.special_dtype(vlen=get_vlen_str_type())
    with h5py.File(filepath, 'w') as h5f:
        root = h5f.create_group('report')
        gpop_all = h5f.create_group('/report/' + population_names[0])
        ddata = gpop_all.create_dataset('data', data=data, dtype=np.float32)
        ddata.attrs.create('units', data="mV", dtype=string_dtype)
        gmapping = h5f.create_group('/report/' + population_names[0] + '/mapping')

        dnodes = gmapping.create_dataset('node_ids', data=node_ids, dtype=np.uint64)
        dnodes.attrs.create('sorted', data=True, dtype=np.uint8)
        gmapping.create_dataset('index_pointers', data=index_pointers, dtype=np.uint64)
        gmapping.create_dataset('element_ids', data=element_ids, dtype=np.uint32)
        dtimes = gmapping.create_dataset('time', data=times, dtype=np.double)
        dtimes.attrs.create('units', data="ms", dtype=string_dtype)

        gpop_soma1 = h5f.create_group('/report/' + population_names[1])
        gpop_soma2 = h5f.create_group('/report/' + population_names[2])


def write_element_report(filepath):
    population_names = ['All', 'element1', 'element42']
    node_ids = np.arange(1, 21)
    index_pointers = np.arange(0, 105, 5)
    repeated = np.arange(50)
    element_ids = np.repeat(repeated, 2)
    times = (0.0, 4.0, 0.2)
    string_dtype = h5py.special_dtype(vlen=get_vlen_str_type())
    with h5py.File(filepath, 'w') as h5f:
        root = h5f.create_group('report')
        gpop_all = h5f.create_group('/report/' + population_names[0])
        d1 = np.arange(0.0, 200, 0.1).reshape(20, -1)
        ddata = gpop_all.create_dataset('data', data=d1, dtype=np.float32)
        ddata.attrs.create('units', data="mV", dtype=string_dtype)
        gmapping = h5f.create_group('/report/' + population_names[0] + '/mapping')

        dnodes = gmapping.create_dataset('node_ids', data=node_ids, dtype=np.uint64)
        dnodes.attrs.create('sorted', data=True, dtype=np.uint8)
        gmapping.create_dataset('index_pointers', data=index_pointers, dtype=np.uint64)
        gmapping.create_dataset('element_ids', data=element_ids, dtype=np.uint32)
        dtimes = gmapping.create_dataset('time', data=times, dtype=np.double)
        dtimes.attrs.create('units', data="ms", dtype=string_dtype)

        gpop_soma1 = h5f.create_group('/report/' + population_names[1])
        gpop_soma2 = h5f.create_group('/report/' + population_names[2])

def write_spikes(filepath):
    population_names = ['All', 'spikes1', 'spikes2']
    timestamps_base = (0.3, 0.1, 0.2, 1.3, 0.7)
    node_ids_base = (3, 5, 2, 3, 2)

    sorting_type = h5py.enum_dtype({"none": 0, "by_id": 1, "by_time": 2})
    string_dtype = h5py.special_dtype(vlen=get_vlen_str_type())

    with h5py.File(filepath, 'w') as h5f:
        root = h5f.create_group('spikes')
        gpop_all = h5f.create_group('/spikes/' + population_names[0])
        gpop_all.attrs.create('sorting', data=2, dtype=sorting_type)
        timestamps, node_ids = zip(*sorted(zip(timestamps_base, node_ids_base)))
        set = gpop_all.create_dataset('timestamps', data=timestamps, dtype=np.double)
        gpop_all.create_dataset('node_ids', data=node_ids, dtype=np.uint64)

        gpop_spikes1 = h5f.create_group('/spikes/' + population_names[1])
        gpop_spikes1.attrs.create('sorting', data=1, dtype=sorting_type)
        node_ids, timestamps = zip(*sorted(zip(node_ids_base, timestamps_base)))
        gpop_spikes1.create_dataset('timestamps', data=timestamps, dtype=np.double)
        gpop_spikes1.create_dataset('node_ids', data=node_ids, dtype=np.uint64)

        gpop_spikes2 = h5f.create_group('/spikes/' + population_names[2])
        gpop_spikes2.attrs.create('sorting', data=0, dtype=sorting_type)
        dtimestamps = gpop_spikes2.create_dataset('timestamps', data=timestamps_base, dtype=np.double)
        dtimestamps.attrs.create('units', data="ms", dtype=string_dtype)
        gpop_spikes2.create_dataset('node_ids', data=node_ids_base, dtype=np.uint64)


if __name__ == '__main__':
    write_nodes('nodes1.h5')
    write_edges('edges1.h5')
    write_spikes('spikes.h5')
    write_soma_report('somas.h5')
    write_element_report('elements.h5')

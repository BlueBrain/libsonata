#!/usr/bin/env python

import numpy as np
import h5py


def write_population_one_group(pop, prefix):
    string_dtype = h5py.special_dtype(vlen=unicode)

    pop.create_dataset('%s_group_id' % prefix, data=np.zeros(6), dtype=np.uint8)
    pop.create_dataset('%s_group_index' % prefix, data=np.arange(6), dtype=np.uint16)
    pop.create_dataset('%s_type_id' % prefix, data=np.full(6, -1), dtype=np.int32)

    attrs = pop.create_group('0')
    attrs.create_dataset('attr-X', data=np.arange(11., 17.), dtype=np.float64)
    attrs.create_dataset('attr-Y', data=np.arange(21., 27.), dtype=np.int64)
    attrs.create_dataset('attr-Z', data=[(2 * x) for x in 'abcdef'], dtype=string_dtype)

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

    dparams = attrs.create_group('dynamics_params')
    dparams.create_dataset('dparam-X', data=np.arange(1011., 1017.), dtype=np.float64)
    dparams.create_dataset('dparam-Y', data=np.arange(1021., 1027.), dtype=np.int64)
    dparams.create_dataset('dparam-Z', data=[('d-' + 2 * x) for x in 'abcdef'], dtype=string_dtype)


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


if __name__ == '__main__':
    write_nodes('nodes1.h5')
    write_edges('edges1.h5')

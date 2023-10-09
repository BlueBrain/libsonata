import json
import os
import unittest

from libsonata import (
    NodeSets,
    NodeStorage,
    Selection,
    SonataError,
)


PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                    '../../tests/data')


class TestNodeSetFailure(unittest.TestCase):
    def test_CorrectStructure(self):
        self.assertRaises(SonataError, NodeSets, "1")
        self.assertRaises(SonataError, NodeSets, "[1]")
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": 1 }')
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": "string" }')
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": null }')
        self.assertRaises(SonataError, NodeSets, '{ "NodeSet0": true }')

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

    def test_MissingFile(self):
        self.assertRaises(SonataError, NodeSets.from_file, 'this/file/does/not/exist')


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
        j = json.dumps({"NodeSet0": {"E-mapping-good": "C"}})
        sel = NodeSets(j).materialize("NodeSet0", self.population)
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

        sel = NodeSets('{ "NodeSet0": { "node_id": [1, 3, 500] } }').materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection(((1, 2), (3, 4), )))

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

        j = json.dumps({
                "NodeSet0": { "E-mapping-good": ["A", "B"] },
                "NodeSet1": { "attr-Y": 21 },
                "NodeSetCompound0": ["NodeSet0", "NodeSet1"],
                "NodeSetCompound1": ["NodeSetCompound0"],
                "NodeSetCompound2": ["NodeSetCompound1"],
                "NodeSetCompound3": ["NodeSetCompound2"],
                "NodeSetCompound4": ["NodeSetCompound3"],
                "NodeSetCompound5": ["NodeSetCompound4"]
            })
        expected = Selection(((0, 2), (3, 4)))
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound1", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound2", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound3", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound4", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound5", self.population), expected)

        j = json.dumps({
                "NodeSetString0": { "E-mapping-good": ["A", ] },
                "NodeSetString1": { "E-mapping-good": ["A", "B"] },
                "NodeSetInt0": { "attr-Y": 21 },
                "NodeSetInt1": { "attr-Y": 22 },
                "NodeSetCompound0": ["NodeSetString0", "NodeSetString1", "NodeSetInt0", "NodeSetInt1"],
                "NodeSetCompound1": ["NodeSetCompound0", ],
                "NodeSetCompound2": ["NodeSetCompound1", "NodeSetString1"],
                "NodeSetCompound3": ["NodeSetCompound2", "NodeSetString0"],
                "NodeSetCompound4": ["NodeSetCompound3",],
                "NodeSetCompound5": ["NodeSetCompound4",],
            })
        expected = Selection(((0, 2), (3, 4)))
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound1", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound2", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound3", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound4", self.population), expected)
        self.assertEqual(NodeSets(j).materialize("NodeSetCompound5", self.population), expected)

    def test_NodeSet_toJSON(self):
        j = json.dumps(
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
        })
        new = NodeSets(j).toJSON()
        ns1 = NodeSets(new)
        self.assertEqual(new, ns1.toJSON())

        ns = NodeSets.from_file(os.path.join(PATH, 'node_sets.json'))
        self.assertEqual(new, ns.toJSON())

    def test_NodeSetEmptyArray(self):
        j = '''{"NodeSet0": { "node_id": [] } }'''
        sel = NodeSets(j).materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection([]))

        j = '''{ "NodeSetCompound0": [] } '''
        sel = NodeSets(j).materialize("NodeSetCompound0", self.population)
        self.assertEqual(sel, Selection([]))

    def test_library_datatype(self):
        # E-mapping-good is an @library value, we don't want to allow
        # materialization of @libraries by integers
        j = json.dumps({"NodeSet0": { "E-mapping-good": 0 }})
        self.assertRaises(SonataError, NodeSets(j).materialize, "NodeSet0", self.population)

        j = json.dumps({"NodeSet0": { "E-mapping-good": [0, 1, ] }})
        self.assertRaises(SonataError, NodeSets(j).materialize, "NodeSet0", self.population)

    def test_extend(self):
        ns0 = NodeSets(json.dumps({"NodeSet0": { "E-mapping-good": 0 }}))
        dup = ns0.update(ns0)
        self.assertEqual(dup, {"NodeSet0"})
        self.assertEqual(ns0.names, {"NodeSet0"})

        ns1 = NodeSets(json.dumps({"NodeSet1": { "E-mapping-good": 0 }}))
        dup = ns0.update(ns1)
        self.assertEqual(dup, set())
        self.assertEqual(ns0.names, {"NodeSet0", "NodeSet1"})

        ns0 = NodeSets(json.dumps({"NodeSet0": { "E-mapping-good": 0 }}))
        ns1 = NodeSets(json.dumps({"NodeSet0": { "E-mapping-good": 0 }}))
        dup = ns0.update(ns1)
        self.assertEqual(dup, {"NodeSet0"})
        self.assertEqual(ns0.names, {"NodeSet0"})

        j0 = '''{"NodeSet0": { "attr-Y": [21, 22] } }'''
        j1 = '''{"NodeSet0": { "attr-Y": [22] } }'''
        ns = NodeSets(j0)
        ns.update(NodeSets(j1))
        sel = ns.materialize("NodeSet0", self.population)
        self.assertEqual(sel, Selection(((1, 2), )))

import json
import os
import unittest

from libsonata import (CircuitConfig, CircuitConfigStatus, SimulationConfig, SonataError,
                       )


PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                    '../../tests/data')


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

        self.assertEqual(self.config.edge_population('edges-AB').name, 'edges-AB')

        self.assertEqual(self.config.config_status, CircuitConfigStatus.complete)

    def test_expanded_json(self):
        config = json.loads(self.config.expanded_json)
        self.assertEqual(config['components']['biophysical_neuron_models_dir'],
                         'biophysical_neuron_models')
        self.assertEqual(config['networks']['nodes'][0]['node_types_file'],
                         None)
        self.assertEqual(config['networks']['nodes'][0]['nodes_file'],
                         '../nodes1.h5')

    def test_spatial_directories(self):
        self.assertEqual(self.config.node_population_properties('nodes-A')
                        .spatial_segment_index_dir, '')
        self.assertTrue(self.config.node_population_properties('nodes-B')
                        .spatial_segment_index_dir.endswith('spatial_segment_index_dir'))

        self.assertEqual(self.config.edge_population_properties('edges-AB')
                        .spatial_synapse_index_dir, '')
        self.assertTrue(self.config.edge_population_properties('edges-AC')
                        .spatial_synapse_index_dir.endswith('spatial_synapse_index_dir'))

    def test_get_population_properties(self):
        node_prop = self.config.node_population_properties('nodes-A')
        self.assertEqual(node_prop.type, 'biophysical')
        self.assertTrue(node_prop.morphologies_dir.endswith('morphologies'))
        self.assertTrue(node_prop.biophysical_neuron_models_dir.endswith('biophysical_neuron_models'))
        self.assertEqual(node_prop.alternate_morphology_formats, {})

        self.assertEqual(node_prop.types_path, '')
        self.assertTrue(node_prop.elements_path.endswith('tests/data/nodes1.h5'))

        edge_prop = self.config.edge_population_properties('edges-AC')
        self.assertEqual(edge_prop.type, 'chemical')
        self.assertTrue(edge_prop.morphologies_dir.endswith('morphologies'))
        self.assertTrue(edge_prop.biophysical_neuron_models_dir.endswith('biophysical_neuron_models'))
        self.assertEqual(edge_prop.alternate_morphology_formats, {})

        self.assertEqual(edge_prop.types_path, '')
        self.assertTrue(edge_prop.elements_path.endswith('tests/data/edges1.h5'))
        self.assertTrue(edge_prop.spine_morphologies_dir.endswith('spine_morphologies_dir_edges-AC'))

        edge_prop = self.config.edge_population_properties('edges-AB')
        self.assertTrue(edge_prop.spine_morphologies_dir.endswith('spine_morphologies_dir_component'))

    def test_partial(self):
        contents = { "metadata": { "status": "NOT A TYPE" }, }
        self.assertRaises(SonataError, CircuitConfig, json.dumps(contents), PATH)

        contents = { "metadata": { "status": "partial" }, }
        cc = CircuitConfig(json.dumps(contents), PATH)
        self.assertEqual(cc.config_status, CircuitConfigStatus.partial)
        self.assertEqual(cc.node_populations, set())
        self.assertEqual(cc.edge_populations, set())

        contents = {
            "metadata": { "status": "partial" },
            "networks-mispelled": { },
            }
        cc = CircuitConfig(json.dumps(contents), PATH)
        self.assertEqual(cc.config_status, CircuitConfigStatus.partial)
        self.assertEqual(cc.node_populations, set())
        self.assertEqual(cc.edge_populations, set())

        contents = {
            "metadata": { "status": "partial" },
            "components": {
                "morphologies_dir": "some/morph/dir",
                },
            "networks": {
                "nodes": [],
                # Note: Missing "edges": []
                },
            }
        cc = CircuitConfig(json.dumps(contents), PATH)
        self.assertEqual(cc.config_status, CircuitConfigStatus.partial)
        self.assertEqual(cc.node_populations, set())
        self.assertEqual(cc.edge_populations, set())

        contents = {
            "metadata": { "status": "partial" },
            "components": {
                "morphologies_dir": "some/morph/dir",
                },
            "networks": {
                "nodes": [
                    {"nodes_file": "./nodes1.h5",
                     "populations": {
                         # nothing overridden; missing biophysical_neuron_models_dir
                         "nodes-A": { },
                         },
                     }],
                    # Note: Missing "edges": []
                }
            }
        cc = CircuitConfig(json.dumps(contents), PATH)
        prop = cc.node_population_properties('nodes-A')
        self.assertEqual(prop.alternate_morphology_formats, {})
        self.assertTrue(prop.morphologies_dir.endswith('some/morph/dir'))
        self.assertEqual(prop.biophysical_neuron_models_dir, '')
        self.assertEqual(cc.edge_populations, set())

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

    def test_vasculature_properties(self):
        contents = {
            "manifest": {
                "$BASE_DIR": "./",
                },
            "components": {
                "vasculature_mesh": "$BASE_DIR/vasculature_mesh.h5",
            },
            "networks": {
                "nodes": [
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": {
                            "nodes-A": {
                                "type": "vasculature",
                                "vasculature_file": "$BASE_DIR/vasculature_file.h5",
                            }
                        }
                    }],
                "edges": []
                }
            }
        res = CircuitConfig(json.dumps(contents), PATH)
        properties = res.node_population_properties('nodes-A')
        assert properties.vasculature_file.endswith('tests/data/vasculature_file.h5')
        assert properties.vasculature_mesh.endswith('tests/data/vasculature_mesh.h5')

    def test_astrocyte_properties(self):
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
                                "type": "astrocyte",
                                "microdomains_file": "$BASE_DIR/microdomains_file.h5",
                            }
                        }
                    }],
                "edges": []
                }
            }
        res = CircuitConfig(json.dumps(contents), PATH)
        properties = res.node_population_properties('nodes-A')
        assert properties.microdomains_file.endswith('tests/data/microdomains_file.h5')

    def test_endfeet_properties(self):
        contents = {
            "manifest": { "$BASE_DIR": "./", },
            "networks": {
                "nodes": [],
                "edges": [
                  {
                    "edges_file": "$NETWORK_DIR/edges1.h5",
                    "populations": {
                      "edges-AB": {
                        "type": "endfoot",
                        "endfeet_meshes_file": "$BASE_DIR/meshes.h5"
                      }
                    }
                  }
                ]
                }
            }
        res = CircuitConfig(json.dumps(contents), PATH)
        properties = res.edge_population_properties('edges-AB')
        assert properties.endfeet_meshes_file.endswith('tests/data/meshes.h5')

    def test_duplicate_population(self):
        # in nodes
        contents = {
            "manifest": {
                "$BASE_DIR": "./",
                },
            "components": {
                "biophysical_neuron_models_dir": "/biophysical_neuron_models",
                "morphologies_dir": "some/morph/dir",
            },
            "networks": {
                "nodes": [
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": { "nodes-A": { } }
                    },
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": { "nodes-A": { } }
                    }
                    ],
                "edges": [
                    ]
                }
            }
        self.assertRaises(SonataError, CircuitConfig, json.dumps(contents), PATH)

        # in edges
        contents = {
            "manifest": {
                "$BASE_DIR": "./",
                },
            "components": {
                "biophysical_neuron_models_dir": "/biophysical_neuron_models",
                "morphologies_dir": "some/morph/dir",
            },
            "networks": {
                "nodes": [
                    {
                        "nodes_file": "$BASE_DIR/nodes1.h5",
                        "populations": { "nodes-A": { } }
                    }
                    ],
                "edges": [
                    {
                        "edges_file": "$NETWORK_DIR/edges1.h5",
                        "edge_types_file": None,
                        "populations": { "edges-AB": {}, }
                    },
                    {
                        "edges_file": "$NETWORK_DIR/edges1.h5",
                        "edge_types_file": None,
                        "populations": { "edges-AB": {}, }
                    }
                    ]
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

        contents = {
              "manifest": {
                "$NETWORK_DIR": "./data",
                "$COMPONENT_DIR": "./"
              },
              "components": {
                "morphologies_dir": "$COMPONENT_DIR/morphologies",
                "biophysical_neuron_models_dir": "$COMPONENT_DIR/biophysical_neuron_models",
                "alternate_morphologies": {
                  "h5v1": "$COMPONENT_DIR/morphologies/h5"
                }
              },
              "networks": {
                "edges": [
                  {
                    "edges_file": "$NETWORK_DIR/edges1.h5",
                    "populations": {
                      "edges-AB": {
                        "morphologies_dir": "/my/custom/morphologies/dir"
                      }
                    }
                  }
                ],
                "nodes":[]
              }
            }
        cc = CircuitConfig(json.dumps(contents), PATH)
        ep = cc.edge_population_properties('edges-AB')
        assert ep.morphologies_dir == "/my/custom/morphologies/dir"


class TestSimulationConfig(unittest.TestCase):
    def setUp(self):
        self.config = SimulationConfig.from_file(
                        os.path.join(PATH, 'config', 'simulation_config.json'))

    def test_basic(self):
        self.assertEqual(self.config.base_path, os.path.abspath(os.path.join(PATH, 'config')))

        self.assertEqual(self.config.run.tstop, 1000)
        self.assertEqual(self.config.run.dt, 0.025)
        self.assertEqual(self.config.run.random_seed, 201506)
        self.assertEqual(self.config.run.spike_threshold, -35.5)
        self.assertEqual(self.config.run.integration_method,
                         SimulationConfig.Run.IntegrationMethod.crank_nicolson_ion)
        self.assertEqual(self.config.run.stimulus_seed, 111)
        self.assertEqual(self.config.run.ionchannel_seed, 222)
        self.assertEqual(self.config.run.minis_seed, 333)
        self.assertEqual(self.config.run.synapse_seed, 444)
        self.assertEqual(self.config.run.electrodes_file,
                         os.path.abspath(os.path.join(PATH, 'config/electrodes/electrode_weights.h5')))

        self.assertEqual(self.config.output.output_dir,
                         os.path.abspath(os.path.join(PATH, 'config/some/path/output')))
        self.assertEqual(self.config.output.spikes_file, 'out.h5')
        self.assertEqual(self.config.output.log_file, '')
        self.assertEqual(self.config.output.spikes_sort_order,
                         SimulationConfig.Output.SpikesSortOrder.by_id)

        self.assertEqual(self.config.conditions.celsius, 35.0)
        self.assertEqual(self.config.conditions.v_init, -80)
        self.assertEqual(self.config.conditions.spike_location,
                         SimulationConfig.Conditions.SpikeLocation.AIS)
        self.assertEqual(self.config.conditions.extracellular_calcium, None)
        self.assertEqual(self.config.conditions.randomize_gaba_rise_time, False)
        self.assertEqual(self.config.conditions.mechanisms, {'ProbAMPANMDA_EMS': {'property2': -1,
                                                                                  'property1': False},
                                                             'GluSynapse': {'property4': 'test',
                                                                            'property3': 0.025}})
        modifications = {o.name: o for o in self.config.conditions.modifications()}
        self.assertEqual(modifications["applyTTX"].type.name, "TTX")
        self.assertEqual(modifications["applyTTX"].node_set, "single")
        self.assertEqual(modifications["no_SK_E2"].type.name, "ConfigureAllSections")
        self.assertEqual(modifications["no_SK_E2"].node_set, "single")
        self.assertEqual(modifications["no_SK_E2"].section_configure, "%s.gSK_E2bar_SK_E2 = 0")

        self.assertEqual(self.config.list_report_names,
                         { "axonal_comp_centers", "cell_imembrane", "compartment", "soma", "lfp" })

        Report = SimulationConfig.Report
        self.assertEqual(self.config.report('soma').cells, 'Column')
        self.assertEqual(self.config.report('soma').type, Report.Type.compartment)
        self.assertEqual(self.config.report('soma').compartments, Report.Compartments.center)
        self.assertEqual(self.config.report('soma').enabled, True)
        self.assertEqual(self.config.report('soma').file_name,
                         os.path.abspath(os.path.join(self.config.output.output_dir, 'soma.h5')))
        self.assertEqual(self.config.report('compartment').dt, 0.1)
        self.assertEqual(self.config.report('compartment').sections, Report.Sections.all)
        self.assertEqual(self.config.report('compartment').compartments, Report.Compartments.all)
        self.assertEqual(self.config.report('compartment').enabled, False)
        self.assertEqual(self.config.report('axonal_comp_centers').start_time, 0)
        self.assertEqual(self.config.report('axonal_comp_centers').compartments, Report.Compartments.center)
        self.assertEqual(self.config.report('axonal_comp_centers').scaling, Report.Scaling.none)
        self.assertEqual(self.config.report('axonal_comp_centers').file_name,
                         os.path.abspath(os.path.join(self.config.output.output_dir, 'axon_centers.h5')))
        self.assertEqual(self.config.report('cell_imembrane').end_time, 500)
        self.assertEqual(self.config.report('cell_imembrane').type.name, 'summation')
        self.assertEqual(self.config.report('cell_imembrane').variable_name, 'i_membrane, IClamp')
        self.assertEqual(self.config.report('lfp').type, Report.Type.lfp)

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
                          "ex_sinusoidal",
                          "ex_sinusoidal_default_dt",
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

        self.assertEqual(self.config.input('ex_sinusoidal').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_sinusoidal').module.name, 'sinusoidal')
        self.assertEqual(self.config.input('ex_sinusoidal').delay, 10)
        self.assertEqual(self.config.input('ex_sinusoidal').duration, 80)
        self.assertEqual(self.config.input('ex_sinusoidal').node_set, "Mosaic")
        self.assertEqual(self.config.input('ex_sinusoidal').frequency, 8)
        self.assertEqual(self.config.input('ex_sinusoidal').amp_start, 0.2)
        self.assertEqual(self.config.input('ex_sinusoidal').dt, 0.5)

        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').module.name, 'sinusoidal')
        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').delay, 10)
        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').duration, 80)
        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').node_set, "Mosaic")
        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').frequency, 80)
        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').amp_start, 2)
        self.assertEqual(self.config.input('ex_sinusoidal_default_dt').dt, 0.025)

        self.assertEqual(self.config.input('ex_noise_meanpercent').input_type.name, 'current_clamp')
        self.assertEqual(self.config.input('ex_noise_meanpercent').module.name, 'noise')
        self.assertEqual(self.config.input('ex_noise_meanpercent').delay, 0)
        self.assertEqual(self.config.input('ex_noise_meanpercent').duration, 5000)
        self.assertEqual(self.config.input('ex_noise_meanpercent').node_set, "Rt_RC")

        self.assertEqual(self.config.input('ex_subthreshold').percent_less, 80)
        self.assertEqual(self.config.input('ex_shotnoise').rise_time, 0.4)
        self.assertEqual(self.config.input('ex_shotnoise').amp_mean, 70)
        self.assertEqual(self.config.input('ex_shotnoise').reversal, 10)
        self.assertEqual(self.config.input('ex_shotnoise').random_seed, None)

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
        self.assertEqual(self.config.input('ex_rel_shotnoise').random_seed, 230522)
        self.assertEqual(self.config.input('ex_rel_shotnoise').dt, 0.25)
        self.assertEqual(self.config.input('ex_rel_shotnoise').reversal, 0)
        self.assertEqual(self.config.input('ex_rel_shotnoise').relative_skew, 0.5)

        self.assertEqual(self.config.input('ex_replay').input_type.name, 'spikes')
        self.assertEqual(self.config.input('ex_replay').module.name, 'synapse_replay')
        self.assertEqual(self.config.input('ex_replay').delay, 0)
        self.assertEqual(self.config.input('ex_replay').duration, 40000)
        self.assertEqual(self.config.input('ex_replay').node_set, "Column")
        self.assertEqual(self.config.input('ex_replay').spike_file,
                         os.path.abspath(os.path.join(PATH, 'config/replay.h5')))
        self.assertEqual(self.config.input('ex_extracellular_stimulation').node_set, 'Column')

        self.assertEqual(self.config.input('ex_abs_shotnoise').input_type.name, "conductance")
        self.assertEqual(self.config.input('ex_abs_shotnoise').mean, 50)
        self.assertEqual(self.config.input('ex_abs_shotnoise').sigma, 5)
        self.assertEqual(self.config.input('ex_abs_shotnoise').reversal, 10)
        self.assertEqual(self.config.input('ex_abs_shotnoise').random_seed, None)
        self.assertEqual(self.config.input('ex_abs_shotnoise').represents_physical_electrode, True)
        self.assertEqual(self.config.input('ex_abs_shotnoise').relative_skew, 0.1)

        self.assertEqual(self.config.input('ex_OU').module.name, "ornstein_uhlenbeck")
        self.assertEqual(self.config.input('ex_OU').input_type.name, "conductance")
        self.assertEqual(self.config.input('ex_OU').tau, 2.8)
        self.assertEqual(self.config.input('ex_OU').reversal, 10)
        self.assertEqual(self.config.input('ex_OU').mean, 50)
        self.assertEqual(self.config.input('ex_OU').sigma, 5)
        self.assertEqual(self.config.input('ex_OU').random_seed, None)
        self.assertEqual(self.config.input('ex_OU').represents_physical_electrode, False)

        self.assertEqual(self.config.input('ex_rel_OU').input_type.name, "current_clamp")
        self.assertEqual(self.config.input('ex_rel_OU').tau, 2.8)
        self.assertEqual(self.config.input('ex_rel_OU').reversal, 0)
        self.assertEqual(self.config.input('ex_rel_OU').mean_percent, 70)
        self.assertEqual(self.config.input('ex_rel_OU').sd_percent, 10)
        self.assertEqual(self.config.input('ex_rel_OU').random_seed, 230522)

        self.assertEqual(self.config.input('ex_seclamp').voltage, 1.1)
        self.assertEqual(self.config.input('ex_seclamp').series_resistance, 0.5)

        overrides = {o.name: o for o in self.config.connection_overrides()}
        self.assertEqual(overrides['ConL3Exc-Uni'].source, 'Excitatory')
        self.assertEqual(overrides['ConL3Exc-Uni'].target, 'Mosaic')
        self.assertEqual(overrides['ConL3Exc-Uni'].weight, 1)
        self.assertEqual(overrides['ConL3Exc-Uni'].spont_minis, 0.01)
        self.assertEqual(overrides['ConL3Exc-Uni'].modoverride, 'GluSynapse')
        self.assertEqual(overrides['ConL3Exc-Uni'].delay, 0.5)
        self.assertEqual(overrides['ConL3Exc-Uni'].synapse_delay_override, None)
        self.assertEqual(overrides['ConL3Exc-Uni'].synapse_configure, None)
        self.assertEqual(overrides['ConL3Exc-Uni'].neuromodulation_dtc, None)
        self.assertEqual(overrides['ConL3Exc-Uni'].neuromodulation_strength, None)
        self.assertEqual(overrides['GABAB_erev'].spont_minis, None)
        self.assertEqual(overrides['GABAB_erev'].synapse_delay_override, 0.5)
        self.assertEqual(overrides['GABAB_erev'].delay, 0)
        self.assertEqual(overrides['GABAB_erev'].modoverride, None)
        self.assertEqual(overrides['GABAB_erev'].synapse_configure, '%s.e_GABAA = -82.0 tau_d_GABAB_ProbGABAAB_EMS = 77')
        self.assertEqual(overrides['GABAB_erev'].neuromodulation_dtc, 100)
        self.assertEqual(overrides['GABAB_erev'].neuromodulation_strength, 0.75)

        self.assertEqual(self.config.metadata,
                         {'sim_version': 1, 'note': 'first attempt of simulation', 'v_float': 0.5, 'v_bool': False})
        self.assertEqual(self.config.beta_features, {'v_str': 'abcd', 'v_int': 10, 'v_float': 0.5, 'v_bool': False})

    def test_expanded_json(self):
        config = json.loads(self.config.expanded_json)
        self.assertEqual(config['output']['output_dir'], 'some/path/output')

    def test_empty_connection_overrides(self):
        contents = """
        {
          "manifest": {
            "$CIRCUIT_DIR": "./circuit"
          },
          "network": "$CIRCUIT_DIR/circuit_config.json",
          "run": { "random_seed": 12345, "dt": 0.05, "tstop": 1000 },
          "connection_overrides": []
        }
        """
        conf = SimulationConfig(contents, "./")
        self.assertEqual(conf.connection_overrides(), [])

    def test_empty_conditions(self):
        contents = """
        {
          "manifest": { "$CIRCUIT_DIR": "./circuit" },
          "network": "$CIRCUIT_DIR/circuit_config.json",
          "run": { "random_seed": 12345, "dt": 0.05, "tstop": 1000 },
          "connection_overrides": []
        }
        """
        conf = SimulationConfig(contents, "./")
        self.assertEqual(conf.conditions.celsius, 34.0)
        self.assertEqual(conf.conditions.v_init, -80)

    def test_run(self):
        contents = """
        {
          "manifest": {
            "$CIRCUIT_DIR": "./circuit"
          },
          "network": "$CIRCUIT_DIR/circuit_config.json",
          "run": {
            "random_seed": 12345,
            "dt": 0.05,
            "tstop": 1000
          }
        }
        """
        conf = SimulationConfig(contents, "./")
        self.assertEqual(conf.run.stimulus_seed, 0)
        self.assertEqual(conf.run.ionchannel_seed, 0)
        self.assertEqual(conf.run.minis_seed, 0)
        self.assertEqual(conf.run.synapse_seed, 0)
        self.assertEqual(conf.run.electrodes_file, "")
        self.assertEqual(conf.run.spike_threshold, -30.0)

    def test_simulation_config_failures(self):
        with self.assertRaises(SonataError) as e:
            contents = """
            {
              "manifest": {
                "$CIRCUIT_DIR": "./circuit"
              },
              "network": "$CIRCUIT_DIR/circuit_config.json",
              "run": { "random_seed": 12345, "dt": 0.05, "tstop": 1000 },
              "connection_overrides": {"foo": "bar"}
            }
            """
            SimulationConfig(contents, "./")
        self.assertEqual(e.exception.args, ('`connection_overrides` must be an array', ))

        with self.assertRaises(SonataError) as e:
            contents = """
            {
              "run": { "random_seed": 12345, "dt": 0.05, "tstop": 1000 },
              "inputs" : {
                "ex_replay": {
                    "input_type": "spikes",
                    "module": "synapse_replay",
                    "delay": 0.0,
                    "duration": 40000.0,
                    "spike_file": "replay.dat",
                    "node_set": "Column"
                }
              }
            }
            """
            SimulationConfig(contents, "./")
        self.assertEqual(e.exception.args, ('Replay spike_file should be a SONATA h5 file', ))

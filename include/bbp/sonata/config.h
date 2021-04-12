/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *                         Jonas Karlsson <jonas.karlsson@epfl.ch>
 *                         Juan Hernando <juan.hernando@epfl.ch>
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#pragma once

#include <memory>  // std::unique_ptr
#include <set>
#include <string>
#include <vector>

#include <bbp/sonata/edges.h>
#include <bbp/sonata/nodes.h>

#include "common.h"


namespace bbp {
namespace sonata {


/** Read access to a SONATA circuit config file.
 */
class SONATA_API CircuitConfig
{
  public:
    /** Load SONATA circuit config JSON
     *
     * @param std::string contents of a circuit config JSON file
     * @throw bbp::sonata::SonataError if file is not found or invalid
     */
    CircuitConfig(const std::string& contents, const std::string& basePath);

    CircuitConfig(CircuitConfig&&);
    CircuitConfig(const CircuitConfig& other) = delete;
    ~CircuitConfig();

    /** Open a SONATA circuit config from a path to JSON */
    static CircuitConfig fromFile(const std::string& path);

    /** Return the target simulator */
    std::string getTargetSimulator() const;

    /** Return the node_sets path */
    std::string getNodeSetsPath() const;

    /** Return the names of the node population available */
    std::set<std::string> listNodePopulations() const;

    /** Return the directory of a component in the components_dir given its name
     *
     * @param name population name
     * @throw bbp::sonata::SonataError if population not found
     */
    NodePopulation getNodePopulation(const std::string& name) const;

    /** Return the names of the edge populations available */
    std::set<std::string> listEdgePopulations() const;

    /** Return the directory of a component in the components_dir given its name
     *
     * @param name population name
     * @throw bbp::sonata::SonataError if population not found
     */
    EdgePopulation getEdgePopulation(const std::string& name) const;

    /** Return the names of the components available */
    std::set<std::string> listComponents() const;

    /** Return the directory of a component in the components_dir given its name
     *
     * @param name component name
     * @throw bbp::sonata::SonataError if component not found
     */
    std::string getComponent(const std::string& name) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}  // namespace sonata
}  // namespace bbp

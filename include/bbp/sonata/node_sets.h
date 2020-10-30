#pragma once

#include <bbp/sonata/nodes.h>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace bbp {
namespace sonata {

class NodeSets;

class NodeSetRule
{
  public:
    virtual ~NodeSetRule(){};

    virtual Selection materialize(const NodeSets&, const NodePopulation&) const = 0;
    virtual std::string toJSON() const = 0;
};

using NodeSetRules = std::vector<std::unique_ptr<NodeSetRule>>;

class SONATA_API NodeSets
{
  public:
    /**
     * Create nodeset from JSON
     *
     * See also:
     * https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md#node-sets-file
     *
     * Note: floating point values aren't supported for comparison
     *
     * \param content is the JSON node_sets value
     * \throw if content cannot be parsed
     */
    NodeSets(const std::string& content);

    /**
     * Return a selection corresponding to the node_set name
     *
     * \param name is the name of the node_set rule to be evaluated
     * \param population is the population overwhich the returned selection will be valid
     */
    Selection materialize(const std::string& name, const NodePopulation& population) const;

    /**
     * Names of the node sets available
     */
    std::set<std::string> names() const;

    /**
     * Return string version of node sets
     */
    std::string toJSON() const;

  private:
    std::map<std::string, NodeSetRules> node_sets_;
};

}  // namespace sonata
}  // namespace bbp

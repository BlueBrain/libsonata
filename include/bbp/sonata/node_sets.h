#pragma once

#include <bbp/sonata/nodes.h>
#include <set>
#include <string>
#include <vector>

namespace bbp {
namespace sonata {
namespace detail {
class NodeSets;
}

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
    NodeSets(NodeSets&&);
    NodeSets(const NodeSets& other) = delete;
    NodeSets& operator=(NodeSets&&);
    ~NodeSets();

    /** Open a SONATA `node sets` file from a path */
    static NodeSets fromFile(const std::string& path);

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
    std::unique_ptr<detail::NodeSets> impl_;
};

}  // namespace sonata
}  // namespace bbp

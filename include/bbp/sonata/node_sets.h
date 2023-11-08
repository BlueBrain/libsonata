#pragma once

#include <bbp/sonata/nodes.h>
#include <set>
#include <string>
#include <vector>

namespace bbp {
namespace sonata {
namespace detail {
class NodeSets;
}  // namespace detail

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
    explicit NodeSets(const std::string& content);
    explicit NodeSets(std::unique_ptr<detail::NodeSets>&& impl);
    NodeSets(NodeSets&&) noexcept;
    NodeSets(const NodeSets& other) = delete;
    NodeSets& operator=(NodeSets&&) noexcept;
    ~NodeSets();

    /** Open a SONATA `node sets` file from a path */
    static NodeSets fromFile(const std::string& path);

    /**
     * Return a selection corresponding to the node_set name
     *
     * \param name is the name of the node_set rule to be evaluated
     * \param population is the population for which the returned selection will be valid
     */
    Selection materialize(const std::string& name, const NodePopulation& population) const;

    /**
     * Names of the node sets available
     */
    std::set<std::string> names() const;

    /**
     * Update `this` to include all nodesets from `this` and `other`.
     *
     * Duplicate names are overridden with the values from `other`.
     *
     * The duplicate names are returned.
     */
    std::set<std::string> update(const NodeSets& other) const;

    /**
     * Return the nodesets as a JSON string.
     */
    std::string toJSON() const;

  private:
    std::unique_ptr<detail::NodeSets> impl_;
};

}  // namespace sonata
}  // namespace bbp

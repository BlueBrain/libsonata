#include <algorithm>  // std::transform
#include <cmath>
#include <fmt/format.h>
#include <sstream>
#include <type_traits>

#include <json.hpp>

#include <bbp/sonata/node_sets.h>

namespace bbp {
namespace sonata {

namespace {
using json = nlohmann::json;

const size_t MAX_COMPOUND_RECURSION = 10;

template <typename T>
std::string toString(const std::string& key, const std::vector<T>& values) {
    return fmt::format(R"("{}": [{}])", key, fmt::join(values, ", "));
}

template <>
std::string toString(const std::string& key, const std::vector<std::string>& values) {
    // strings need to be wrapped in quotes
    return fmt::format(R"("{}": ["{}"])", key, fmt::join(values, "\", \""));
}

template <typename T>
class NodeSetBasicRule: public NodeSetRule
{
  public:
    NodeSetBasicRule(std::string attribute, std::vector<T>&& values)
        : attribute_(std::move(attribute))
        , values_(values) {}

    Selection materialize(const NodeSets& /* unused */, const NodePopulation& np) const final {
        Selection ret{{}};
        for (const auto& v : values_) {
            ret = ret | np.matchAttributeValues(attribute_, v);
        }
        return ret;
    }

    std::string toJSON() const final {
        return toString(attribute_, values_);
    }

  private:
    std::string attribute_;
    std::vector<T> values_;
};

class NodeSetBasicPopulation: public NodeSetRule
{
  public:
    explicit NodeSetBasicPopulation(std::string name)
        : name_(std::move(name)) {}

    Selection materialize(const NodeSets& /* unused */, const NodePopulation& np) const final {
        if (name_ == np.name()) {
            return np.selectAll();
        }

        return Selection{{}};
    }

    std::string toJSON() const final {
        const std::vector<std::string> values = {name_};
        return toString("population", values);
    }

  private:
    std::string name_;
};

class NodeSetBasicNodeIds: public NodeSetRule
{
  public:
    explicit NodeSetBasicNodeIds(Selection::Values&& values)
        : values_(values) {}

    Selection materialize(const NodeSets& /* unused */,
                          const NodePopulation& /* unused */) const final {
        return Selection::fromValues(values_.begin(), values_.end());
    }

    std::string toJSON() const final {
        return toString("node_ids", values_);
    }

  private:
    Selection::Values values_;
};

using CompoundTargets = std::vector<std::string>;
class NodeSetCompoundRule: public NodeSetRule
{
  public:
    NodeSetCompoundRule(std::string attribute, const CompoundTargets& targets)
        : attribute_(std::move(attribute))
        , targets_(targets) {}

    Selection materialize(const NodeSets& ns, const NodePopulation& np) const final {
        Selection ret{{}};
        for (const auto& target : targets_) {
            ret = ret | ns.materialize(target, np);
        }
        return ret;
    }

    std::string toJSON() const final {
        return toString("node_ids", targets_);
    }

  private:
    std::string attribute_;
    CompoundTargets targets_;
};

int64_t get_integer_or_throw(const json& el) {
    auto v = el.get<double>();
    if (std::floor(v) != v) {
        throw SonataError("Only allowed integers in node set rules");
    }
    return static_cast<int64_t>(v);
}

NodeSetRules _dispatch_node(const json& contents) {
    NodeSetRules ret;

    for (auto& el : contents.items()) {
        const std::string& attribute = el.key();
        if (el.value().is_number()) {
            if (attribute == "population") {
                throw SonataError("'population' must be a string");
            }

            int64_t v = get_integer_or_throw(el.value());
            if (attribute == "node_id") {
                if (v < 0) {
                    throw SonataError("'node_id' must be positive");
                }

                ret.emplace_back(new NodeSetBasicNodeIds({static_cast<Selection::Value>(v)}));
            } else {
                ret.emplace_back(new NodeSetBasicRule<int64_t>(attribute, {v}));
            }
        } else if (el.value().is_string()) {
            if (attribute == "node_id") {
                throw SonataError("'node_id' must be numeric or a list of numbers");
            }

            if (attribute == "population") {
                ret.emplace_back(new NodeSetBasicPopulation(el.value().get<std::string>()));
            } else {
                ret.emplace_back(
                    new NodeSetBasicRule<std::string>(attribute, {el.value().get<std::string>()}));
            }
        } else if (el.value().is_array()) {
            const auto array = el.value();

            if (attribute == "population" && (array.size() != 1 || !array[0].is_string())) {
                throw SonataError("'population' must be a single string");
            }

            if (array[0].is_number()) {
                std::vector<int64_t> values;
                for (auto& inner_el : array.items()) {
                    values.emplace_back(get_integer_or_throw(inner_el.value()));
                }

                if (attribute == "node_id") {
                    Selection::Values node_ids;
                    std::transform(begin(values),
                                   end(values),
                                   back_inserter(node_ids),
                                   [](int64_t integer) {
                                       if (integer < 0) {
                                           throw SonataError("'node_id' must be positive");
                                       }
                                       return static_cast<Selection::Value>(integer);
                                   });
                    ret.emplace_back(new NodeSetBasicNodeIds(std::move(node_ids)));
                } else {
                    ret.emplace_back(new NodeSetBasicRule<int64_t>(attribute, std::move(values)));
                }
            } else if (array[0].is_string()) {
                if (attribute == "node_id") {
                    throw SonataError("'node_id' must be numeric or a list of numbers");
                }

                std::vector<std::string> values;
                for (auto& inner_el : array.items()) {
                    values.emplace_back(inner_el.value().get<std::string>());
                }

                ret.emplace_back(new NodeSetBasicRule<std::string>(attribute, std::move(values)));
            } else {
                throw SonataError("Unknown array type");
            }
        }
    }
    return ret;
}

void parse_basic(const json& j, std::map<std::string, NodeSetRules>& node_sets) {
    for (auto& el : j.items()) {
        if (el.value().is_object()) {
            node_sets[el.key()] = _dispatch_node(el.value());
        }
    }
}

void check_compound(const std::map<std::string, NodeSetRules>& node_sets,
                    const std::map<std::string, CompoundTargets>& compound_rules,
                    size_t depth) {
    if (depth > MAX_COMPOUND_RECURSION) {
        throw SonataError("Compound node_set recursion depth exceeded");
    }

    for (auto const& it : compound_rules) {
        for (auto const& name : it.second) {
            if (compound_rules.count(name) > 0) {
                check_compound(node_sets, compound_rules, depth + 1);
            } else if (node_sets.count(name) == 0) {
                throw SonataError(fmt::format("Missing '{}' from node_sets", name));
            }
        }
    }
}

void parse_compound(const json& j, std::map<std::string, NodeSetRules>& node_sets) {
    std::map<std::string, CompoundTargets> compound_rules;
    for (auto& el : j.items()) {
        if (el.value().is_array()) {
            CompoundTargets targets;
            for (const auto& name : el.value()) {
                if (!name.is_string()) {
                    throw SonataError("All compound node types must be strings");
                }

                targets.emplace_back(name);
            }
            compound_rules[el.key()] = targets;
        }
    }

    check_compound(node_sets, compound_rules, 0);

    for (const auto& rule : compound_rules) {
        NodeSetRules rules;
        rules.emplace_back(new NodeSetCompoundRule(rule.first, rule.second));
        node_sets.emplace(rule.first, std::move(rules));
    }
}
}  // anonymous namespace

NodeSets::NodeSets(const std::string& content) {
    json j = json::parse(content);
    if (!j.is_object()) {
        throw SonataError("Top level node_set must be an object");
    }

    // Need to two pass parsing the json so that compound lookup can rely
    // on all the basic rules existing
    parse_basic(j, node_sets_);
    parse_compound(j, node_sets_);
}

Selection NodeSets::materialize(const std::string& name, const NodePopulation& population) const {
    Selection ret = population.selectAll();

    const auto& node_set = node_sets_.find(name);
    for (const auto& ns : node_set->second) {
        Selection a = ns->materialize(*this, population);
        ret = ret & a;
    }
    return ret;
}


std::set<std::string> NodeSets::names() const {
    std::set<std::string> ret;
    std::transform(begin(node_sets_),
                   end(node_sets_),
                   std::inserter(ret, ret.begin()),
                   [](decltype(node_sets_)::value_type const& pair) { return pair.first; });
    return ret;
}

std::string NodeSets::toJSON() const {
    auto replace_trailing_coma = [](std::string& s, char c) {
        s.pop_back();
        s.pop_back();
        s.push_back(c);
    };

    std::string ret{"{\n"};
    for (const auto& pair : node_sets_) {
        ret += fmt::format(R"(  "{}": {{)", pair.first);
        for (const auto& pred : pair.second) {
            ret += pred->toJSON();
            ret += ", ";
        }
        replace_trailing_coma(ret, ' ');
        ret += "},\n";
    }
    replace_trailing_coma(ret, '\n');
    ret += "}";

    return ret;
}

}  // namespace sonata
}  // namespace bbp

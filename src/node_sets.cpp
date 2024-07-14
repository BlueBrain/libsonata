#include <algorithm>  // std::find, std::transform
#include <cassert>
#include <cmath>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>

#include "../extlib/filesystem.hpp"

#include <nlohmann/json.hpp>
#include <utility>

#include "utils.h"  // readFile

#include <bbp/sonata/node_sets.h>

namespace bbp {
namespace sonata {

namespace fs = ghc::filesystem;

namespace detail {

const size_t MAX_COMPOUND_RECURSION = 10;

using json = nlohmann::json;

void replace_trailing_coma(std::string& s, char c) {
    s.pop_back();
    if (s.back() == ',') {
        s.pop_back();
    }
    s.push_back(c);
}

template <typename T>
std::string toString(const std::string& key, const std::vector<T>& values) {
    return fmt::format(R"("{}": [{}])", key, fmt::join(values, ", "));
}

template <>
std::string toString(const std::string& key, const std::vector<std::string>& values) {
    // strings need to be wrapped in quotes
    return fmt::format(R"("{}": ["{}"])", key, fmt::join(values, "\", \""));
}

class NodeSets;

class NodeSetRule
{
  public:
    NodeSetRule() = default;
    NodeSetRule(const NodeSetRule&) = delete;
    void operator=(const NodeSetRule&) = delete;
    NodeSetRule(const NodeSetRule&&) = delete;
    void operator=(const NodeSetRule&&) = delete;

    virtual ~NodeSetRule() = default;

    virtual Selection materialize(const NodeSets&, const NodePopulation&) const = 0;
    virtual std::string toJSON() const = 0;
    virtual bool is_compound() const {
        return false;
    }
    virtual std::unique_ptr<NodeSetRule> clone() const = 0;
};

using NodeSetRulePtr = std::unique_ptr<NodeSetRule>;
void parse_basic(const json& j, std::map<std::string, NodeSetRulePtr>& node_sets);
void parse_compound(const json& j, std::map<std::string, NodeSetRulePtr>& node_sets);

class NodeSets
{
    std::map<std::string, NodeSetRulePtr> node_sets_;

  public:
    explicit NodeSets(const json& j) {
        if (!j.is_object()) {
            throw SonataError("Top level node_set must be an object");
        }

        // Need to two pass parsing the json so that compound lookup can rely
        // on all the basic rules existing
        parse_basic(j, node_sets_);
        parse_compound(j, node_sets_);
    }

    static const fs::path& validate_path(const fs::path& path) {
        if (!fs::exists(path)) {
            throw SonataError(fmt::format("Path does not exist: {}", std::string(path)));
        }
        return path;
    }

    explicit NodeSets(const fs::path& path)
        : NodeSets(json::parse(std::ifstream(validate_path(path)))) {}

    static std::unique_ptr<NodeSets> fromFile(const std::string& path_) {
        fs::path path(path_);
        return std::make_unique<detail::NodeSets>(path);
    }

    explicit NodeSets(const std::string& content)
        : NodeSets(json::parse(content)) {}

    Selection materialize(const std::string& name, const NodePopulation& population) const;

    std::set<std::string> names() const {
        return getMapKeys(node_sets_);
    }

    std::set<std::string> update(const NodeSets& other) {
        if (&other == this) {
            return names();
        }
        std::set<std::string> duplicates;
        for (const auto& ns : other.node_sets_) {
            if (node_sets_.count(ns.first) > 0) {
                duplicates.insert(ns.first);
            }
            node_sets_[ns.first] = ns.second->clone();
        }
        return duplicates;
    }

    std::string toJSON() const {
        std::string ret{"{\n"};
        for (const auto& pair : node_sets_) {
            if (pair.second->is_compound()) {
                ret += fmt::format("  {},\n", pair.second->toJSON());
            } else {
                std::string contents = pair.second->toJSON();
                if (contents.empty()) {
                    continue;
                }
                ret += fmt::format(R"(  "{}": {{)", pair.first);
                ret += contents;
                ret += "},\n";
            }
        }
        replace_trailing_coma(ret, '\n');
        ret += "}";

        return ret;
    }
};

class NodeSetNullRule: public NodeSetRule
{
    Selection materialize(const detail::NodeSets& /* unused */,
                          const NodePopulation& /* unused */) const final {
        return Selection({});
    }

    std::string toJSON() const final {
        return {};
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        return std::make_unique<detail::NodeSetNullRule>();
    }
};

// { 'region': ['region1', 'region2', ...] }
template <typename T>
class NodeSetBasicRule: public NodeSetRule
{
  public:
    NodeSetBasicRule(std::string attribute, const std::vector<T>& values)
        : attribute_(std::move(attribute))
        , values_(values) {}

    Selection materialize(const detail::NodeSets& /* unused */,
                          const NodePopulation& np) const final {
        return np.matchAttributeValues(attribute_, values_);
    }

    void add_attribute2rule(std::map<std::string, std::set<T>>& attribute2rule) const {
        auto& s = attribute2rule[attribute_];
        for (const auto& v : values_) {
            s.insert(v);
        }
    }

    std::string toJSON() const final {
        return toString(attribute_, values_);
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        return std::make_unique<detail::NodeSetBasicRule<T>>(attribute_, values_);
    }

  private:
    std::string attribute_;
    std::vector<T> values_;
};

// { 'population': ['popA', 'popB', ] }
class NodeSetBasicPopulation: public NodeSetRule
{
  public:
    explicit NodeSetBasicPopulation(const std::vector<std::string>& values)
        : values_(values) {}

    Selection materialize(const detail::NodeSets& /* unused */,
                          const NodePopulation& np) const final {
        if (std::find(values_.begin(), values_.end(), np.name()) != values_.end()) {
            return np.selectAll();
        }

        return Selection{{}};
    }

    std::string toJSON() const final {
        return toString("population", values_);
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        return std::make_unique<detail::NodeSetBasicPopulation>(values_);
    }

  private:
    std::vector<std::string> values_;
};

// { 'node_id': [1, 2, 3, 4] }
class NodeSetBasicNodeIds: public NodeSetRule
{
  public:
    explicit NodeSetBasicNodeIds(Selection::Values values)
        : values_(std::move(values)) {}

    Selection materialize(const detail::NodeSets& /* unused */,
                          const NodePopulation& np) const final {
        return np.selectAll() & Selection::fromValues(values_.begin(), values_.end());
    }

    std::string toJSON() const final {
        return toString("node_id", values_);
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        return std::make_unique<detail::NodeSetBasicNodeIds>(values_);
    }

  private:
    Selection::Values values_;
};

//  {
//      "population": "biophysical",
//      "model_type": "point",
//      "node_id": [1, 2, 3, 5, 7, 9, ...]
//  }
class NodeSetBasicMultiClause: public NodeSetRule
{
  public:
    explicit NodeSetBasicMultiClause(std::vector<NodeSetRulePtr>&& clauses)
        : clauses_(std::move(clauses)) {}

    Selection materialize(const detail::NodeSets& ns, const NodePopulation& np) const final {
        Selection ret = np.selectAll();
        for (const auto& clause : clauses_) {
            ret = ret & clause->materialize(ns, np);
        }
        return ret;
    }

    std::string toJSON() const final {
        std::string ret;
        for (const auto& clause : clauses_) {
            std::string contents = clause->toJSON();
            if (contents.empty()) {
                continue;
            }
            ret += contents;
            ret += ", ";
        }
        replace_trailing_coma(ret, ' ');
        return ret;
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        std::vector<NodeSetRulePtr> clauses;
        clauses.reserve(clauses_.size());
        for (const auto& clause : clauses_) {
            clauses.push_back(clause->clone());
        }
        return std::make_unique<detail::NodeSetBasicMultiClause>(std::move(clauses));
    }

  private:
    std::vector<NodeSetRulePtr> clauses_;
};

// "string_attr": { "$regex": "^[s][o]me value$" }
class NodeSetBasicOperatorString: public NodeSetRule
{
  public:
    explicit NodeSetBasicOperatorString(std::string attribute,
                                        const std::string& op,
                                        std::string value)
        : op_(string2op(op))
        , attribute_(std::move(attribute))
        , value_(std::move(value)) {}

    Selection materialize(const detail::NodeSets& /* unused */,
                          const NodePopulation& np) const final {
        switch (op_) {
        case Op::regex:
            return np.regexMatch(attribute_, value_);
        default:              // LCOV_EXCL_LINE
            LIBSONATA_THROW_IF_REACHED  // LCOV_EXCL_LINE
        }
    }

    std::string toJSON() const final {
        return fmt::format(R"("{}": {{ "{}": "{}" }})", attribute_, op2string(op_), value_);
    }

    enum class Op {
        regex = 1,
    };

    static Op string2op(const std::string& s) {
        if (s == "$regex") {
            return Op::regex;
        }
        throw SonataError(fmt::format("Operator '{}' not available for strings", s));
    }

    static std::string op2string(const Op op) {
        switch (op) {
        case Op::regex:
            return "$regex";
        default:              // LCOV_EXCL_LINE
            LIBSONATA_THROW_IF_REACHED  // LCOV_EXCL_LINE
        }
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        return std::make_unique<detail::NodeSetBasicOperatorString>(attribute_,
                                                                    op2string(op_),
                                                                    value_);
    }

  private:
    Op op_;
    std::string attribute_;
    std::string value_;
};

// "numeric_attribute_gt": { "$gt": 3 },
class NodeSetBasicOperatorNumeric: public NodeSetRule
{
  public:
    explicit NodeSetBasicOperatorNumeric(std::string name, const std::string& op, double value)
        : name_(std::move(name))
        , value_(value)
        , op_(string2op(op)) {}

    Selection materialize(const detail::NodeSets& /* unused */,
                          const NodePopulation& np) const final {
        switch (op_) {
        case Op::gt:
            return np.filterAttribute<double>(name_, [this](const double v) { return v > value_; });
        case Op::lt:
            return np.filterAttribute<double>(name_, [this](const double v) { return v < value_; });
        case Op::gte:
            return np.filterAttribute<double>(name_,
                                              [this](const double v) { return v >= value_; });
        case Op::lte:
            return np.filterAttribute<double>(name_,
                                              [this](const double v) { return v <= value_; });
        default:              // LCOV_EXCL_LINE
            LIBSONATA_THROW_IF_REACHED  // LCOV_EXCL_LINE
        }
    }

    std::string toJSON() const final {
        return fmt::format(R"("{}": {{ "{}": {} }})", name_, op2string(op_), value_);
    }

    enum class Op {
        gt = 1,
        lt = 2,
        gte = 3,
        lte = 4,
    };

    static Op string2op(const std::string& s) {
        if (s == "$gt") {
            return Op::gt;
        } else if (s == "$lt") {
            return Op::lt;
        } else if (s == "$gte") {
            return Op::gte;
        } else if (s == "$lte") {
            return Op::lte;
        }
        throw SonataError(fmt::format("Operator '{}' not available for numeric", s));
    }

    static std::string op2string(const Op op) {
        switch (op) {
        case Op::gt:
            return "$gt";
        case Op::lt:
            return "$lt";
        case Op::gte:
            return "$gte";
        case Op::lte:
            return "$lte";
        default:              // LCOV_EXCL_LINE
            LIBSONATA_THROW_IF_REACHED  // LCOV_EXCL_LINE
        }
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        return std::make_unique<detail::NodeSetBasicOperatorNumeric>(name_, op2string(op_), value_);
    }

  private:
    std::string name_;
    double value_;
    Op op_;
};

using CompoundTargets = std::vector<std::string>;
class NodeSetCompoundRule: public NodeSetRule
{
  public:
    NodeSetCompoundRule(std::string name, CompoundTargets targets)
        : name_(std::move(name))
        , targets_(std::move(targets)) {}

    Selection materialize(const detail::NodeSets& ns, const NodePopulation& np) const final {
        Selection ret{{}};
        for (const auto& target : targets_) {
            ret = ret | ns.materialize(target, np);
        }
        return ret;
    }

    std::string toJSON() const final {
        return toString(name_, targets_);
    }

    bool is_compound() const override {
        return true;
    }
    const CompoundTargets& getTargets() const {
        return targets_;
    }

    std::unique_ptr<NodeSetRule> clone() const final {
        return std::make_unique<detail::NodeSetCompoundRule>(name_, targets_);
    }

  private:
    std::string name_;
    CompoundTargets targets_;
};

int64_t get_int64_or_throw(const json& el) {
    auto v = el.get<double>();
    if (std::floor(v) != v) {
        throw SonataError(fmt::format("expected integer, got float {}", v));
    }
    return static_cast<int64_t>(v);
}

uint64_t get_uint64_or_throw(const json& el) {
    auto v = el.get<double>();
    if (v < 0) {
        throw SonataError(fmt::format("expected unsigned integer, got {}", v));
    }

    if (std::floor(v) != v) {
        throw SonataError(fmt::format("expected integer, got float {}", v));
    }
    return static_cast<uint64_t>(v);
}

NodeSetRulePtr _dispatch_node(const std::string& attribute, const json& value) {
    if (value.is_number()) {
        if (attribute == "population") {
            throw SonataError("'population' must be a string");
        }

        if (attribute == "node_id") {
            Selection::Values node_ids{get_uint64_or_throw(value)};
            return std::make_unique<NodeSetBasicNodeIds>(std::move(node_ids));
        } else {
            std::vector<int64_t> f = {get_int64_or_throw(value)};
            return std::make_unique<NodeSetBasicRule<int64_t>>(attribute, f);
        }
    } else if (value.is_string()) {
        if (attribute == "node_id") {
            throw SonataError("'node_id' must be numeric or a list of numbers");
        }

        if (attribute == "population") {
            std::vector<std::string> v{value.get<std::string>()};
            return std::make_unique<NodeSetBasicPopulation>(v);
        } else {
            std::vector<std::string> f = {value.get<std::string>()};
            return std::make_unique<NodeSetBasicRule<std::string>>(attribute, f);
        }
    } else if (value.is_array()) {
        const auto& array = value;

        if (array.empty()) {
            return std::make_unique<NodeSetNullRule>();
        }

        if (array[0].is_number()) {
            if (attribute == "population") {
                throw SonataError("'population' must be a string");
            }

            std::vector<int64_t> values;
            for (auto& inner_el : array.items()) {
                values.emplace_back(get_int64_or_throw(inner_el.value()));
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
                return std::make_unique<NodeSetBasicNodeIds>(std::move(node_ids));
            } else {
                return std::make_unique<NodeSetBasicRule<int64_t>>(attribute, values);
            }
        } else if (array[0].is_string()) {
            if (attribute == "node_id") {
                throw SonataError("'node_id' must be numeric or a list of numbers");
            }

            std::vector<std::string> values;
            for (auto& inner_el : array.items()) {
                values.emplace_back(inner_el.value().get<std::string>());
            }

            if (attribute == "population") {
                return std::make_unique<NodeSetBasicPopulation>(values);
            } else {
                return std::make_unique<NodeSetBasicRule<std::string>>(attribute, values);
            }
        } else {
            throw SonataError("Unknown array type");
        }
    } else if (value.is_object()) {
        const auto& definition = value;
        if (definition.size() != 1) {
            throw SonataError(
                fmt::format("Operator '{}' must have object with one key value pair", attribute));
        }
        const auto key = definition.begin().key();
        const auto value = definition.begin().value();

        if (value.is_number()) {
            return std::make_unique<NodeSetBasicOperatorNumeric>(attribute,
                                                                 key,
                                                                 value.get<double>());
        } else if (value.is_string()) {
            return std::make_unique<NodeSetBasicOperatorString>(attribute,
                                                                key,
                                                                value.get<std::string>());
        } else {
            throw SonataError("Unknown operator");
        }
    } else {
        LIBSONATA_THROW_IF_REACHED  // LCOV_EXCL_LINE
    }
}

void parse_basic(const json& j, std::map<std::string, NodeSetRulePtr>& node_sets) {
    for (const auto& el : j.items()) {
        const auto& value = el.value();
        if (value.is_object()) {
            if (value.empty()) {
                // ignore
            } else if (value.size() == 1) {
                const auto& inner_el = value.items().begin();
                node_sets[el.key()] = _dispatch_node(inner_el.key(), inner_el.value());
            } else {
                std::vector<NodeSetRulePtr> clauses;
                for (const auto& inner_el : value.items()) {
                    clauses.push_back(_dispatch_node(inner_el.key(), inner_el.value()));
                }
                node_sets[el.key()] = std::make_unique<NodeSetBasicMultiClause>(std::move(clauses));
            }
        } else if (value.is_array()) {
            // will be parsed by the parse_compound
        } else {
            // null/boolean/number/string ?
            throw SonataError(fmt::format("Expected an array or an object, got: {}", value.dump()));
        }
    }
}

void check_compound(const std::map<std::string, NodeSetRulePtr>& node_sets,
                    const std::map<std::string, CompoundTargets>& compound_rules,
                    const std::string& name,
                    size_t depth) {
    if (node_sets.count(name) > 0) {
        return;
    }

    if (depth > MAX_COMPOUND_RECURSION) {
        throw SonataError("Compound node_set recursion depth exceeded");
    }

    const auto it = compound_rules.find(name);
    assert(it != compound_rules.end());

    for (auto const& target : it->second) {
        if (node_sets.count(target) == 0 && compound_rules.count(target) == 0) {
            throw SonataError(fmt::format("Missing '{}' from node_sets", target));
        }
        check_compound(node_sets, compound_rules, target, depth + 1);
    }
}

void parse_compound(const json& j, std::map<std::string, NodeSetRulePtr>& node_sets) {
    std::map<std::string, CompoundTargets> compound_rules;
    for (auto& el : j.items()) {
        if (el.value().is_array()) {
            CompoundTargets targets;
            for (const auto& name : el.value()) {
                if (!name.is_string()) {
                    throw SonataError("All compound elements must be strings");
                }

                targets.emplace_back(name);
            }
            compound_rules[el.key()] = targets;
        }
    }


    for (const auto& rule : compound_rules) {
        check_compound(node_sets, compound_rules, rule.first, 0);

        NodeSetRulePtr rules = std::make_unique<NodeSetCompoundRule>(rule.first, rule.second);
        node_sets.emplace(rule.first, std::move(rules));
    }
}

Selection NodeSets::materialize(const std::string& name, const NodePopulation& population) const {
    const auto& node_set = node_sets_.find(name);
    if (node_set == node_sets_.end()) {
        throw SonataError(fmt::format("Unknown node_set {}", name));
    }
    const auto& ns = node_set->second;
    if (!ns->is_compound()) {
        return population.selectAll() & ns->materialize(*this, population);
    }

    // it's common to have a deep structure of compound statements
    // (ie: a whole hierarchy of regions), all checking the same attribute
    // rather than `materializing` them separately, we group them, and materialize
    // them all at once
    Selection ret{{}};

    std::vector<NodeSetRule*> queue{ns.get()};
    std::map<std::string, std::set<std::string>> attribute2rule_strings;
    std::map<std::string, std::set<int64_t>> attribute2rule_int64;
    while (!queue.empty()) {
        const auto* ns = queue.back();
        queue.pop_back();

        if (ns->is_compound()) {
            const auto* targets = dynamic_cast<const NodeSetCompoundRule*>(ns);
            for (const auto& target : targets->getTargets()) {
                const auto& node_set = node_sets_.find(target)->second;
                if (node_set->is_compound()) {
                    queue.push_back(node_set.get());
                    continue;
                }

                {
                    const auto* basic_int = dynamic_cast<const NodeSetBasicRule<int64_t>*>(
                        node_set.get());
                    if (basic_int != nullptr) {
                        basic_int->add_attribute2rule(attribute2rule_int64);
                        continue;
                    }
                }

                {
                    const auto* basic_string = dynamic_cast<const NodeSetBasicRule<std::string>*>(
                        node_set.get());
                    if (basic_string != nullptr) {
                        basic_string->add_attribute2rule(attribute2rule_strings);
                        continue;
                    }
                }

                ret = ret | ns->materialize(*this, population);
            }
        } else {
            ret = ret | ns->materialize(*this, population);
        }
    }

    for (const auto& it : attribute2rule_strings) {
        std::vector<std::string> values(it.second.begin(), it.second.end());
        ret = ret | population.matchAttributeValues(it.first, values);
    }

    for (const auto& it : attribute2rule_int64) {
        std::vector<int64_t> values(it.second.begin(), it.second.end());
        ret = ret | population.matchAttributeValues(it.first, values);
    }

    return ret;
}
}  // namespace detail

NodeSets::NodeSets(const std::string& content)
    : impl_(new detail::NodeSets(content)) {}

NodeSets::NodeSets(std::unique_ptr<detail::NodeSets>&& impl)
    : impl_(std::move(impl)) {}

NodeSets::NodeSets(NodeSets&&) noexcept = default;
NodeSets& NodeSets::operator=(NodeSets&&) noexcept = default;
NodeSets::~NodeSets() = default;

NodeSets NodeSets::fromFile(const std::string& path) {
    return NodeSets(detail::NodeSets::fromFile(path));
}

Selection NodeSets::materialize(const std::string& name, const NodePopulation& population) const {
    return impl_->materialize(name, population);
}

std::set<std::string> NodeSets::names() const {
    return impl_->names();
}

std::set<std::string> NodeSets::update(const NodeSets& other) const {
    return impl_->update(*other.impl_);
}

std::string NodeSets::toJSON() const {
    return impl_->toJSON();
}

}  // namespace sonata
}  // namespace bbp

#pragma once

#include "common.h"

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <vector>


namespace bbp {
namespace sonata {

//--------------------------------------------------------------------------------------------------

class SONATA_API Selection
{
public:
    using Value = uint64_t;
    using Values = std::vector<Value>;
    using Range = std::pair<Value, Value>;
    using Ranges = std::vector<Range>;

    explicit Selection(Ranges&& ranges);
    explicit Selection(const Ranges& ranges);

    static Selection fromValues(const Values& values);

    const Ranges& ranges() const;

    Values flatten() const;

    size_t flatSize() const;

    bool empty() const;

private:
    const Ranges ranges_;
};

//--------------------------------------------------------------------------------------------------

class SONATA_API Population
{
public:
    /**
     * Name of the population used for identifying it in circuit composition
     */
    std::string name() const;

    /**
     * Total number of elements
     */
    uint64_t size() const;

    /**
     * All attribute names (CSV columns + required attributes + union of attributes in groups)
     */
    const std::set<std::string>& attributeNames() const;

    /**
     * Get attribute values for given Selection
     *
     * @param name is a string to allow attributes not defined in spec
     * @throw if there is no such attribute for the population
     * @throw if the attribute is not defined for _any_ element from the selection
     */
    template <typename T>
    std::vector<T> getAttribute(const std::string& name, const Selection& selection) const;

    /**
     * Get attribute values for given Selection
     *
     * @param name is a string to allow attributes not defined in spec
     * @param default is a value to use for groups w/o given attribute
     * @throw if there is no such attribute for the population
     */
    template <typename T>
    std::vector<T> getAttribute(const std::string& name, const Selection& selection, const T& defaultValue) const;

    /**
     * Get attribute data type

     * @internal
     * It is a helper method for dynamic languages bindings;
     * and is not intended for use in the ordinary client C++ code.
     */
    std::string _attributeDataType(const std::string& name) const;

    /**
     * All dynamics attribute names (JSON keys + union of attributes in groups)
     */
    const std::set<std::string>& dynamicsAttributeNames() const;

    /**
     * Get dynamics attribute values for given Selection
     *
     * @param name is a string to allow attributes not defined in spec
     * @throw if there is no such attribute for the population
     * @throw if the attribute is not defined for _any_ edge from the edge selection
     */
    template <typename T>
    std::vector<T> getDynamicsAttribute(const std::string& name, const Selection& selection) const;

    /**
     * Get dynamics attribute values for given Selection
     *
     * @param name is a string to allow attributes not defined in spec
     * @param default is a value to use for edge groups w/o given attribute
     * @throw if there is no such attribute for the population
     */
    template <typename T>
    std::vector<T> getDynamicsAttribute(const std::string& name, const Selection& selection, const T& defaultValue) const;

    /**
     * Get dynamics attribute data type

     * @internal
     * It is a helper method for dynamic languages bindings;
     * and is not intended for use in the ordinary client C++ code.
     */
    std::string _dynamicsAttributeDataType(const std::string& name) const;

protected:
    Population(
        const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name, const std::string& prefix
    );

    Population(const Population&) = delete;

    virtual ~Population();

    struct Impl;
    const std::unique_ptr<Impl> impl_;
};

//--------------------------------------------------------------------------------------------------

template<typename Population>
class SONATA_API PopulationStorage
{
public:
    PopulationStorage(const std::string& h5FilePath, const std::string& csvFilePath = "");

    PopulationStorage(const PopulationStorage&) = delete;

    ~PopulationStorage();

    /**
     * List all populations.
     *
     */
    std::set<std::string> populationNames() const;

    /**
     * Open specific population.
     * @throw if no population with such a name exists
     */
    std::shared_ptr<Population> openPopulation(const std::string& name) const;

protected:
    struct Impl;
    const std::unique_ptr<Impl> impl_;
};

//--------------------------------------------------------------------------------------------------

}
} // namespace bbp::sonata

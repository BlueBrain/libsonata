#include <bbp/sonata/common.h>
#include <bbp/sonata/edges.h>

#include <highfive/H5File.hpp>


namespace bbp {
namespace sonata {

struct EdgeStorage::Impl
{
    Impl(const std::string& _h5FilePath, const std::string& _csvFilePath)
        : h5FilePath(_h5FilePath)
        , csvFilePath(_csvFilePath)
        , h5File(h5FilePath)
        , h5Root(h5File.getGroup("/edges"))
    {
        if (!csvFilePath.empty()) {
            throw SonataError("CSV not supported at the moment");
        }
    }

    const std::string h5FilePath;
    const std::string csvFilePath;
    const HighFive::File h5File;
    const HighFive::Group h5Root;
};


EdgeStorage::EdgeStorage(const std::string& h5FilePath, const std::string& csvFilePath)
    : impl_(new EdgeStorage::Impl(h5FilePath, csvFilePath))
{
}


EdgeStorage::~EdgeStorage() = default;


std::set<std::string> EdgeStorage::populationNames() const
{
    std::set<std::string> result;
    for (const auto& name : impl_->h5Root.listObjectNames()) {
        result.insert(name);
    }
    return result;
}


std::shared_ptr<EdgePopulation> EdgeStorage::openPopulation(const std::string& name) const
{
    if (!impl_->h5Root.exist(name)) {
        throw SonataError(std::string("No such population: ") + name);
    }
    return std::make_shared<EdgePopulation>(impl_->h5FilePath, impl_->csvFilePath, name);
}


struct EdgePopulation::Impl
{
    Impl(const std::string& h5FilePath, const std::string& csvFilePath, const std::string& _name)
        : name(_name)
        , h5File(h5FilePath)
        , h5Root(h5File.getGroup("/edges").getGroup(name))
    {
    }

    const std::string name;
    const HighFive::File h5File;
    const HighFive::Group h5Root;
};


EdgePopulation::EdgePopulation(const std::string& h5FilePath, const std::string& csvFilePath, const std::string& name)
    : impl_(new EdgePopulation::Impl(h5FilePath, csvFilePath, name))
{
}


EdgePopulation::~EdgePopulation() = default;


std::string EdgePopulation::name() const
{
    return impl_->name;
}


uint64_t EdgePopulation::size() const
{
    const auto dset = impl_->h5Root.getDataSet("edge_type_id");
    return dset.getSpace().getDimensions()[0];
}


std::string EdgePopulation::sourcePopulation() const
{
    std::string result;
    impl_->h5Root.getDataSet("source_node_id").getAttribute("node_population").read(result);
    return result;
}


std::string EdgePopulation::targetPopulation() const
{
    std::string result;
    impl_->h5Root.getDataSet("target_node_id").getAttribute("node_population").read(result);
    return result;
}
}
} // namespace bbp::sonata

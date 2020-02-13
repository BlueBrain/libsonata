#include <iostream>
#include <tuple>

#include "../library/implementation_interface.hpp"
#include "../library/sonatareport.h"
#include "hdf5_writer.h"

namespace bbp {
namespace sonata {

HDF5Writer::HDF5Writer(const std::string& report_name)
    : report_name_(report_name)
    , file_(0)
    , dataset_(0)
    , collective_list_(0)
    , independent_list_(0) {
    hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
    std::tie(collective_list_, independent_list_) = Implementation::prepare_write(report_name,
                                                                                  plist_id);
    // Create hdf5 file named after the report_name
    const std::string file_name = report_name + ".h5";
    file_ = H5Fcreate(file_name.data(), H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    offset_[0] = 0;
    offset_[1] = 0;

    H5Pclose(plist_id);
}

void HDF5Writer::configure_group(const std::string& group_name) {
    hid_t group = H5Gcreate2(file_, group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Gclose(group);
}

void HDF5Writer::configure_attribute(const std::string& group_name,
                                     const std::string& attribute_name,
                                     const std::string& attribute_value) {
    logger->trace("Configuring attribute '{}' for group name '{}'", attribute_name, group_name);
    hid_t group_id = H5Gopen(file_, group_name.data(), H5P_DEFAULT);
    hsize_t attr_size = attribute_value.size();
    hid_t attr_space = H5Screate_simple(1, &attr_size, &attr_size);

    hid_t attr_id = H5Acreate2(
        group_id, attribute_name.c_str(), H5T_C_S1, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_id, H5T_C_S1, attribute_value.c_str());

    H5Aclose(attr_id);
    H5Sclose(attr_space);
    H5Gclose(group_id);
}

void HDF5Writer::configure_dataset(const std::string& dataset_name,
                                   int total_steps,
                                   int total_elements) {
    hsize_t dims[2];
    dims[0] = total_steps;
    dims[1] = Implementation::get_global_dims(report_name_, total_elements);
    hid_t data_space = H5Screate_simple(2, dims, nullptr);
    dataset_ = H5Dcreate(file_,
                         dataset_name.c_str(),
                         H5T_IEEE_F32LE,
                         data_space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);

    // Caculate the offset of each rank
    offset_[1] = Implementation::get_offset(report_name_, total_elements);
    H5Sclose(data_space);
}

void HDF5Writer::write(const std::vector<double>& buffer, int steps_to_write, int total_elements) {
    hsize_t count[2];
    count[0] = steps_to_write;
    count[1] = total_elements;

    hid_t memspace = H5Screate_simple(2, count, nullptr);
    hid_t filespace = H5Dget_space(dataset_);

    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset_.data(), nullptr, count, nullptr);
    /*for(int i=0; i<steps_to_write; i++) {
        if(i==0)
            H5Sselect_hyperslab(space, H5S_SELECT_SET, &offset_[i], NULL, &count[i], NULL);
        H5Sselect_hyperslab(space, H5S_SELECT_OR, &offset_[i], NULL, &count[i], NULL);
    }*/

    H5Dwrite(dataset_, H5T_NATIVE_DOUBLE, memspace, filespace, H5P_DEFAULT, buffer.data());
    offset_[0] += steps_to_write;

    H5Sclose(filespace);
    H5Sclose(memspace);
}

void HDF5Writer::write(const std::string& dataset_name, const std::vector<uint32_t>& buffer) {
    write_any(dataset_name, buffer);
}

void HDF5Writer::write(const std::string& dataset_name, const std::vector<uint64_t>& buffer) {
    write_any(dataset_name, buffer);
}

void HDF5Writer::write(const std::string& dataset_name, const std::vector<float>& buffer) {
    write_any(dataset_name, buffer);
}

void HDF5Writer::write(const std::string& dataset_name, const std::vector<double>& buffer) {
    write_any(dataset_name, buffer);
}

template <typename T>
void HDF5Writer::write_any(const std::string& dataset_name, const std::vector<T>& buffer) {
    hsize_t dims = buffer.size();
    hid_t type = h5typemap::get_h5_type(T(0));

    hsize_t global_dims = Implementation::get_global_dims(report_name_, dims);
    hsize_t offset = Implementation::get_offset(report_name_, dims);

    hid_t data_space = H5Screate_simple(1, &global_dims, nullptr);
    hid_t data_set = H5Dcreate(
        file_, dataset_name.c_str(), type, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t filespace = H5Dget_space(data_set);
    hid_t memspace = H5Screate_simple(1, &dims, nullptr);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, nullptr, &dims, nullptr);

    H5Dwrite(data_set, type, memspace, filespace, collective_list_, buffer.data());

    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(data_set);
    H5Sclose(data_space);
}

void HDF5Writer::close() {
    // We close the dataset "/data" and the hdf5 file
    if (dataset_) {
        H5Dclose(dataset_);
    }
    H5Fclose(file_);
}

}  // namespace sonata
}  // namespace bbp

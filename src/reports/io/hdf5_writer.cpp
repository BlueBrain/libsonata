#include <iostream>
#include <tuple>
#include <mpi.h>

#include <reports/library/reportinglib.hpp>
#include <reports/library/implementation_interface.hpp>
#include "hdf5_writer.hpp"

HDF5Writer::HDF5Writer(const std::string& report_name)
: IoWriter(report_name), m_file(0), m_dataset(0), m_collective_list(0), m_independent_list(0) {

    hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);
    std::tie(m_collective_list, m_independent_list) = Implementation::prepare_write(report_name, plist_id);

    // Create hdf5 file named after the report_name
    m_file = H5Fcreate(report_name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, plist_id);
    m_offset[0] = 0;
    m_offset[1] = 0;

    H5Pclose(plist_id);
}

void HDF5Writer::configure_group(const std::string& group_name) {
    hid_t group = H5Gcreate2(m_file, group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Gclose(group);
}

void HDF5Writer::configure_attribute(const std::string& group_name, const std::string& attribute_name, const std::string& attribute_value) {

    logger->trace("Configuring attribute '{}'", attribute_name);
    hsize_t attr_size = attribute_value.size();
    hid_t attr_space = H5Screate_simple(1, &attr_size, &attr_size);
    hid_t attr_id = H5Acreate2(m_file, attribute_name.c_str(), H5T_C_S1, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_id, H5T_C_S1, attribute_value.c_str());

    H5Aclose (attr_id);
    H5Sclose (attr_space);
}

void HDF5Writer::configure_dataset(const std::string& dataset_name, int total_steps, int total_elements) {

    hsize_t dims[2];
    dims[0] = total_steps;
    dims[1] = Implementation::get_global_dims(m_report_name, total_elements);
    hid_t data_space = H5Screate_simple(2, dims, nullptr);
    m_dataset = H5Dcreate(m_file, dataset_name.c_str(), H5T_IEEE_F32LE, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Caculate the offset of each rank
    m_offset[1] = Implementation::get_offset(m_report_name, total_elements);
    H5Sclose(data_space);
}

void HDF5Writer::write(double* buffer, int steps_to_write, int total_steps, int total_elements) {

    hsize_t count[2];
    count[0] = steps_to_write;
    count[1] = total_elements;

    hid_t memspace = H5Screate_simple(2, count, nullptr);
    hid_t filespace = H5Dget_space(m_dataset);

    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, m_offset, nullptr, count, nullptr);
    /*for(int i=0; i<steps_to_write; i++) {
        if(i==0)
            H5Sselect_hyperslab(space, H5S_SELECT_SET, &m_offset[i], NULL, &count[i], NULL);
        H5Sselect_hyperslab(space, H5S_SELECT_OR, &m_offset[i], NULL, &count[i], NULL);
    }*/

    H5Dwrite(m_dataset, H5T_NATIVE_DOUBLE, memspace, filespace, H5P_DEFAULT, buffer);
    m_offset[0] += steps_to_write;

    H5Sclose(filespace);
    H5Sclose(memspace);
}

void HDF5Writer::write(const std::string& dataset_name, const std::vector<int>& buffer) {

    write_any(dataset_name, buffer);
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

    hsize_t global_dims = Implementation::get_global_dims(m_report_name, dims);
    hsize_t offset = Implementation::get_offset(m_report_name, dims);

    hid_t data_space = H5Screate_simple(1, &global_dims, nullptr);
    hid_t data_set = H5Dcreate(m_file, dataset_name.c_str(), type, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t filespace = H5Dget_space(data_set);
    hid_t memspace = H5Screate_simple(1, &dims, nullptr);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, nullptr, &dims, nullptr);

    H5Dwrite(data_set, type, memspace, filespace, m_collective_list, &buffer[0]);

    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(data_set);
    H5Sclose(data_space);
}

void HDF5Writer::close() {
    // We close the dataset "/data" and the hdf5 file
    if(m_dataset) {
        H5Dclose(m_dataset);
    }
    H5Fclose(m_file);
}

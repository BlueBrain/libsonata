#include <iostream>
#include <mpi.h>

#include "reportinglib.hpp"
#include "hdf5_writer.hpp"

HDF5Writer::HDF5Writer(const std::string& report_name)
: IoWriter(report_name), m_file(0), m_dataSet(0), m_collective_list(0), m_independent_list(0) {

    hid_t plist_id = H5Pcreate(H5P_FILE_ACCESS);

#ifdef HAVE_MPI
    // Enable MPI access
    MPI_Info info = MPI_INFO_NULL;
    H5Pset_fapl_mpio(plist_id, ReportingLib::m_allCells, info);

    // Initialize independent/collective lists
    m_collective_list = H5Pcreate(H5P_DATASET_XFER);
    m_independent_list = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(m_collective_list, H5FD_MPIO_COLLECTIVE);
    H5Pset_dxpl_mpio(m_independent_list, H5FD_MPIO_INDEPENDENT);
#endif

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

void HDF5Writer::configure_attribute(const std::string& group_name, const std::string& attribute_name) {
    std::cout << "Configuring attribute: " << attribute_name << std::endl;

    const std::string value = "time";
    hsize_t attr_size = value.size();
    hid_t attr_space = H5Screate_simple(1, &attr_size, &attr_size);
    hid_t attr_id = H5Acreate2(m_file, attribute_name.c_str(), H5T_C_S1, attr_space, H5P_DEFAULT, H5P_DEFAULT);
    H5Awrite(attr_id, H5T_C_S1, value.c_str());

    H5Aclose (attr_id);
    H5Sclose (attr_space);
}

void HDF5Writer::configure_dataset(const std::string& dataset_name, int total_steps, int total_compartments) {

    hsize_t dims[2];
    dims[0] = total_steps;
    dims[1] = get_global_dims(total_compartments);
    hid_t data_space = H5Screate_simple(2, dims, nullptr);
    m_dataSet = H5Dcreate(m_file, dataset_name.c_str(), H5T_IEEE_F32LE, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Caculate the offset of each rank
    m_offset[1] = get_offset(total_compartments);
    H5Sclose(data_space);
}

void HDF5Writer::write(const std::string& name, float* buffer, int steps_to_write, int total_steps, int total_compartments) {

    hsize_t count[2];
    count[0] = steps_to_write;
    count[1] = total_compartments;

    hid_t memspace = H5Screate_simple(2, count, nullptr);
    hid_t filespace = H5Dget_space(m_dataSet);

    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, m_offset, nullptr, count, nullptr);
    /*for(int i=0; i<steps_to_write; i++) {
        if(i==0)
            H5Sselect_hyperslab(space, H5S_SELECT_SET, &m_offset[i], NULL, &count[i], NULL);
        H5Sselect_hyperslab(space, H5S_SELECT_OR, &m_offset[i], NULL, &count[i], NULL);
    }*/

    H5Dwrite(m_dataSet, H5T_NATIVE_FLOAT, memspace, filespace, H5P_DEFAULT, buffer);
    m_offset[0] += steps_to_write;

    H5Sclose(filespace);
    H5Sclose(memspace);
}

void HDF5Writer::write(const std::string& name, const std::vector<int>& buffer) {

    // element_ids
    hsize_t dims = buffer.size();

    hsize_t global_dims = get_global_dims(dims);
    hsize_t offset = get_offset(dims);

    hid_t data_space = H5Screate_simple(1, &global_dims, nullptr);
    hid_t data_set = H5Dcreate(m_file, name.c_str(), H5T_STD_I32LE, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t filespace = H5Dget_space(data_set);
    hid_t memspace = H5Screate_simple(1, &dims, nullptr);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, nullptr, &dims, nullptr);
    H5Dwrite(data_set, H5T_NATIVE_INT32, memspace, filespace, m_independent_list, &buffer[0]);

    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(data_set);
    H5Sclose(data_space);
}

void HDF5Writer::write(const std::string& name, const std::vector<uint32_t>& buffer) {

    // element_ids
    hsize_t dims = buffer.size();

    hsize_t global_dims = get_global_dims(dims);
    hsize_t offset = get_offset(dims);

    hid_t data_space = H5Screate_simple(1, &global_dims, nullptr);
    hid_t data_set = H5Dcreate(m_file, name.c_str(), H5T_STD_U32LE, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t filespace = H5Dget_space(data_set);
    hid_t memspace = H5Screate_simple(1, &dims, nullptr);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, nullptr, &dims, nullptr);
    H5Dwrite(data_set, H5T_NATIVE_UINT32, memspace, filespace, m_independent_list, &buffer[0]);

    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(data_set);
    H5Sclose(data_space);
}

void HDF5Writer::write(const std::string& name, const std::vector<uint64_t>& buffer) {

    // node_ids and index_pointers
    hsize_t dims = buffer.size();

    hsize_t global_dims = get_global_dims(dims);
    hsize_t offset = get_offset(dims);

    hid_t data_space = H5Screate_simple(1, &global_dims, nullptr);
    hid_t data_set = H5Dcreate(m_file, name.c_str(), H5T_STD_U64LE, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t filespace = H5Dget_space(data_set);
    hid_t memspace = H5Screate_simple(1, &dims, nullptr);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, nullptr, &dims, nullptr);
    H5Dwrite(data_set, H5T_NATIVE_UINT64, memspace, filespace, m_independent_list, &buffer[0]);

    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(data_set);
    H5Sclose(data_space);
}

void HDF5Writer::write(const std::string& name, const std::vector<float>& buffer) {

}

void HDF5Writer::write(const std::string& name, const std::vector<double>& buffer) {

    // timestamps
    hsize_t dims = buffer.size();

    hsize_t global_dims = get_global_dims(dims);
    hsize_t offset = get_offset(dims);

    hid_t data_space = H5Screate_simple(1, &global_dims, nullptr);
    hid_t data_set = H5Dcreate(m_file, name.c_str(), H5T_INTEL_F64, data_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t filespace = H5Dget_space(data_set);
    hid_t memspace = H5Screate_simple(1, &dims, nullptr);
    H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, nullptr, &dims, nullptr);
    H5Dwrite(data_set, H5T_NATIVE_DOUBLE, memspace, filespace, m_independent_list, &buffer[0]);

    H5Sclose(memspace);
    H5Sclose(filespace);
    H5Dclose(data_set);
    H5Sclose(data_space);
}

void HDF5Writer::close() {
    // We close the dataset "/data" and the hdf5 file
    if(m_dataSet) {
        H5Dclose(m_dataSet);
    }
    H5Fclose(m_file);
}


hsize_t HDF5Writer::get_global_dims(int dimension) {
    // Return dimension when serial
    hsize_t global_dims = dimension;
#ifdef HAVE_MPI
    MPI_Allreduce(&dimension, &global_dims, 1, MPI_INT, MPI_SUM, ReportingLib::m_allCells);
#endif
    return global_dims;
}

hsize_t HDF5Writer::get_offset(int dimension) {
    hsize_t offset = 0;
#ifdef HAVE_MPI
    MPI_Scan(&dimension, &offset, 1, MPI_INT, MPI_SUM, ReportingLib::m_allCells);
    offset -= dimension;
#endif
    return offset;
}

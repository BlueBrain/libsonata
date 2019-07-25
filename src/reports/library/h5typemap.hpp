//
// Created by jblanco on 7/25/19.
//
#pragma once

/****************************************************************************
 * Copyright (C) 2016, EPFL-BBP                                             *
 * All rights reserved                                                      *
 *                                                                          *
 * For the licensing terms see LICENSE file inside the root directory.      *
 * For the list of contributors see AUTHORS file inside the root directory. *
 ***************************************************************************/

/** @file h5typemap.hpp
 *  @brief Returns HDF5 type for basic types
 *  @bug No known bugs.
 */

namespace h5typemap {

    /** @brief Add explicit specialization for all required types.
     *
     *  @todo There are some inconsistencies that we have address here, for example,
     *  hdf5 suggest hbool_t instead of c++ type but MPI defines c99 _Bool only.
     *  Are they all compatible? More important is size_t which is platform dependent
     *  and may not be 64 byte unsigned int all the time. May be better to use uint64_t
     *  for all internal offsets that we are going to write to hdf5? */

    template <typename T>
    hid_t get_h5_type(__attribute__((unused)) T v);

    template <>
    hid_t inline get_h5_type(__attribute__((unused)) bool v) {
        return H5T_NATIVE_HBOOL;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) char v) {
        return H5T_NATIVE_CHAR;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) unsigned char v) {
        return H5T_NATIVE_UCHAR;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) short v) {
        return H5T_NATIVE_SHORT;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) unsigned short v) {
        return H5T_NATIVE_USHORT;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) int v) {
        return H5T_NATIVE_INT;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) unsigned v) {
        return H5T_NATIVE_UINT;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) long v) {
        return H5T_NATIVE_LONG;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) uint64_t v) {
        return H5T_NATIVE_UINT64;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) float v) {
        return H5T_NATIVE_FLOAT;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) double v) {
        return H5T_NATIVE_DOUBLE;
    }
    template <>
    hid_t inline get_h5_type(__attribute__((unused)) long double v) {
        return H5T_NATIVE_LDOUBLE;
    }
}

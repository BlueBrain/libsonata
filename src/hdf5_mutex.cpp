/*************************************************************************
 * Copyright (C) 2018-2019 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License.
 *
 * See top-level LICENSE.txt file for details.
 *************************************************************************/

#include <mutex>


namespace bbp {
namespace sonata {

std::mutex& hdf5Mutex() {
    static std::mutex _hdf5Mutex;
    return _hdf5Mutex;
}

}  // namespace sonata
}  // namespace bbp

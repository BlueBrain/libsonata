/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include <bbp/sonata/common.h>

namespace bbp {
namespace sonata {

SonataError::SonataError(const std::string& what)
    : std::runtime_error(what) {}
}  // namespace sonata
}  // namespace bbp

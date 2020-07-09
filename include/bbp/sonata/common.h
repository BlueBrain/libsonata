/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#pragma once

#include <cstdint>
#include <stdexcept>


#if defined(_WIN32) || defined __CYGWIN__
#define SONATA_DLLEXPORT __declspec(dllexport)
#define SONATA_DLLIMPORT __declspec(dllimport)
#else
#define SONATA_DLLEXPORT __attribute__((visibility("default")))
#define SONATA_DLLIMPORT
#endif

#if defined(SONATA_DLL)
#if defined(SONATA_DLL_EXPORTS)
#define SONATA_API SONATA_DLLEXPORT
#else
#define SONATA_API SONATA_DLLIMPORT
#endif
#else
#define SONATA_API
#endif


namespace bbp {
namespace sonata {
SONATA_API const std::string version();

using NodeID = uint64_t;
using EdgeID = uint64_t;
using ElementID = uint32_t;

class SONATA_API SonataError: public std::runtime_error
{
  public:
    explicit SonataError(const std::string& what);
};
}  // namespace sonata
}  // namespace bbp

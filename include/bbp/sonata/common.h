#pragma once

#include <cstdint>
#include <stdexcept>


namespace bbp {
namespace sonata {

using NodeID = uint64_t;
using EdgeID = uint64_t;

class SonataError : public std::runtime_error
{
public:
    explicit SonataError(const std::string& what);
};
}
} // namespace bbp::sonata

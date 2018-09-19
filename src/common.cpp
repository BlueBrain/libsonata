#include <bbp/sonata/common.h>

namespace bbp {
namespace sonata {

SonataError::SonataError(const std::string& what)
    : std::runtime_error(what)
{
}
}
} // namespace bbp::sonata

#include <mutex>


namespace bbp {
namespace sonata {

std::mutex& hdf5Mutex()
{
    static std::mutex _hdf5Mutex;
    return _hdf5Mutex;
}

}
} // namespace bbp::sonata

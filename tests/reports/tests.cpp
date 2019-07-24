#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

int main(int argc, char* argv[]) {

#ifdef HAVE_MPI
    MPI_Init(nullptr, nullptr);
#endif
    int result = Catch::Session().run(argc, argv);
#ifdef HAVE_MPI
    MPI_Finalize();
#endif
    return result;
}

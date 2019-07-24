#include <vector>

#include "report_format.hpp"

class SonataFormat: public ReportFormat {
  private:
    std::vector<uint64_t> node_ids;
    std::vector<uint64_t> index_pointers;
    std::vector<uint32_t> element_ids;

    std::vector<double> spike_timestamps;
    std::vector<int> spike_node_ids;

  public:
    SonataFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<cells_t> cells);
    ~SonataFormat() = default;

    void prepare_dataset() override;
    void write_report_header();
    void write_spikes_header();
    void write_data() override;
    void close() override;

    const std::vector<uint64_t>& get_node_ids() { return node_ids; }
    const std::vector<uint64_t>& get_index_pointers() { return index_pointers; }
    const std::vector<uint32_t>& get_element_ids() { return element_ids; }

    int get_compartment_offset();

    // Sort spikes
    double nrnmpi_dbl_allmin(double x, int numprocs);
    double nrnmpi_dbl_allmax(double x, int numprocs);
    void nrnmpi_int_alltoall(int* s, int* r, int n);
    void nrnmpi_dbl_alltoallv(double* s, int* scnt,int* sdispl,
                              double* r, int* rcnt, int* rdispl);
    void nrnmpi_int_alltoallv(int* s, int* scnt, int* sdispl, int* r, int* rcnt, int* rdispl);
    void local_spikevec_sort(std::vector<double>& isvect, std::vector<int>& isvecg,
                             std::vector<double>& osvect, std::vector<int>& osvecg);
    void sort_spikes(std::vector<double>& spikevec_time, std::vector<int>& spikevec_gid);
};
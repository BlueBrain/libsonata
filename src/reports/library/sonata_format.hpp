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
    SonataFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<nodes_t> nodes);
    ~SonataFormat() = default;

    void prepare_dataset() override;
    void write_report_header();
    void write_spikes_header();
    void write_data() override;
    void close() override;

    const std::vector<uint64_t>& get_node_ids() const { return node_ids; }
    const std::vector<uint64_t>& get_index_pointers() const { return index_pointers; }
    const std::vector<uint32_t>& get_element_ids() const { return element_ids; }

};
#pragma once
#include <map>

#include "cell.hpp"
#include "io_writer.hpp"

enum KindFormat {SONATA, BINARY};

class ReportFormat {
  protected:
    std::string m_reportName;
    float* m_reportBuffer;
    size_t m_bufferSize;
    int m_totalCompartments;
    int m_totalSpikes;
    int m_numSteps;
    int m_steps_to_write;
    int m_currentStep;
    int m_lastPosition;
    int m_remainingSteps;

    std::unique_ptr<IoWriter> m_ioWriter;

    using cells_t = std::map<int, Cell>;
    std::shared_ptr<cells_t> m_cells;

    void prepare_buffer(size_t max_buffer_size);

  public:
    ReportFormat(const std::string& report_name, size_t max_buffer_size, int num_steps, std::shared_ptr<cells_t> cells);
    virtual ~ReportFormat();

    static std::unique_ptr<ReportFormat> create_ReportFormat(const std::string& report_name, size_t max_buffer_size,
            int num_steps, std::shared_ptr<cells_t> cells, const KindFormat& kind);

    virtual void prepare_dataset() = 0;
    virtual void write_data() = 0;
    virtual void close() = 0;

    int record_data(double timestep, int ncells, int* cellids);
    int update_timestep(double timestep);

    int get_num_steps() { return m_numSteps; }
    size_t get_buffer_size() { return m_bufferSize; }
    float* get_report_buffer() { return m_reportBuffer; }
    void print_buffer();
};
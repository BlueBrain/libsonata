#include <iostream>

#include "ascii_writer.hpp"

AsciiWriter::AsciiWriter(const std::string& report_name)
: IoWriter(report_name), m_file(report_name) {}

AsciiWriter::~AsciiWriter() {}

void AsciiWriter::write(const std::string& name, float* buffer, int steps_to_write, int total_steps, int total_compartments) {
    m_file << name << std::endl;
    size_t buffer_size = total_steps * total_compartments;
    for(int i=0; i<buffer_size; i++) {
        m_file << buffer[i] << ", ";
    }
    m_file << std::endl << std::endl;
}

void AsciiWriter::write(const std::string& name, const std::vector<int>& buffer) {

}

void AsciiWriter::write(const std::string& name, const std::vector<uint32_t>& buffer) {
    m_file << name << std::endl;
    for(int elem: buffer) {
        m_file << elem << ", ";
    }
    m_file << std::endl << std::endl;
}

void AsciiWriter::write(const std::string& name, const std::vector<uint64_t>& buffer) {
    m_file << name << std::endl;
    for(int elem: buffer) {
        m_file << elem << ", ";
    }
    m_file << std::endl << std::endl;
}

void AsciiWriter::write(const std::string& name, const std::vector<float>& buffer) {

}

void AsciiWriter::write(const std::string& name, const std::vector<double>& buffer) {

}

void AsciiWriter::close() {
    std::cout << "Closing ascii writer!" << std::endl;
}
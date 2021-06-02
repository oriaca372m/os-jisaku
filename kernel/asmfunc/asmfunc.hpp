#include <cstdint>

extern "C" void io_out_32(std::uint16_t addr, std::uint32_t data);
extern "C" std::uint32_t io_in_32(std::uint16_t addr);

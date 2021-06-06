#include <cstdint>

extern "C" void io_out_32(std::uint16_t addr, std::uint32_t data);
extern "C" std::uint32_t io_in_32(std::uint16_t addr);
extern "C" std::uint16_t get_cs();
extern "C" void load_idt(std::uint16_t limit, std::uint64_t offset);

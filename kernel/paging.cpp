#include "paging.hpp"

#include <array>
#include <cstdint>

#include <asmfunc.hpp>

namespace {
	constexpr std::uint64_t page_size_4k = 4096;
	constexpr std::uint64_t page_size_2m = 512 * page_size_4k;
	constexpr std::uint64_t page_size_1g = 512 * page_size_2m;

	alignas(page_size_4k) std::array<std::uint64_t, 512> pml4_table;
	alignas(page_size_4k) std::array<std::uint64_t, 512> pdp_table;
	alignas(page_size_4k) std::array<std::array<std::uint64_t, 512>, page_directory_count> page_directory;
}

void setup_identity_page_table() {
	pml4_table[0] = reinterpret_cast<std::uint64_t>(&pdp_table[0]) | 0x003;
	for (int i_pdpt = 0; i_pdpt < page_directory.size(); ++i_pdpt) {
		pdp_table[i_pdpt] = reinterpret_cast<std::uint64_t>(&page_directory[i_pdpt]) | 0x003;

		for (int i_pd = 0; i_pd < 512; ++i_pd) {
			page_directory[i_pdpt][i_pd] = i_pdpt * page_size_1g + i_pd * page_size_2m | 0x083;
		}
	}

	set_cr3(reinterpret_cast<std::uint64_t>(&pml4_table[0]));
}

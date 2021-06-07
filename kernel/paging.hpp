#pragma once

#include <cstddef>

constexpr std::size_t page_directory_count = 64;

void setup_identity_page_table();

#pragma once
#include <filesystem>
#include <iostream>

#define LOG_ERROR(x) std::cout << __LINE__ << ": ERROR: " << x << "\n";

auto testpath = std::filesystem::temp_directory_path().string() + "/testing/";
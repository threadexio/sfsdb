#pragma once
#include <filesystem>

auto testpath = std::filesystem::temp_directory_path().string() + "/testing/";
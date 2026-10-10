#include "core/ConsoleLogger.hpp"

#include <iostream>

std::mutex ConsoleLogger::mtx_;

void ConsoleLogger::info(std::string_view msg)
{
    std::lock_guard lock(mtx_);
    std::cout << "[info] " << msg << std::endl;
}

void ConsoleLogger::warn(std::string_view msg)
{
    std::lock_guard lock(mtx_);
    std::cout << "[warn] " << msg << std::endl;
}

void ConsoleLogger::error(std::string_view msg)
{
    std::lock_guard lock(mtx_);
    std::cerr << "[error] " << msg << std::endl;
}

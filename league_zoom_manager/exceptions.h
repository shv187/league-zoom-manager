#pragma once

#include <exception>
#include <format>
#include <source_location>

#define THROW_EXCEPTION(msg) \
{ \
    const std::source_location& location = std::source_location::current(); \
    throw std::runtime_error(std::format("File: {}\n\n Line: {}\n\n Function: {}\n\n Error Message: {}\n", location.file_name(), location.line(), location.function_name(), msg)); \
}
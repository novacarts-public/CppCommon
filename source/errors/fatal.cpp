/*!
    \file fatal.cpp
    \brief Fatal abort execution implementation
    \author Ivan Shynkarenka
    \date 04.04.2016
    \copyright MIT License
*/

#include "errors/fatal.h"

#include <cstdlib>
#include <iostream>

namespace CppCommon {

void fatal(const SourceLocation& location, const StackTrace& trace, const std::string& message, int error) noexcept
{
    std::cerr << "Fatal error: " << message << std::endl;
    std::cerr << "System error: " << error << std::endl;
    std::cerr << "System message: " << SystemError::to_string(error) << std::endl;
    std::cerr << "Source location: " << location.to_string() << std::endl;
    std::cerr << "Stack trace: " << std::endl << trace.to_string() << std::endl;
    std::abort();
}

} // namespace CppCommon

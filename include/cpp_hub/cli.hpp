// include/cpp_hub/cli.hpp
#pragma once

#include <iosfwd>

namespace cpp_hub::cli {

int run(int argc, char** argv);
void print_usage(std::ostream& os);

} // namespace cpp_hub::cli
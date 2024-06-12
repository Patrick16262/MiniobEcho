#pragma once

#include <string>


namespace DebugUtils {
void write_startup_debug_external(int argc, char **argv);
void write_start_debug_external(int argc, char **argv);
void write_debug_external(const char *msg);
std::string read_debug_external();

void exec_debug_thread();
}  // namespace DebugUtils

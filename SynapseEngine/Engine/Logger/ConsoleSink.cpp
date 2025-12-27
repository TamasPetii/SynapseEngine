#include "ConsoleSink.h"
#include <filesystem>
#include <print>

 namespace Syn
 {
     void ConsoleSink::Log(const LogMessage& msg) {
         std::scoped_lock lock(_mtx);
         std::println("{}", msg.ToString());
     }
 }
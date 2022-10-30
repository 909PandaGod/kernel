#include "handlers.h"
#include <klog/klog.h>
#include <smp/smp.h>
#include <tty/tty.h>

namespace handlers
{
    void handle_overflow(std::uint64_t i1, std::uint64_t i2)
    {
        klog::log("====================== " RED("#OF") " ======================\n");
        debug::log_register(smp::core_local::get().ctxbuffer);
        klog::panic("#OF");
    }
} // namespace handlers

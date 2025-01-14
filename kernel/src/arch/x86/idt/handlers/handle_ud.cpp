#include <klog/klog.h>
#include <idt/handlers/handlers.h>
#include <smp/smp.h>
#include <tty/tty.h>

namespace handlers
{
    void handle_ud(std::uint64_t /*unused*/, std::uint64_t /*unused*/)
    {
        klog::log("====================== " RED("#UD") " ======================");
        debug::log_register(smp::core_local::get().ctxbuffer);
        klog::panic("#UD");
    }
} // namespace handlers

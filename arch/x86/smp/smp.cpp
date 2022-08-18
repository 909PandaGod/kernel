// cSpell:ignore apic, hhdm, lapic, wrmsr, efer, stivale, rdmsr
#include "smp.h"
#include "context/context.h"
#include "mm/mm.h"
#include <asm/asm_cpp.h>
#include <atomic>
#include <gdt/gdt.h>
#include <idt/handlers/handlers.h>
#include <klog/klog.h>
#include <mm/pmm.h>
#include <paging/paging.h>
#include <sync_wrappers.h>

namespace smp
{
    void core_local::create(core_local* cpu0)
    {
        entries = new core_local*[boot_resource::instance().core_count()];
        entries[0] = cpu0;
        for (std::size_t i = 1; i < boot_resource::instance().core_count(); i++)
            entries[i] = new core_local;
        // point entries
    }

    static void initialize_apic(smp::core_local& local)
    {
        uint64_t l = local.apic.get_apic_base();
        paging::map_hhdm_phys(paging::page_type::SMALL, l);
        local.apic.enable();
        klog::log("APIC: ticks per ms: %lu\n", local.apic.calibrate());
        idt::register_idt(handlers::handle_timer, 32ul);
        local.apic.set_tick(32, 20);
    }

    [[noreturn]] static void idle(uint64_t)
    {
        enable_interrupt();
        std::detail::errors::__halt();
    }

    [[noreturn]] static void smp_main(stivale2_smp_info* info)
    {
        mark_stack(debug::SMP);
        disable_interrupt();
        info = mm::make_virtual<stivale2_smp_info>((uint64_t)info);

        const auto [core_id, cr3] = *((smp::init_data*)(info->extra_argument));

        write_cr3(mm::make_physical(cr3));

        wrmsr(msr::IA32_GS_BASE, smp::core_local::gs_of(core_id));
        wrmsr(msr::IA32_KERNEL_GS_BASE, smp::core_local::gs_of(core_id));
        wrmsr(msr::IA32_PAT, 0x706050403020100);
        wrmsr(msr::IA32_EFER, rdmsr(msr::IA32_EFER) | (1 << 11));

        smp::core_local& local = smp::core_local::get();
        local.ctxbuffer = new context; // dummy execution context
        local.idt_handler_entries = new uintptr_t[256];
        local.idt_entries = new idt::idt_entry[256];
        local.coreid = core_id;
        local.current_tid = { 0xffffffff, 0xffffffff };

        //local.gdt.set_ist(&local.ist);
        gdt::install_gdt();
        gdt::reload_gdt_smp();
        idt::init_idt();
        idt::install_idt();

        klog::log("SMP started\n");
    /*
        for (std::size_t i = 0; i < 32; i++)
            if (!idt::register_idt(handlers::INTERRUPT_HANDLERS[i], i))
                klog::panic("failed to allocate irq");*/

        local.tasks = new scheduler::task_queue;
        proc::make_kthread(idle, 0);

        proc::make_kthread(
            +[](uint64_t a) {
                klog::log("value: %lu\n", a);
                idle(0);
            },
            local.coreid);
        initialize_apic(smp::core_local::get());
        idle(0);
    }

    // used for proper frame pointer generation
    static void main_wrapper(stivale2_smp_info* s) { smp_main(s); }

    [[noreturn]] void init(stivale2_struct_tag_smp* smp)
    {
        boot_resource::instance().mark_smp_start();
        std::printf("init_smp(): bootstrap_processor_id=%u\n", smp->bsp_lapic_id);
        std::printf("  cpus: %lu\n", smp->cpu_count);

        std::size_t core_id = 0;
        std::size_t bsp_index = 0;
        for (std::size_t i = 0; i < smp->cpu_count; i++)
        {
            if (smp->bsp_lapic_id == smp->smp_info[i].lapic_id)
            {
                bsp_index = i;
                static init_data cpu0_data{0, smp::core_local::get(0).pagemap};
                smp->smp_info[i].extra_argument = (uintptr_t)&cpu0_data;
                continue;
            }

            core_id++;
            std::printf("initalizing extra smp core: %lu\n", i);
            smp->smp_info[i].extra_argument = core_id;

            uintptr_t stack_base = (uintptr_t)mm::pmm_allocate();

            uintptr_t stack_ptr = stack_base + paging::PAGE_SMALL_SIZE - sizeof(init_data);

            std::printf("  stack: 0x%016lx-0x%016lx\n", stack_base, stack_ptr);

            // remember that stack grows down
            smp->smp_info[i].target_stack = stack_ptr;

            smp->smp_info[i].extra_argument = (uintptr_t) new ((void*)stack_ptr) smp::init_data{
                .coreid = core_id,
                .cr3 = (smp::core_local::get(core_id).pagemap = (paging::page_table_entry*)mm::pmm_allocate())};

            paging::sync_page_tables(core_id, 0);
            stdext::direct_atomic_store(&smp->smp_info[i].goto_address, (uintptr_t)smp::main_wrapper,
                                        std::memory_order_seq_cst);
        }
        // initialize myself too!
        smp::smp_main(&smp->smp_info[bsp_index]);
    }
} // namespace smp

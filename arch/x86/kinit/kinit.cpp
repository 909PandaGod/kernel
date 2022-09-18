// cSpell:ignore stivale, alignas, rsdp, lapic, efer, wrmsr, kpages, rdmsr, cpuid, kinit, xsdt
#include "boot_resource.h"
#include "stivale2.h"
#include <acpi/acpi.h>
#include <asm/asm_cpp.h>
#include <config.h>
#include <cpuid/cpuid.h>
#include <cstdint>
#include <debug/kinit_dump.h>
#include <gdt/gdt.h>
#include <idt/idt.h>
#include <mm/malloc.h>
#include <mm/pmm.h>
#include <new>
#include <paging/paging.h>
#include <pci/pci.h>
#include <printf.h>
#include <smp/smp.h>
#include <sync/spinlock.h>
#include <tty/tty.h>

static std::uint8_t stack[4096];

static stivale2_header_tag_smp smp_tag{.tag = {.identifier = STIVALE2_HEADER_TAG_SMP_ID, .next = 0}, .flags = 0};

static stivale2_tag unmap_null_tag = {.identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID, .next = (std::uint64_t)&smp_tag};

static stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    .tag = {.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID, .next = (std::uint64_t)&unmap_null_tag},
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0};

static stivale2_header stivale_hdr
    [[gnu::section(".stivale2hdr"), gnu::used]] = {.entry_point = 0,
                                                   .stack = (std::uintptr_t)stack + sizeof(stack),
                                                   .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
                                                   .tags = (std::uintptr_t)&framebuffer_hdr_tag};

template <typename H>
static H* stivale2_get_tag(struct stivale2_struct* stivale2_struct, std::uint64_t id)
{
    struct stivale2_tag* current_tag = (stivale2_tag*)stivale2_struct->tags;
    while (true)
    {
        if (current_tag == nullptr)
            return nullptr;
        if (current_tag->identifier == id)
            return (H*)current_tag;
        current_tag = (stivale2_tag*)current_tag->next;
    }
}

alignas(boot_resource) static char buf[sizeof(boot_resource)];
boot_resource& boot_resource::instance() { return *(boot_resource*)(buf); }

boot_resource::boot_resource(stivale2_struct* root)
{
    auto mmap_tag = stivale2_get_tag<stivale2_struct_tag_memmap>(root, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    mmap_length = mmap_tag->entries;
    for (std::size_t i = 0; i < mmap_length; i++)
        mmap_entries[i] = mmap_tag->memmap[i];
    ksize = stivale2_get_tag<stivale2_struct_tag_kernel_file_v2>(root, STIVALE2_STRUCT_TAG_KERNEL_FILE_V2_ID)->kernel_size;
    phys_addr = stivale2_get_tag<stivale2_struct_tag_kernel_base_address>(root, STIVALE2_STRUCT_TAG_KERNEL_BASE_ADDRESS_ID)
                    ->physical_base_address;
    root_table = (acpi::rsdp_descriptor*)stivale2_get_tag<stivale2_struct_tag_rsdp>(root, STIVALE2_STRUCT_TAG_RSDP_ID)->rsdp;
    auto smp = stivale2_get_tag<stivale2_struct_tag_smp>(root, STIVALE2_STRUCT_TAG_SMP_ID);
    cores = smp->cpu_count;
    bsp_id_lapic = smp->bsp_lapic_id;
}

static smp::core_local cpu0;
static smp::core_local* cpu0_ptr = &cpu0;

extern "C"
{
    extern std::uint64_t __start_init_array[];
    extern std::uint64_t __end_init_array[];

    // calls the global initialization function from the init_array sections
    // this is required to say, call the constructors of global variables
    static void init_array()
    {
        using init_array_t = void (*)();
        for (std::uint64_t* i = (std::uint64_t*)&__start_init_array; i < (std::uint64_t*)&__end_init_array;
             i++)
            if (*i != 0 && *i != -1ul)
                ((init_array_t)*i)();
    }

    [[noreturn]] void _start(stivale2_struct* root)
    {
        init_array();
        new (buf) boot_resource(root);
        wrmsr(msr::IA32_GS_BASE, (std::uint64_t)&cpu0_ptr);

        mm::init();
        paging::init();
        alloc::init((void*)config::get_val<"mmap.start.heap">,
                    paging::PAGE_SMALL_SIZE * config::get_val<"preallocate-pages">);

        tty::init(*stivale2_get_tag<stivale2_struct_tag_framebuffer>(root, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID));

        debug::print_kinfo();
        std::printf("kinit: _start() started tty\n");
        debug::dump_memory_map();
        cpuid_info::initialize_cpuglobal();
        debug::dump_cpuid_info();

        boot_resource::instance().iterate_xsdt([](const acpi::acpi_sdt_header* entry) {
            paging::map_hhdm_phys(paging::MEDIUM, (std::uint64_t)entry);
            entry = mm::make_virtual<acpi::acpi_sdt_header>((std::uint64_t)entry);
            invlpg((void*)entry);
        });

        debug::dump_acpi_info();

        // PCI time!
        // Note: this should be moved to post-smp init
        pci::scan();

        smp::core_local::create(cpu0_ptr);

        smp::init(stivale2_get_tag<stivale2_struct_tag_smp>(root, STIVALE2_STRUCT_TAG_SMP_ID));
    }
}

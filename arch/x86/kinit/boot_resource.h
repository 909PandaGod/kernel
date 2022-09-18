// cSpell:ignore kinit, stivale, lapic, rsdp, xsdt
#ifndef __KERNEL_ARCH_X86_KINIT_BOOT_RESOURCE_H__
#define __KERNEL_ARCH_X86_KINIT_BOOT_RESOURCE_H__
#include "stivale2.h"
#include <acpi/acpi.h>
#include <cstddef>
#include <cstdint>

class boot_resource
{
    std::uint64_t phys_addr;
    std::uint64_t ksize;
    std::size_t mmap_length;
    std::size_t cores;
    std::size_t bsp_id_lapic;
    stivale2_mmap_entry mmap_entries[0x100];
    acpi::rsdp_descriptor* root_table;
    bool smp_status;
public:
    boot_resource(stivale2_struct*);
    static boot_resource& instance();

    constexpr std::uint64_t kernel_phys_addr() const { return phys_addr; }
    constexpr std::uint64_t kernel_size() const { return ksize; }
    constexpr acpi::rsdp_descriptor* rsdp() const { return root_table; }
    constexpr std::size_t core_count() const { return cores; }
    constexpr std::size_t bsp_id() const { return bsp_id_lapic; }

    template <typename T>
    void iterate_mmap(T callback)
    {
        for (std::size_t i = 0; i < mmap_length; i++)
            callback(mmap_entries[i]);
    }

    template <typename T>
    void iterate_xsdt(T callback)
    {
        acpi::xsdt* table = (acpi::xsdt*)(root_table->xsdt_address + 0xffff800000000000ul);
        std::size_t n = (table->h.length - sizeof(acpi::acpi_sdt_header)) / 8;
        for (std::size_t i = 0; i < n; i++)
            callback(table->table[i]);
    }

    bool is_smp() const { return smp_status; }
    constexpr void mark_smp_start() { smp_status = true; }
};

#endif

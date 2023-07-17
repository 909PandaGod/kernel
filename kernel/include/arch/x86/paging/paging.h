// cSpell:ignore hhdm
#pragma once

#include "paging_entries.h"
#include <bitmanip.h>
#include <mm/mm.h>

namespace paging
{
    enum page_type : std::uint8_t
    {
        SMALL = 0, // 4KiB
        MEDIUM,    // 2 MiB
        BIG        // 1 GiB
    };

    inline constexpr std::size_t PAGE_SMALL_SIZE = 0x1000;
    inline constexpr std::size_t PAGE_MEDIUM_SIZE = 0x200000;
    inline constexpr std::size_t PAGE_LARGE_SIZE = 0x40000000;

    void install();

    auto map_page_for(page_table_entry* table, page_type pt, std::uintptr_t virtual_addr, std::uintptr_t physical_addr, page_prop prop, bool overwrite) -> bool;
    auto request_page(page_type pt, std::uint64_t vaddr, std::uint64_t paddr, page_prop prop = {}, bool overwrite = false) -> bool;
    void map_section(std::uint64_t addr, std::uint64_t len, paging::page_prop prop);
    void init();

    template <std::uint8_t t>
    constexpr auto get_page_entry(std::uint64_t virtual_addr) -> std::uint16_t
    {
        return std::get_bits<(3 - t) * 9 + 12, (3 - t) * 9 + 20>(virtual_addr) >> ((3 - t) * 9 + 12);
    }

    constexpr auto get_page_entry(std::uint64_t virtual_addr, std::uint8_t t) -> std::uint16_t
    {
        return std::get_bits(virtual_addr, (3 - t) * 9 + 12, (3 - t) * 9 + 20) >> ((3 - t) * 9 + 12);
    }

    inline auto map_hhdm_phys(page_type pt, std::uint64_t paddr, page_prop prop = {}, bool overwrite = false) -> bool
    {
        return request_page(pt, mm::make_virtual(paddr), paddr, prop, overwrite);
    }

    inline auto map_hhdm_virt(page_type pt, std::uint64_t vaddr, page_prop prop = {}, bool overwrite = false) -> bool
    {
        return request_page(pt, mm::make_physical(vaddr), vaddr, prop, overwrite);
    }

    void sync_page_tables(std::size_t dest_core, std::size_t src_core);
    void copy_kernel_page_tables(page_table_entry* dest, const page_table_entry* src);
} // namespace paging
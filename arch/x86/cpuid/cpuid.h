#pragma once

#include <cstddef>

namespace cpuid_info
{
    // cSpell:disable
    static constexpr const char* FEATURE_STRINGS[] = {
        "fpu",
        "vme",
        "de",
        "pse",
        "tsc",
        "msr",
        "pae",
        "mce",
        "cx8",
        "apic",
        nullptr,
        "sep",
        "mtrr",
        "pge",
        "mca",
        "cmov",
        "pat",
        "pse-36",
        "psn",
        "clfsh",
        nullptr,
        "ds",
        "acpi",
        "mmx",
        "fxsr",
        "sse",
        "sse2",
        "ss",
        "htt",
        "tm",
        "ia64",
        "pbe",
        "sse3",
        "pclmulqdq",
        "dtes64",
        "monitor",
        "ds-cpl",
        "vmx",
        "smx",
        "est",
        "tm2",
        "ssse3",
        "cnxt-id",
        "sdbg",
        "fma",
        "cx16",
        "xtpr",
        "pdcm",
        nullptr,
        "pcid",
        "dca",
        "sse4.1",
        "sse4.2",
        "x2apic",
        "movbe",
        "popcnt",
        "tsc-deadline",
        "aes",
        "fsave",
        "osxsave",
        "avx",
        "f16c",
        "rdn",
        "hypervisor",
        "fsgsbase",
        "IA32_TSC_ADJUST",
        "sgx",
        "bmi1",
        "hle",
        "avx2",
        "FDP_EXCEPT_ONLY",
        "smep",
        "bmi2",
        "erms",
        "invalid",
        "rtm",
        "pqm",
        "what the fuck",
        "mpx",
        "pqe",
        "avx512_f",
        "avx512_dq",
        "rdseed",
        "adx",
        "smap",
        "avx512_ifma",
        "pcommit",
        "clflushopt",
        "clwb",
        "intel_pt",
        "avx512_pf",
        "avx512_er",
        "avx512_cd",
        "sha",
        "avx512_bw",
        "avx512_vl",
        "prefetchwt1",
        "avx512_vbmi",
        "umip",
        "pku",
        "ospke",
        "waitpkg",
        "avx512_vbmi2",
        "cet_ss",
        "gfni",
        "vaes",
        "vpclmulqdq",
        "avx512_vnni",
        "avx512_bitalg",
        "TME_EN",
        "avx512_vpopcntdq",
        nullptr,
        "pml5",
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        "rdpid",
        "KL",
        nullptr,
        "cldemote",
        nullptr,
        "MOVDIRI",
        "MOVDIR64B",
        "ENQCMD",
        "sgx_lc",
        "pks",
        nullptr,
        nullptr,
        "avx512_4vnniw",
        "avx512_4fmaps",
        "fsrm",
        nullptr,
        nullptr,
        nullptr,
        "avx512_vp2intersect",
        "SRBDS_CTRL",
        "md_clear",
        "RTM_ALWAYS_ABORT",
        nullptr,
        "TSX_FORCE_ABORT",
        "SERIALIZE",
        "Hybrid",
        "TSXLDTRK",
        nullptr,
        "pconfig",
        "lbr",
        "cet_ibt",
        nullptr,
        "amx-bf16",
        "AVX512_FP16",
        "amx-tile",
        "amx-int8",
        "spec_ctrl",
        "stibp",
        "L1D_FLUSH",
        "IA32_ARCH_CAPABILITIES",
        "IA32_CORE_CAPABILITIES",
        "ssbd",
    };
    // cSpell:enable

    /// \brief Initializes the caches for cpu-global cpuid based information
    ///
    void initialize_cpuglobal();

    /// \brief Obtains the cpu vendor string
    /// \return The cpu vendor string
    const char* cpu_vendor_string();

    /// \brief Obtains the cpu brand string
    /// \return The cpu brand string
    const char* cpu_brand_string();

    /// \brief Check to see if a cpu feature is present
    /// \param feature The cpu feature ID to test
    /// \return Wether or not the feature is present
    bool test_feature(std::size_t feature);
}; // namespace cpuid_info

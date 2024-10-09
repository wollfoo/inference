if (WITH_DEEPPREDICTOR)
    add_definitions(/DXMRIG_ALGO_DEEPPREDICTOR)
    set(WITH_ARGON2 ON)

    list(APPEND HEADERS_CRYPTO
        src/crypto/rx/Rx.h
        src/crypto/rx/RxAlgo.h
        src/crypto/rx/RxBasicStorage.h
        src/crypto/rx/RxCache.h
        src/crypto/rx/RxConfig.h
        src/crypto/rx/RxDataset.h
        src/crypto/rx/RxQueue.h
        src/crypto/rx/RxSeed.h
        src/crypto/rx/RxVm.h
    )

    list(APPEND SOURCES_CRYPTO
        src/crypto/deeppredictor/aes_hash.cpp
        src/crypto/deeppredictor/allocator.cpp
        src/crypto/deeppredictor/blake2_generator.cpp
        src/crypto/deeppredictor/blake2/blake2b.c
        src/crypto/deeppredictor/bytecode_machine.cpp
        src/crypto/deeppredictor/dataset.cpp
        src/crypto/deeppredictor/instructions_portable.cpp
        src/crypto/deeppredictor/deeppredictor.cpp
        src/crypto/deeppredictor/reciprocal.c
        src/crypto/deeppredictor/soft_aes.cpp
        src/crypto/deeppredictor/superscalar.cpp
        src/crypto/deeppredictor/virtual_machine.cpp
        src/crypto/deeppredictor/virtual_memory.cpp
        src/crypto/deeppredictor/vm_compiled_light.cpp
        src/crypto/deeppredictor/vm_compiled.cpp
        src/crypto/deeppredictor/vm_interpreted_light.cpp
        src/crypto/deeppredictor/vm_interpreted.cpp
        src/crypto/rx/Rx.cpp
        src/crypto/rx/RxAlgo.cpp
        src/crypto/rx/RxBasicStorage.cpp
        src/crypto/rx/RxCache.cpp
        src/crypto/rx/RxConfig.cpp
        src/crypto/rx/RxDataset.cpp
        src/crypto/rx/RxQueue.cpp
        src/crypto/rx/RxVm.cpp
    )

    if (WITH_ASM AND CMAKE_C_COMPILER_ID MATCHES MSVC)
        enable_language(ASM_MASM)
        list(APPEND SOURCES_CRYPTO
             src/crypto/deeppredictor/jit_compiler_x86_static.asm
             src/crypto/deeppredictor/jit_compiler_x86.cpp
            )
    elseif (WITH_ASM AND NOT XMRIG_ARM AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND SOURCES_CRYPTO
             src/crypto/deeppredictor/jit_compiler_x86_static.S
             src/crypto/deeppredictor/jit_compiler_x86.cpp
            )
        # cheat because cmake and ccache hate each other
        set_property(SOURCE src/crypto/deeppredictor/jit_compiler_x86_static.S PROPERTY LANGUAGE C)
    elseif (XMRIG_ARM AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND SOURCES_CRYPTO
             src/crypto/deeppredictor/jit_compiler_a64_static.S
             src/crypto/deeppredictor/jit_compiler_a64.cpp
            )
        # cheat because cmake and ccache hate each other
        if (CMAKE_GENERATOR STREQUAL Xcode)
            set_property(SOURCE src/crypto/deeppredictor/jit_compiler_a64_static.S PROPERTY LANGUAGE ASM)
        else()
            set_property(SOURCE src/crypto/deeppredictor/jit_compiler_a64_static.S PROPERTY LANGUAGE C)
        endif()
    else()
        list(APPEND SOURCES_CRYPTO
             src/crypto/deeppredictor/jit_compiler_fallback.cpp
            )
    endif()

    if (WITH_SSE4_1)
        list(APPEND SOURCES_CRYPTO src/crypto/deeppredictor/blake2/blake2b_sse41.c)

        if (CMAKE_C_COMPILER_ID MATCHES GNU OR CMAKE_C_COMPILER_ID MATCHES Clang)
            set_source_files_properties(src/crypto/deeppredictor/blake2/blake2b_sse41.c PROPERTIES COMPILE_FLAGS "-Ofast -msse4.1")
        endif()
    endif()

    if (WITH_AVX2)
        list(APPEND SOURCES_CRYPTO src/crypto/deeppredictor/blake2/avx2/blake2b_avx2.c)

        if (CMAKE_C_COMPILER_ID MATCHES GNU OR CMAKE_C_COMPILER_ID MATCHES Clang)
            set_source_files_properties(src/crypto/deeppredictor/blake2/avx2/blake2b_avx2.c PROPERTIES COMPILE_FLAGS "-Ofast -mavx2")
        endif()
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
        set_source_files_properties(src/crypto/deeppredictor/jit_compiler_x86.cpp PROPERTIES COMPILE_FLAGS -Wno-unused-const-variable)
    endif()

    if (WITH_HWLOC)
        list(APPEND HEADERS_CRYPTO
             src/crypto/rx/RxNUMAStorage.h
            )

        list(APPEND SOURCES_CRYPTO
             src/crypto/rx/RxNUMAStorage.cpp
            )
    endif()

    if (WITH_MSR AND NOT XMRIG_ARM AND CMAKE_SIZEOF_VOID_P EQUAL 8 AND (XMRIG_OS_WIN OR XMRIG_OS_LINUX))
        add_definitions(/DXMRIG_FEATURE_MSR)
        add_definitions(/DXMRIG_FIX_RYZEN)
        message("-- WITH_MSR=ON")

        if (XMRIG_OS_WIN)
            list(APPEND SOURCES_CRYPTO
                src/crypto/rx/RxFix_win.cpp
                src/hw/msr/Msr_win.cpp
                )
        elseif (XMRIG_OS_LINUX)
            list(APPEND SOURCES_CRYPTO
                src/crypto/rx/RxFix_linux.cpp
                src/hw/msr/Msr_linux.cpp
                )
        endif()

        list(APPEND HEADERS_CRYPTO
            src/crypto/rx/RxFix.h
            src/crypto/rx/RxMsr.h
            src/hw/msr/Msr.h
            src/hw/msr/MsrItem.h
            )

        list(APPEND SOURCES_CRYPTO
            src/crypto/rx/RxMsr.cpp
            src/hw/msr/Msr.cpp
            src/hw/msr/MsrItem.cpp
            )
    else()
        remove_definitions(/DXMRIG_FEATURE_MSR)
        remove_definitions(/DXMRIG_FIX_RYZEN)
        message("-- WITH_MSR=OFF")
    endif()

    if (WITH_PROFILING)
        add_definitions(/DXMRIG_FEATURE_PROFILING)

        list(APPEND HEADERS_CRYPTO src/crypto/rx/Profiler.h)
        list(APPEND SOURCES_CRYPTO src/crypto/rx/Profiler.cpp)
    endif()
else()
    remove_definitions(/DXMRIG_ALGO_DEEPPREDICTOR)
endif()

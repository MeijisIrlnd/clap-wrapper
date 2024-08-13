function(guarantee_clap)
    if(TARGET
       base-sdk-clap
        )
        return()
    endif()

    include(FetchContent)

    FetchContent_Declare(
        clap
        GIT_REPOSITORY "https://github.com/free-audio/clap.git"
        GIT_TAG "1.2.0"
        GIT_SHALLOW ON
        SOURCE_SUBDIR
        "NULL"
        )

    FetchContent_MakeAvailable(clap)

    add_library(
        base-sdk-clap
        INTERFACE
        )

    target_include_directories(base-sdk-clap INTERFACE "${clap_SOURCE_DIR}/include")
endfunction()

function(guarantee_vst3)
    if(TARGET
       base-sdk-vst3
        )
        return()
    endif()

    include(FetchContent)

    FetchContent_Declare(
        vst3sdk
        GIT_REPOSITORY "https://github.com/steinbergmedia/vst3sdk.git"
        GIT_TAG "v3.7.6_build_18"
        GIT_SHALLOW ON
        SOURCE_SUBDIR
        "NULL"
        )

    FetchContent_MakeAvailable(vst3sdk)

    add_library(base-sdk-vst3 STATIC)
    file(GLOB VST3_GLOB
        "${vst3sdk_SOURCE_DIR}/base/source/*.cpp"
        "${vst3sdk_SOURCE_DIR}/base/thread/source/*.cpp"
        "${vst3sdk_SOURCE_DIR}/public.sdk/source/common/*.cpp"
        "${vst3sdk_SOURCE_DIR}/plugininterfaces/base/*.cpp")
    if (UNIX AND NOT APPLE AND ${SDKVERSION} VERSION_LESS 3.7.9)
        # Sigh - VST3 SDK before 3.7.9 ships with non-working timer code
        get_filename_component(full_path_test_cpp ${VST3_SDK_ROOT}/base/source/timer.cpp ABSOLUTE)
        list(REMOVE_ITEM VST3_GLOB "${full_path_test_cpp}")
    endif()
endfunction()

function(guarantee_wil)
    if(TARGET
       base-sdk-wil
        )
        return()
    endif()

    include(FetchContent)

    FetchContent_Declare(
        wil
        GIT_REPOSITORY "https://github.com/microsoft/wil.git"
        GIT_TAG "v1.0.240803.1"
        GIT_SHALLOW ON
        SOURCE_SUBDIR
        "NULL"
        )

    FetchContent_MakeAvailable(wil)

    add_library(
        base-sdk-wil
        INTERFACE
        )

    target_include_directories(base-sdk-wil INTERFACE "${wil_SOURCE_DIR}/include")
endfunction()

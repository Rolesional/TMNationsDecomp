add_library(tmnf_core STATIC
    src/compression/lzo.cpp
    src/content/block_assets.cpp
    src/content/materials.cpp
    src/content/track_scene.cpp
    src/content/track_collision.cpp
    src/crypto/blowfish.cpp
    src/crypto/md5.cpp
    src/gbx/campaign.cpp
    src/gbx/challenge.cpp
    src/gbx/block_info.cpp
    src/gbx/class_registry.cpp
    src/gbx/body.cpp
    src/gbx/document.cpp
    src/gbx/header.cpp
    src/gbx/solid.cpp
    src/gbx/span_input.cpp
    src/gameplay/vehicle.cpp
    src/image/dds.cpp
    src/image/tga.cpp
    src/pak/archive.cpp
    src/pak/cipher.cpp
    src/pak/cipher_recovery.cpp
    src/pak/file_data.cpp
    src/pak/header.cpp
    src/pak/pack_list.cpp
    src/pak/vfs.cpp
    src/platform/audio.cpp
    src/platform/input.cpp
)

target_include_directories(tmnf_core PUBLIC include)
target_compile_features(tmnf_core PUBLIC cxx_std_20)

if(ANDROID)
    target_link_libraries(tmnf_core PUBLIC z)
else()
    if(NOT TMNF_ZLIB_LIBRARY)
        find_library(TMNF_ZLIB_LIBRARY NAMES z zlib REQUIRED)
    endif()
    target_link_libraries(tmnf_core PUBLIC "${TMNF_ZLIB_LIBRARY}")
endif()

if(MSVC)
    target_compile_options(tmnf_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(tmnf_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

cmake_minimum_required(VERSION 3.22.1)
project(tmnf_recovery_tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core)

add_executable(gbx_inspect tools/gbx_inspect.cpp)
target_link_libraries(gbx_inspect PRIVATE tmnf_core)

add_executable(gbx_body_inspect tools/gbx_body_inspect.cpp)
target_link_libraries(gbx_body_inspect PRIVATE tmnf_core)

add_executable(challenge_inspect tools/challenge_inspect.cpp)
target_link_libraries(challenge_inspect PRIVATE tmnf_core)

add_executable(campaign_inspect tools/campaign_inspect.cpp)
target_link_libraries(campaign_inspect PRIVATE tmnf_core)

add_executable(pak_inspect tools/pak_inspect.cpp)
target_link_libraries(pak_inspect PRIVATE tmnf_core)

add_executable(packlist_inspect tools/packlist_inspect.cpp)
target_link_libraries(packlist_inspect PRIVATE tmnf_core)

add_executable(archive_inspect tools/archive_inspect.cpp)
target_link_libraries(archive_inspect PRIVATE tmnf_core)

add_executable(archive_extract tools/archive_extract.cpp)
target_link_libraries(archive_extract PRIVATE tmnf_core)

add_executable(archive_gbx_probe tools/archive_gbx_probe.cpp)
target_link_libraries(archive_gbx_probe PRIVATE tmnf_core)

add_executable(archive_gbx_prefix tools/archive_gbx_prefix.cpp)
target_link_libraries(archive_gbx_prefix PRIVATE tmnf_core)

add_executable(archive_recover tools/archive_recover.cpp)
target_link_libraries(archive_recover PRIVATE tmnf_core)

add_executable(archive_solid_probe tools/archive_solid_probe.cpp)
target_link_libraries(archive_solid_probe PRIVATE tmnf_core)

add_executable(archive_material_probe tools/archive_material_probe.cpp)
target_link_libraries(archive_material_probe PRIVATE tmnf_core)

add_executable(archive_block_assets tools/archive_block_assets.cpp)
target_link_libraries(archive_block_assets PRIVATE tmnf_core)

add_executable(archive_track_scene tools/archive_track_scene.cpp)
target_link_libraries(archive_track_scene PRIVATE tmnf_core)

add_executable(image_inspect tools/image_inspect.cpp)
target_link_libraries(image_inspect PRIVATE tmnf_core)

add_executable(gbx_header_test tests/gbx_header_test.cpp)
target_link_libraries(gbx_header_test PRIVATE tmnf_core)

add_executable(pak_header_test tests/pak_header_test.cpp)
target_link_libraries(pak_header_test PRIVATE tmnf_core)

add_executable(pack_list_test tests/pack_list_test.cpp)
target_link_libraries(pack_list_test PRIVATE tmnf_core)

add_executable(blowfish_test tests/blowfish_test.cpp)
target_link_libraries(blowfish_test PRIVATE tmnf_core)

add_executable(input_state_test tests/input_state_test.cpp)
target_link_libraries(input_state_test PRIVATE tmnf_core)

add_executable(audio_ring_buffer_test tests/audio_ring_buffer_test.cpp)
target_link_libraries(audio_ring_buffer_test PRIVATE tmnf_core)

add_executable(vfs_path_test tests/vfs_path_test.cpp)
target_link_libraries(vfs_path_test PRIVATE tmnf_core)

add_executable(image_decode_test tests/image_decode_test.cpp)
target_link_libraries(image_decode_test PRIVATE tmnf_core)

add_executable(lzo_decode_test tests/lzo_decode_test.cpp)
target_link_libraries(lzo_decode_test PRIVATE tmnf_core)

add_executable(challenge_parse_test tests/challenge_parse_test.cpp)
target_link_libraries(challenge_parse_test PRIVATE tmnf_core)

add_executable(block_info_test tests/block_info_test.cpp)
target_link_libraries(block_info_test PRIVATE tmnf_core)

add_executable(block_assets_test tests/block_assets_test.cpp)
target_link_libraries(block_assets_test PRIVATE tmnf_core)

add_executable(track_collision_test tests/track_collision_test.cpp)
target_link_libraries(track_collision_test PRIVATE tmnf_core)

add_executable(vehicle_simulation_test tests/vehicle_simulation_test.cpp)
target_link_libraries(vehicle_simulation_test PRIVATE tmnf_core)

add_executable(race_start_test tests/race_start_test.cpp)
target_link_libraries(race_start_test PRIVATE tmnf_core)

add_executable(race3_camera_test tests/race3_camera_test.cpp)
target_link_libraries(race3_camera_test PRIVATE tmnf_core)

add_executable(legacy_string_view_test tests/legacy_string_view_test.cpp)
target_link_libraries(legacy_string_view_test PRIVATE tmnf_core)

enable_testing()
add_test(NAME gbx_header_fixture COMMAND gbx_header_test)
add_test(NAME pak_header_fixture COMMAND pak_header_test)
add_test(NAME pack_list_fixture COMMAND pack_list_test)
add_test(NAME blowfish_vector COMMAND blowfish_test)
add_test(NAME input_state COMMAND input_state_test)
add_test(NAME audio_ring_buffer COMMAND audio_ring_buffer_test)
add_test(NAME vfs_path COMMAND vfs_path_test)
add_test(NAME image_decode COMMAND image_decode_test)
add_test(NAME lzo_decode COMMAND lzo_decode_test)
add_test(NAME challenge_parse COMMAND challenge_parse_test)
add_test(NAME block_info COMMAND block_info_test)
add_test(NAME block_assets COMMAND block_assets_test)
add_test(NAME track_collision COMMAND track_collision_test)
add_test(NAME vehicle_simulation COMMAND vehicle_simulation_test)
add_test(NAME race_start COMMAND race_start_test)
add_test(NAME race3_camera COMMAND race3_camera_test)
add_test(NAME legacy_string_view COMMAND legacy_string_view_test)

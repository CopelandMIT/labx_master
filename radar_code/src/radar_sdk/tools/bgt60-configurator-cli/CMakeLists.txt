add_executable(bgt60-configurator-cli bgt60-configurator-cli.cpp)
target_link_libraries(bgt60-configurator-cli sdk_radar sdk_avian BGT60TR13C_common argparse)
cmake_minimum_required(VERSION 3.24)

project(
    jiwoo
    VERSION 0.1.0
    DESCRIPTION "Automatic memory management of containers of array pointers"
    LANGUAGES CXX
)

add_library(jiwoo INTERFACE)
add_library(ltla::jiwoo ALIAS jiwoo)

target_compile_features(jiwoo INTERFACE cxx_std_17)

include(GNUInstallDirs)
target_include_directories(jiwoo INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ltla_jiwoo>"
)

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(JIWOO_TESTS "Build jiwoo's test suite." ON)
else()
    option(JIWOO_TESTS "Build jiwoo's test suite." OFF)
endif()

if(JIWOO_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()

# Installing for find_package.
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ltla_jiwoo)

install(TARGETS jiwoo
    EXPORT jiwooTargets)

install(EXPORT jiwooTargets
    FILE ltla_jiwooTargets.cmake
    NAMESPACE ltla::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_jiwoo)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_jiwooConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_jiwoo)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ltla_jiwooConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ltla_jiwooConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_jiwooConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_jiwoo)

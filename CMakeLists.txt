cmake_minimum_required(VERSION 3.14)

project(default_midas_unpacking_pipeline_plugin VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Require MIDASSYS environment
if(NOT DEFINED ENV{MIDASSYS})
  message(FATAL_ERROR
    "Environment variable MIDASSYS not set.\n"
    "Please source your MIDAS environment or run the detect_environment.sh script."
  )
endif()

set(MIDASSYS_INCLUDE_DIRS
  $ENV{MIDASSYS}/include
  $ENV{MIDASSYS}/midasio
)
set(MIDASSYS_LIB_DIR $ENV{MIDASSYS}/lib)

# Find dependencies
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(AnalysisPipelineStagesCore REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
include(${ROOT_USE_FILE})

# Gather source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Gather stage headers for ROOT dictionary (exclude LinkDef.h)
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Setup include directories including MIDAS
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<TARGET_PROPERTY:AnalysisPipelineStagesCore::analysis_pipeline_stages_core,INTERFACE_INCLUDE_DIRECTORIES>
)

# Add MIDAS include directories
foreach(dir IN LISTS MIDASSYS_INCLUDE_DIRS)
  target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${dir}>
    $<INSTALL_INTERFACE:include/midas>
  )
endforeach()

# Link against core, ROOT, and MIDAS libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        AnalysisPipelineStagesCore::analysis_pipeline_stages_core
        ROOT::Core
        ROOT::RIO
        ROOT::Tree
        ROOT::Hist
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        ${MIDASSYS_LIB_DIR}/libmfe.a
        ${MIDASSYS_LIB_DIR}/libmidas.a
)

# ROOT dictionary generation setup
set(ROOT_LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h)

# Prepare MIDAS include flags for dictionary generation
set(MIDAS_INCLUDE_FLAGS "")
foreach(dir IN LISTS MIDASSYS_INCLUDE_DIRS)
  list(APPEND MIDAS_INCLUDE_FLAGS "-I${dir}")
endforeach()

# For ROOT dictionary, only use concrete paths that exist at configure time
set(ROOT_DICTIONARY_INCLUDE_OPTIONS
    "-I${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${MIDAS_INCLUDE_FLAGS}
)

# Try to get core include directories if available
get_target_property(CORE_INCLUDE_DIRS AnalysisPipelineStagesCore::analysis_pipeline_stages_core INTERFACE_INCLUDE_DIRECTORIES)
if(CORE_INCLUDE_DIRS AND NOT CORE_INCLUDE_DIRS STREQUAL "CORE_INCLUDE_DIRS-NOTFOUND")
    foreach(dir IN LISTS CORE_INCLUDE_DIRS)
        # Only add non-generator expression paths
        if(NOT dir MATCHES "^\\$<")
            if(EXISTS "${dir}")
                list(APPEND ROOT_DICTIONARY_INCLUDE_OPTIONS "-I${dir}")
            endif()
        endif()
    endforeach()
endif()

# ROOT dictionary generation with MIDAS includes and all stage headers
ROOT_GENERATE_DICTIONARY(G__${PROJECT_NAME}
  ${ALL_STAGE_HEADERS}
  LINKDEF ${ROOT_LINKDEF}
  OPTIONS
    ${ROOT_DICTIONARY_INCLUDE_OPTIONS}
    -Wno-inconsistent-missing-override
)

# Add the generated dictionary to the target
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/G__${PROJECT_NAME}.cxx
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}  # generated headers
)

# Create alias target for CPM compatibility
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Install targets and headers
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install ROOT dictionary files alongside the library
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap
  ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
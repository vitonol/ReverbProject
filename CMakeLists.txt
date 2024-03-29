cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)

set(PROJECT_NAME "ReverbProject")

project(ReverbProject VERSION 1.0.0)

include(FetchContent)
set(FETCH_CONTENT_QUIET OFF)
FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG origin/master
        GIT_SHALLOW ON
        )
FetchContent_MakeAvailable(JUCE)

# Used for the MacOS bundle name and Installers
set(COMPANY_NAME "vitonol")

# Used for the MacOS bundle identifier (and signing)
set(BUNDLE_ID "com.vitonol.vitonol")

set(FORMATS Standalone AU VST3 AUv3)

juce_add_plugin( "${PROJECT_NAME}"
    PLUGIN_MANUFACTURER_CODE VTNL
    PLUGIN_CODE V001
    FORMATS FORMATS "${FORMATS}"
    COMPANY_NAME "${COMPANY_NAME}"
    PRODUCT_NAME "ReverbProject"
    VERSION "1.0.0" 
    PRODUCT_NAME "${PRODUCT_NAME}"

    # On MacOS, plugin is copied to ~/Users/yourname/Library/Audio/Plug-Ins/
#     COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(ReverbProject)

add_subdirectory(source)

# this is so the files aren't flat if you open in visual studio proper or whatnot
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_SOURCE})

target_sources(ReverbProject PRIVATE
        ${PROJECT_SOURCE}
)

target_include_directories(ReverbProject PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# The VERSION thing here is nice because it's defined in your project
target_compile_definitions(ReverbProject PUBLIC
        #[[ JUCE_ASIO=1 ]]
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        ReverbProject_VERSION="${CMAKE_PROJECT_VERSION}"
        PRODUCT_NAME_WITHOUT_VERSION="ReverbProject"
)

# MacOS only: Cleans up folder and target organization on Xcode.
# include(XcodePrettify)


juce_add_binary_data(BinaryData SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/placeholder.txt
)
        
target_link_libraries(ReverbProject PRIVATE
        BinaryData
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
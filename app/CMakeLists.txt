cmake_minimum_required(VERSION 3.15)

# Add source files
file(GLOB_RECURSE SOURCES 
    "*.cpp"
)
file(GLOB_RECURSE HEADERS 
    "*.h" 
)
file(GLOB_RECURSE RESOURCES 
    "*.rc"
)

# Main executable (WIN32 hides console)
add_executable(vds WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Target Windows 10 (0x0A00) or later
target_link_libraries(vds PRIVATE core shlwapi)
target_link_libraries(vds PRIVATE core user32 gdi32 d2d1 shell32 Shcore shlwapi advapi32)

target_compile_definitions(vds PRIVATE 
        WIN32_LEAN_AND_MEAN 
        NOMINMAX 
        UNICODE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
)

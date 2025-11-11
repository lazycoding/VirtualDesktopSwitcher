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

add_library(core SHARED ${SOURCES} ${HEADERS} ${RESOURCES})

# Include directories
target_include_directories(core 
PUBLIC  include
PRIVATE src 
)

target_compile_definitions(core PUBLIC BUILDING_DLL)

# Windows specific libraries
if(WIN32)
    # Target Windows 10 (0x0A00) or later
    target_link_libraries(core PRIVATE 
        user32 gdi32 d2d1 shell32 Shcore shlwapi advapi32)  # Removed gdiplus, using standard GDI
    target_compile_definitions(core PRIVATE 
        WIN32_LEAN_AND_MEAN 
        NOMINMAX 
        UNICODE
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
        _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
    )
endif()



cmake_minimum_required(VERSION 3.16)
project(TinyShell C CXX)

# ── C standard ──────────────────────────────────────────────────────
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

# ── Source files ─────────────────────────────────────────────────────
# Shell logic (C files)
set(SHELL_SOURCES
    src/main.c
    src/globals.c
    src/commands.c
    src/utils.c
)

# ImGui core (C++ files bundled in third_party/)
set(IMGUI_DIR third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
    ${IMGUI_DIR}/imgui_impl_dx9.cpp
)

# Our ImGui UI wrapper (C++)
set(UI_SOURCES
    src/ui.cpp
)

# ── Executable ───────────────────────────────────────────────────────
add_executable(tiny-shell
    ${SHELL_SOURCES}
    ${IMGUI_SOURCES}
    ${UI_SOURCES}
)

# ── Include directories ──────────────────────────────────────────────
target_include_directories(tiny-shell PRIVATE
    include            # our own headers
    ${IMGUI_DIR}       # imgui.h, imconfig.h, imgui_impl_*.h — all flat here
)

# ── Windows libraries ────────────────────────────────────────────────
# d3d9    — DirectX 9 (built into every Windows install)
# dwmapi  — for DPI-aware window handling (ImGui Win32 backend uses it)
target_link_libraries(tiny-shell PRIVATE
    d3d9
    dwmapi
)

# ── Subsystem: GUI app (no console window behind it) ─────────────────
# Comment this out if you want a console window for debug output.
if(WIN32)
    set_target_properties(tiny-shell PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

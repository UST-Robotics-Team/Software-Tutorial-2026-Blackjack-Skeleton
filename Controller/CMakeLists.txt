cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
project(hw2_controller C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(${CMAKE_PROJECT_NAME})
add_subdirectory(cmake/stm32cubemx)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/lcd.c
    Core/Src/lcd_graphics.c
    Core/Src/cards.c
    Core/Src/player_controller.c
    Core/Src/hw.c
)
target_link_libraries(${CMAKE_PROJECT_NAME} stm32cubemx)
# The small controller has 32 KB flash. Keep supplied application code compact.
set_source_files_properties(Core/Src/cards.c Core/Src/player_controller.c Core/Src/hw.c
    PROPERTIES COMPILE_OPTIONS "-Os")

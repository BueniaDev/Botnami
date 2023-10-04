project(botnami-test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL_MAIN_HANDLED")

set(IMGUI_SOURCES
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_impl_sdlrenderer2.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
	imgui/
	${SDL2_INCLUDE_DIRS})

if (WIN32)
    target_link_libraries(imgui PUBLIC imm32)
endif()

if (TARGET SDL2::SDL2)
    target_link_libraries(imgui PUBLIC SDL2::SDL2)
else()
    target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})
endif()

set(EXAMPLE_SOURCES
	m6820.cpp
	m6850.cpp
	main.cpp)

add_executable(${PROJECT_NAME} ${EXAMPLE_SOURCES})
target_link_libraries(${PROJECT_NAME} botnami6809 imgui)

find_package(SDL2 REQUIRED)

if (TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2)
else()
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()
project(botnami-tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TEST_SOURCES 
	main.cpp)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

include_directories(${BOTNAMI_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} libbotnami)
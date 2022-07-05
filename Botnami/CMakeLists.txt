set(BOTNAMI_HEADERS
	botnami_api.h
	botnami.h)

set(BOTNAMI_SOURCES
	botnami.cpp
	botnami6800.cpp
	botnami6809.cpp
	botnami6309.cpp
	botnamikonami.cpp)

add_library(botnami ${BOTNAMI_SOURCES} ${BOTNAMI_HEADERS})
target_include_directories(botnami PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(libbotnami ALIAS botnami)
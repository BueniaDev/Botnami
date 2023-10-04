set(BOTNAMI6809_HEADERS
	botnami_api.h
	botnami6809.h)

set(BOTNAMI6809_SOURCES
	botnami6809.cpp)

add_library(botnami6809 ${BOTNAMI6809_SOURCES} ${BOTNAMI6809_HEADERS})
target_include_directories(botnami6809 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(libbotnami6809 ALIAS botnami6809)
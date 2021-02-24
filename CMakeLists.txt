cmake_minimum_required(VERSION 3.3)
project(QSE)

# Defining Defaults
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")

# Defining Project Structure
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
include_directories("include")
include_directories("lib")


#message(WARNING "${PROJECT_BINARY_DIR}")
#message(WARNING "CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
#message(WARNING "CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")


set(SOURCE_FILES_SEARCH
        src/QSE-Search.cpp
        include/Util.h
        include/IndexHandler.h src/IndexHandler.cpp
        include/Searcher.h src/Searcher.cpp
        include/WebServer.h src/WebServer.cpp)

set(SOURCE_FILES_INDEXER
        src/QSE-Indexer.cpp
        include/Util.h
        lib/tinydir.h
        lib/Q_Dir.h lib/Q_Dir.cpp
        lib/Q_Buffer_Voc.h lib/Q_Buffer_Voc.cpp
        include/Parser.h src/Parser.cpp
        include/Indexer.h src/Indexer.cpp
        include/IndexHandler.h src/IndexHandler.cpp)

if (APPLE)
    find_library(htmlcxx libhtmlcxx.dylib)
else (APPLE)
    find_library(htmlcxx libhtmlcxx.a)
endif (APPLE)

find_library(tidys libtidys.a)

find_library(cppnetlib libcppnetlib-uri.a)
#find_package(cppnetlib)


add_executable(QSE-Searcher ${SOURCE_FILES_SEARCH})

add_executable(QSE-Indexer ${SOURCE_FILES_INDEXER})


target_link_libraries(QSE-Searcher ${htmlcxx})
target_link_libraries(QSE-Searcher ${tidys})

target_link_libraries(QSE-Indexer ${htmlcxx})
target_link_libraries(QSE-Indexer ${tidys})


set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system serialization thread REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    target_link_libraries(QSE-Searcher ${Boost_LIBRARIES})
    target_link_libraries(QSE-Searcher ${cppnetlib})

    target_link_libraries(QSE-Indexer ${Boost_LIBRARIES})
    #    target_link_libraries(QSE-Indexer ${cppnetlib_LIBRARIES})
endif ()

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static -Wl, -Bstatic -lpthread -Wl, -Bdynamic")
    target_link_libraries(QSE-Searcher ws2_32 wsock32 iconv)
    target_link_libraries(QSE-Indexer ws2_32 wsock32 iconv z)
endif (WIN32)

cmake_minimum_required(VERSION 3.19)
project(PictureToWood)

set(CMAKE_CXX_STANDARD 14)

set(EXECUTABLE_NAME PictureToWood)

set(SRC main.cpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

if(CMAKE_BUILD_TYPE MATCHES Release)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif ()

FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system thread)
FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ./ext/nlohmann
)

SET(LIBS
        myLib
        ${Boost_LIBRARIES}
        ${OpenCV_LIBS}
        )

ADD_EXECUTABLE(${EXECUTABLE_NAME} ${SRC})
TARGET_LINK_LIBRARIES(${EXECUTABLE_NAME} ${LIBS})

ADD_SUBDIRECTORY(libs)
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET(LIB_NAME myLib)

SET(SRC
        Picture.cpp Picture.hpp
        Cell.cpp Cell.hpp
        Find_Patches.cpp Find_Patches.hpp
        Time_Measure.cpp Time_Measure.hpp
        Patch_List.cpp Patch_List.hpp
        Pattern.cpp Pattern.hpp
        Shape.cpp Shape.hpp)

ADD_LIBRARY(${LIB_NAME} ${SRC})

TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
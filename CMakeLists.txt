cmake_minimum_required(VERSION 3.15)
project(YOLO_FPS_辅助 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(CUDA QUIET)
find_package(TensorRT QUIET)
find_package(ONNX QUIET)

include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
)

file(GLOB_RECURSE SRC_FILES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
)

if(CUDA_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CUDA_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
    add_definitions(-DWITH_CUDA)
endif()

if(TensorRT_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${TensorRT_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${TensorRT_LIBRARIES})
    add_definitions(-DWITH_TENSORRT)
endif()

if(ONNX_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${ONNX_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${ONNX_LIBRARIES})
    add_definitions(-DWITH_ONNX)
endif()
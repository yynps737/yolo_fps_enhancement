cmake_minimum_required(VERSION 3.15)
project(YOLO_FPS_ASSIST LANGUAGES CXX)

# 设置CMake策略
cmake_policy(SET CMP0146 NEW)
cmake_policy(SET CMP0148 NEW)

# 添加宏定义避免min/max宏和std::min/max冲突
add_definitions(-DNOMINMAX)
# 添加数学定义宏
add_definitions(-D_USE_MATH_DEFINES)
# 添加UTF-8编码支持
if(MSVC)
    add_compile_options("/utf-8")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置OpenCV路径 - 使用用户提供的实际安装路径
set(OpenCV_DIR "C:/opencv/opencv/build" CACHE PATH "OpenCV directory")
# 设置OpenCV bin目录，用于复制DLL
set(OpenCV_BIN_DIR "C:/opencv/opencv/build/x64/vc16/bin" CACHE PATH "OpenCV bin directory")

# 添加RapidJSON
include(FetchContent)
FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_TAG v1.1.0
)
# 禁用RapidJSON测试和示例以避免与较新C++标准的兼容性问题
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "Build rapidjson tests" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "Build rapidjson examples" FORCE)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "Build rapidjson documentation" FORCE)
FetchContent_MakeAvailable(rapidjson)

# 查找OpenCV - 使用特定的配置路径
find_package(OpenCV REQUIRED PATHS "${OpenCV_DIR}" NO_DEFAULT_PATH)

# 输出OpenCV库状态
message(STATUS "OpenCV库状态:")
message(STATUS "    版本: ${OpenCV_VERSION}")
message(STATUS "    包含路径: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "    库: ${OpenCV_LIBS}")
message(STATUS "    DLL目录: ${OpenCV_BIN_DIR}")

# 查找CUDA支持
option(WITH_CUDA "Enable CUDA support if available" ON)
if(WITH_CUDA)
    find_package(CUDA QUIET)
    if(CUDA_FOUND)
        message(STATUS "找到CUDA: ${CUDA_VERSION}")
        add_definitions(-DWITH_CUDA)
    else()
        message(STATUS "未找到CUDA，将禁用GPU加速功能")
    endif()
endif()

# 配置包含路径
include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
        ${rapidjson_SOURCE_DIR}/include
)

if(CUDA_FOUND)
    include_directories(${CUDA_INCLUDE_DIRS})
endif()

# 收集源文件
file(GLOB_RECURSE SRC_FILES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.h"
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})

# 链接库
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

if(CUDA_FOUND)
    target_link_libraries(${PROJECT_NAME} ${CUDA_LIBRARIES})
endif()

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 复制资源文件到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/resources"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/resources"
        COMMENT "复制资源文件到输出目录"
)

# 确保OpenCV的DLL可以被找到
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/opencv_dll"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${OpenCV_BIN_DIR}" "${CMAKE_BINARY_DIR}/bin/opencv_dll"
            COMMENT "复制OpenCV DLL到输出目录"
    )

    # 添加环境变量配置文件
    file(WRITE "${CMAKE_BINARY_DIR}/bin/setup_env.bat"
            "SET PATH=%~dp0opencv_dll;%PATH%\n"
            "echo OpenCV环境已设置\n"
            "echo 执行以下命令开始运行程序: \n"
            "echo %~dp0${PROJECT_NAME}.exe\n"
    )
endif()

message(STATUS "配置完成: YOLO FPS ASSIST项目")
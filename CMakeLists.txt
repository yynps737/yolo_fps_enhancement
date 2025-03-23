cmake_minimum_required(VERSION 3.15)
project(YOLO_FPS_ASSIST LANGUAGES CXX) # 修改为英文名称

# 设置CMake策略
cmake_policy(SET CMP0146 NEW)
cmake_policy(SET CMP0148 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# 设置OpenCV路径
if(DEFINED ENV{OpenCV_DIR})
    set(OpenCV_DIR $ENV{OpenCV_DIR} CACHE PATH "OpenCV directory" FORCE)
    message(STATUS "使用环境变量中的OpenCV目录: ${OpenCV_DIR}")
else()
    set(OpenCV_DIR "C:/opencv-mingw/x64/mingw/lib" CACHE PATH "OpenCV MinGW directory" FORCE)
    message(STATUS "使用默认MinGW版OpenCV，目录设置为: ${OpenCV_DIR}")
endif()

# 添加RapidJSON
include(FetchContent)
FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_TAG v1.1.0
)
FetchContent_MakeAvailable(rapidjson)

# 查找OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV库状态:")
message(STATUS "    版本: ${OpenCV_VERSION}")
message(STATUS "    包含路径: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "    库: ${OpenCV_LIBS}")

# 查找其他可选依赖 - 使用模块方式而非直接查找
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
target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
)

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
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${OpenCV_DIR}/../bin" "${CMAKE_BINARY_DIR}/bin/opencv_dll"
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
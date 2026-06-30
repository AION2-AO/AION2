cmake_minimum_required(VERSION 3.15)
project(AION2Server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# hiredis 패키지 자동 탐색
find_package(hiredis CONFIG REQUIRED)

# 소스 파일 추가
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# 실행 파일
add_executable(AION2Server ${SOURCES})

# 라이브러리 링크
target_link_libraries(AION2Server PRIVATE hiredis::hiredis)
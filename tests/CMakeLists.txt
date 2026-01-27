cmake_minimum_required(VERSION 3.15)

# 简单的测试框架（后续可以集成 Google Test）
enable_testing()

# HTTP Parser 测试
add_executable(test_http_parser test_http_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/http_parser.cpp
)
target_include_directories(test_http_parser PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME HttpParserTest COMMAND test_http_parser)

# Router 测试
add_executable(test_router test_router.cpp
    ${CMAKE_SOURCE_DIR}/src/http_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/server/router.cpp
    ${CMAKE_SOURCE_DIR}/src/logger/logger.cpp
)
target_include_directories(test_router PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME RouterTest COMMAND test_router)


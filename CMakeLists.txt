cmake_minimum_required(VERSION 3.5)

project(http_mongodb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

# add json support
find_package(nlohmann_json CONFIG REQUIRED)

# add enum support
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

find_package(task_channel CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${task_channel_include_dirs})

find_package(http_utils CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${http_utils_include_dirs})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIRS})
link_directories(${OPENSSL_LIBRARY_DIR})

find_package(Threads REQUIRED)

find_package (
  Boost 1.71 REQUIRED COMPONENTS
	system
	thread
	
	)
include_directories (${Boost_INCLUDE_DIRS})
link_directories (${Boost_LIBRARY_DIRS})
add_definitions (-DBOOST_ASIO_NO_DEPRECATED=1)
add_definitions (-DBOOST_ASIO_DISABLE_BOOST_ARRAY=1)
add_definitions (-DBOOST_ASIO_DISABLE_BOOST_BIND=1)
add_definitions (-DBOOST_ASIO_DISABLE_BOOST_DATE_TIME=1)
add_definitions (-DBOOST_ASIO_DISABLE_BOOST_REGEX=1)

find_package(mongocxx REQUIRED)

include_directories(${MONGO_INCLUDE_DIR})





file(GLOB_RECURSE COMMAND_SRC_FILES "${PROJECT_SOURCE_DIR}/src/command/*.cpp")
file(GLOB_RECURSE SERVER_SRC_FILES "${PROJECT_SOURCE_DIR}/src/server/*.cpp")

add_library(command ${COMMAND_SRC_FILES})
target_link_libraries(command nlohmann_json::nlohmann_json)
add_library(server ${SERVER_SRC_FILES})
target_link_libraries(server PUBLIC http_utils nlohmann_json::nlohmann_json task_channel mongo::mongocxx_shared mongo::bsoncxx_shared spdlog::spdlog command)

add_executable(server_test ${PROJECT_SOURCE_DIR}/test/test_server.cpp)
target_link_libraries(server_test server)

add_executable(client_test ${PROJECT_SOURCE_DIR}/test/test_client.cpp)
target_link_libraries(client_test http_utils nlohmann_json::nlohmann_json command)

foreach(p LIB BIN INCLUDE)
	set(var CMAKE_INSTALL_${p}DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		string(TOLOWER ${p} lower_p)
        set(${var} "${CMAKE_INSTALL_PREFIX}/${lower_p}")
	endif()
endforeach()

export(TARGETS command server 
     FILE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)

file(WRITE
    ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
    "include(\${CMAKE_CURRENT_LIST_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)\n")

install(FILES
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})

install(TARGETS command server EXPORT ${PROJECT_NAME}Targets
   ARCHIVE  DESTINATION lib
   LIBRARY DESTINATION lib
   RUNTIME DESTINATION bin
  )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include/${PROJECT_NAME})

install(EXPORT ${CMAKE_PROJECT_NAME}Targets
        NAMESPACE ${CMAKE_PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(roast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

add_executable(roast Server.cpp)

if(UNIX)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_link_libraries(roast PRIVATE Threads::Threads)

	find_package(OpenSSL REQUIRED)
	include_directories(${OPENSSL_INCLUDE_DIR})
	target_link_libraries(roast PRIVATE ${OPENSSL_CRYPTO_LIBRARY})
	target_link_libraries(roast PRIVATE ${OPENSSL_SSL_LIBRARY})


	target_link_libraries(roast PRIVATE MLX90640_API)
endif(UNIX)

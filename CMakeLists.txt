cmake_minimum_required(VERSION 3.0)
project(curl-based-email)

set(CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
set(PROJECT_NAME main)


add_executable(${PROJECT_NAME} main.cpp smtp_mail_sender.cpp base64.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    curl
)
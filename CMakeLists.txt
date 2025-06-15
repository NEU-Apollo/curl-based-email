cmake_minimum_required(VERSION 3.0)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    CURL::libcurl
)
cmake_minimum_required(VERSION 3.16)
project(imgbook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PODOFO REQUIRED libpodofo)
pkg_check_modules(LCMS2  REQUIRED lcms2)
pkg_check_modules(JPEG   REQUIRED libjpeg)

set(SOURCES
    src/main.cpp
    src/args.cpp
    src/image/preflight.cpp
    src/image/jpeg_loader.cpp
    src/image/cmyk_convert.cpp
    src/layout/scaled_layout.cpp
    src/layout/bleed_layout.cpp
    src/pdf/book_writer.cpp
    src/pdf/page_builder.cpp
)

add_executable(imgbook ${SOURCES})

target_compile_options(imgbook PRIVATE -Wall -Wextra -Wno-unused-parameter)

target_include_directories(imgbook PRIVATE
    src
    ${PODOFO_INCLUDE_DIRS}
    ${LCMS2_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
)

target_link_libraries(imgbook PRIVATE
    ${PODOFO_LIBRARIES}
    ${LCMS2_LIBRARIES}
    ${JPEG_LIBRARIES}
)

target_link_options(imgbook PRIVATE
    ${PODOFO_LDFLAGS}
    ${LCMS2_LDFLAGS}
    ${JPEG_LDFLAGS}
)

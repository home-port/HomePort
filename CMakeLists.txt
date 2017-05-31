# Copyright 2011 Aalborg University. All rights reserved.
#  
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 
# The views and conclusions contained in the software and
# documentation are those of the authors and should not be interpreted
# as representing official policies, either expressed.

cmake_minimum_required(VERSION 3.1)
project(homeport C CXX)

set(CMAKE_C_FLAGS "-std=gnu99 -DHPD_REST_ORIGIN")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g -DDEBUG -Wall")
set(CMAKE_CXX_FLAGS "-std=gnu++0x -DHPD_REST_ORIGIN")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG -Wall")

# -fPIC is for compiling on 64bit
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "-fPIC ${CMAKE_C_FLAGS_DEBUG}")
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

# -fPIC for cross-compiling against arm
if(CMAKE_SYSTEM_PROCESSOR STREQUAL armhf)
    set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG "-fPIC ${CMAKE_C_FLAGS_DEBUG}")
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL armhf)

# GoogleTest
find_path(GOOGLE_TEST_INCLUDE pthread.h)
include_directories(${GOOGLE_TEST_INCLUDE})
add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Http-Parser
add_library(http-parser lib/http-parser/http_parser.c)
include_directories(lib/http-parser)

# Generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(GENERATE_HTML "NO")
    set(GENERATE_LATEX "YES")
    configure_file(doxygen.conf.in doxygen-latex.conf @ONLY)
    set(GENERATE_LATEX "NO")
    set(GENERATE_HTML "YES")
    configure_file(doxygen.conf.in doxygen-html.conf @ONLY)
    set(GENERATE_LATEX "YES")
    set(GENERATE_HTML "YES")
    configure_file(doxygen.conf.in doxygen-all.conf @ONLY)
    set(GENERATE_HTML "NO")
    set(GENERATE_LATEX "NO")
    add_custom_target(doc-html
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen-html.conf
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation (HTML)" VERBATIM
            )
    add_custom_command(TARGET doc-html POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
            "Documentation at ${CMAKE_BINARY_DIR}/doc/html/index.html")
    add_custom_target(doc-latex
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen-latex.conf
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation (LaTeX)" VERBATIM
            )
    add_custom_target(doc-pdf
            COMMAND make pdf
            DEPENDS doc-latex
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/latex
            COMMENT "Generating API documentation (PDF)" VERBATIM
            )
    add_custom_target(doc-all ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen-all.conf
            COMMAND make --directory=${CMAKE_CURRENT_BINARY_DIR}/doc/latex pdf
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation (HTML, LaTeX, PDF)" VERBATIM
            )
    add_custom_command(TARGET doc-all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
            "Documentation at ${CMAKE_BINARY_DIR}/doc/html/index.html")
else(DOXYGEN_FOUND)
    message(WARNING "Please install doxygen to generate documentation")
endif(DOXYGEN_FOUND)

set(THREADS_PREFER_PTHREAD_FLAG ON)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL armhf)
    set(THREADS_PTHREAD_ARG 2)
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL armhf)
find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    add_definitions(-DTHREAD_SAFE)
endif (CMAKE_USE_PTHREADS_INIT)

include_directories(hpd/include)
include_directories(common/include)
include_directories(common/map/include)

add_library(hpd_doc SHARED
        main.dox
        )
set_target_properties(hpd_doc PROPERTIES LINKER_LANGUAGE C)

add_subdirectory(common)
add_subdirectory(hpd)
add_subdirectory(example)
add_subdirectory(modules)

SET(MAJOR_VERSION "1")
SET(MINOR_VERSION "0")
SET(PATCH_VERSION "0")

if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
    include(InstallRequiredSystemLibraries)

    set(CPACK_SET_DESTDIR "on")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
    set(CPACK_GENERATOR "DEB")

    set(CPACK_PACKAGE_DESCRIPTION "AAU HomePort")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HomePort middleware/glue for adapters and applications")
    set(CPACK_PACKAGE_VENDOR "CISS/AAU")
    set(CPACK_PACKAGE_CONTACT "kronborg@cs.aau.dk")
    set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
    set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
    set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
    set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${CPACK_DEBIAN_ARCHITECTURE}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${CPACK_DEBIAN_ARCHITECTURE}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")

    #dependencies for homePort
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libev4, libjansson4, libmxml1, libcurl3, libcurl3-gnutls")

    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

    set(CPACK_COMPONENTS_ALL Libraries ApplicationData)
    include(CPack)

endif(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

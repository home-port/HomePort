# Copyright 2013 Aalborg University. All rights reserved.
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

cmake_minimum_required(VERSION 2.6)
project(HOMEPORT C CXX)

set(CMAKE_C_FLAGS "-DLR_ORIGIN")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g -DDEBUG -Wall")
set(CMAKE_CXX_FLAGS "-std=gnu++0x -DLR_ORIGIN")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG -Wall")

# -fPIC is for compiling on 64bit
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "-fPIC ${CMAKE_C_FLAGS_DEBUG}")
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

# GoogleTest
add_subdirectory(lib/googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(GENERATE_HTML "NO")
    set(GENERATE_LATEX "YES")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf ${CMAKE_CURRENT_BINARY_DIR}/doxygen-latex.conf @ONLY)
    set(GENERATE_LATEX "NO")
    set(GENERATE_HTML "YES")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf ${CMAKE_CURRENT_BINARY_DIR}/doxygen-html.conf @ONLY)
    set(GENERATE_LATEX "YES")
    set(GENERATE_HTML "YES")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf ${CMAKE_CURRENT_BINARY_DIR}/doxygen-all.conf @ONLY)
    set(GENERATE_HTML "NO")
    set(GENERATE_LATEX "NO")
    add_custom_target(doc-html
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen-html.conf
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation (HTML)" VERBATIM
            )
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
else(DOXYGEN_FOUND)
    message(WARNING "Please install doxygen to generate documentation")
endif(DOXYGEN_FOUND)

add_subdirectory(api)
add_subdirectory(example)
add_subdirectory(rest)

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

# Main library
add_library(libWebserver
      instance.c
      client.c
      request.c
      response.c
      )
target_link_libraries(libWebserver ev http-parser)

# Request Test
add_executable(request_test EXCLUDE_FROM_ALL
      request_test.c
      )
target_link_libraries(request_test http-parser)
add_test(request_test ${CMAKE_CURRENT_BINARY_DIR}/request_test)
add_dependencies(check request_test)

# Response Test
add_executable(response_test EXCLUDE_FROM_ALL
      request.c
      client.c
      instance.c
      response_test.c
      )
target_link_libraries(response_test http-parser ev)
add_test(response_test ${CMAKE_CURRENT_BINARY_DIR}/response_test)
add_dependencies(check response_test)

add_subdirectory(webserver-test)

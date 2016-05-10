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
add_library(http-webserver
      http-webserver.c
      request.c
      url_parser.c
      header_parser.c
      response.c
      )
target_link_libraries(http-webserver webserver http-parser)

# URL Parser Test
# TODO OLD test deactivated, changing to googletest
# add_executable(url_parser_test EXCLUDE_FROM_ALL
#       url_parser_test.c
#       )
# add_test(url_parser_test ${CMAKE_CURRENT_BINARY_DIR}/url_parser_test)
# add_dependencies(check url_parser_test)

# Header Parser Test
# TODO OLD test deactivated, changing to googletest
# add_executable(header_parser_test EXCLUDE_FROM_ALL
#       header_parser_test.c
#       )
# add_test(header_parser_test ${CMAKE_CURRENT_BINARY_DIR}/header_parser_test)
# add_dependencies(check header_parser_test)

# Http-Webserver Test
# TODO OLD test deactivated, changing to googletest
# add_executable(http-webserver_test EXCLUDE_FROM_ALL
#       http-webserver_test.c
#       )
# target_link_libraries(http-webserver_test curl pthread http-webserver)
# add_test(http-webserver_test ${CMAKE_CURRENT_BINARY_DIR}/http-webserver_test)
# add_dependencies(check http-webserver_test)


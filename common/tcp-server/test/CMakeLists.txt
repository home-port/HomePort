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

# Webserver Test
# TODO OLD test deactivated, changing to googletest
# add_executable(webserver_test EXCLUDE_FROM_ALL
#       webserver_test.c
#       )
# target_link_libraries(webserver_test ev)
# add_test(webserver_test ${CMAKE_CURRENT_BINARY_DIR}/webserver_test)
# add_dependencies(check webserver_test)

# Big Data Test
# TODO OLD test deactivated, changing to googletest
# add_executable(webserver_big_data_test EXCLUDE_FROM_ALL
#       big_data_test.c
#       )
# target_link_libraries(webserver_big_data_test curl pthread tcp-server)
# add_test(webserver_big_data_test ${CMAKE_CURRENT_BINARY_DIR}/webserver_big_data_test)
# add_dependencies(check webserver_big_data_test)

# Load Test
# TODO OLD test deactivated, changing to googletest
# add_executable(webserver_load_test EXCLUDE_FROM_ALL
#       load_test.c
#       )
# target_link_libraries(webserver_load_test curl pthread tcp-server)
# add_test(webserver_load_test ${CMAKE_CURRENT_BINARY_DIR}/webserver_load_test)
# add_dependencies(check webserver_load_test)


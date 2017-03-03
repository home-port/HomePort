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

add_library(example_demo_adapter
        demo_adapter.h
        demo_adapter.c
        )
target_link_libraries(example_demo_adapter hpd)

add_library(example_demo_application
        demo_application.h
        demo_application.c
        )
target_link_libraries(example_demo_application hpd)

add_executable(example_demo
        demo_main.c
        tutorial02.dox
        )
if(CMAKE_SYSTEM_PROCESSOR STREQUAL armhf)
    target_link_libraries(example_demo example_demo_adapter example_demo_application hpd-rest pthread resolv)
else(CMAKE_SYSTEM_PROCESSOR STREQUAL armhf)
    target_link_libraries(example_demo example_demo_adapter example_demo_application hpd-rest)
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL armhf)
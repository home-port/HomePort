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

find_package(CURL)
if (CURL_FOUND)
    add_library(hpd-curl-ev SHARED
            ../include/hpd-0.6/common/hpd_curl_ev_module.h
            ../include/hpd-0.6/common/hpd_curl_ev.h
            curl_ev_intern.h
            curl_ev_module.c
            curl_ev.c
            )
    target_link_libraries(hpd-curl-ev ${CURL_LIBRARIES} ev)
    set_target_properties(hpd-curl-ev PROPERTIES VERSION ${HPD_VERSION_DEFAULT} SOVERSION ${HPD_SOVERSION_DEFAULT})
    install(TARGETS hpd-curl-ev LIBRARY DESTINATION ${HPD_LIB_PATH} NAMELINK_SKIP)
else (CURL_FOUND)
    message(WARNING "Please install curl to build hpd-curl-ev library")
endif(CURL_FOUND)
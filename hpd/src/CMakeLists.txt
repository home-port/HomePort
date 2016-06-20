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

add_library(daemon OBJECT
        daemon.h
        daemon.c
        daemon_api.c
        )

add_library(discovery OBJECT
        discovery.h
        discovery.c
        discovery_id.c
        discovery_api.c
        )

add_library(value OBJECT
        value.h
        value.c
        value_api.c
        )

add_library(request OBJECT
        request.h
        request.c
        request_api.c
        )

add_library(event OBJECT
        event.h
        event.c
        event_api.c
        )

add_library(log OBJECT
        log.h
        log.c
        log_api.c
        )

add_library(hpd SHARED
        $<TARGET_OBJECTS:daemon>
        $<TARGET_OBJECTS:discovery>
        $<TARGET_OBJECTS:value>
        $<TARGET_OBJECTS:request>
        $<TARGET_OBJECTS:event>
        $<TARGET_OBJECTS:log>
        model.h
        comm.h
        ../include/hpd/hpd_adapter_api.h
        ../include/hpd/hpd_api.h
        ../include/hpd/hpd_application_api.h
        ../include/hpd/hpd_daemon_api.h
        ../include/hpd/hpd_shared_api.h
        ../include/hpd/hpd_types.h
        ../intro.dox
        ../api.dox
)
set_target_properties(hpd PROPERTIES LINKER_LANGUAGE C)
set_target_properties(hpd PROPERTIES VERSION 0.0.0 SOVERSION 0)
target_link_libraries(hpd ev hpd-map)
install(TARGETS hpd DESTINATION lib)

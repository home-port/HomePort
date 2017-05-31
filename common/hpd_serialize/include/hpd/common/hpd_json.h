/*
 * Copyright 2011 Aalborg University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 */

#ifndef HOMEPORT_HPD_JSON_H
#define HOMEPORT_HPD_JSON_H

#include <hpd/hpd_types.h>
#include <hpd/common/hpd_jansson.h>

hpd_error_t hpd_json_adapter_id_to_json(const hpd_module_t *context, const hpd_adapter_id_t *adapter, json_t **out);
hpd_error_t hpd_json_adapter_to_json(const hpd_module_t *context, const hpd_adapter_id_t *adapter, json_t **out);
hpd_error_t hpd_json_adapter_to_json_shallow(const hpd_module_t *context, const hpd_adapter_id_t *adapter, json_t **out);
hpd_error_t hpd_json_adapters_to_json(const hpd_module_t *context, json_t **out);
hpd_error_t hpd_json_configuration_to_json(const hpd_module_t *context, json_t **out);
hpd_error_t hpd_json_device_id_to_json(const hpd_module_t *context, const hpd_device_id_t *device, json_t **out);
hpd_error_t hpd_json_device_to_json(const hpd_module_t *context, const hpd_device_id_t *device, json_t **out);
hpd_error_t hpd_json_device_to_json_shallow(const hpd_module_t *context, const hpd_device_id_t *device, json_t **out);
hpd_error_t hpd_json_devices_to_json(const hpd_module_t *context, const hpd_adapter_id_t *adapter, json_t **out);
hpd_error_t hpd_json_parameter_id_to_json(const hpd_module_t *context, const hpd_parameter_id_t *parameter, json_t **out);
hpd_error_t hpd_json_parameter_to_json(const hpd_module_t *context, const hpd_parameter_id_t *parameter, json_t **out);
hpd_error_t hpd_json_parameters_to_json(const hpd_module_t *context, const hpd_service_id_t *service, json_t **out);
hpd_error_t hpd_json_service_id_to_json(const hpd_module_t *context, const hpd_service_id_t *service, json_t **out);
hpd_error_t hpd_json_service_to_json(const hpd_module_t *context, const hpd_service_id_t *service, json_t **out);
hpd_error_t hpd_json_services_to_json(const hpd_module_t *context, const hpd_device_id_t *device, json_t **out);
hpd_error_t hpd_json_value_to_json(const hpd_module_t *context, const hpd_value_t *value, json_t **out);
hpd_error_t hpd_json_response_to_json(const hpd_module_t *context, const hpd_response_t *response, json_t **out);
hpd_error_t hpd_json_request_to_json(const hpd_module_t *context, const hpd_request_t *request, json_t **out);

hpd_error_t hpd_json_value_parse(const hpd_module_t *context, json_t *json, hpd_value_t **out);
hpd_error_t hpd_json_request_parse(const hpd_module_t *context, json_t *json, hpd_response_f on_response, hpd_request_t **out);

#endif //HOMEPORT_HPD_JSON_H

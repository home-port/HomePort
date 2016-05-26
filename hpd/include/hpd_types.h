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
 * THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
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

#ifndef HOMEPORT_HPD_TYPES_H
#define HOMEPORT_HPD_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef char hpd_bool_t;
#define HPD_TRUE 1
#define HPD_FALSE 0

typedef struct hpd hpd_t;
typedef struct hpd_module hpd_module_t;
typedef struct hpd_action hpd_action_t;
typedef struct hpd_listener hpd_listener_t;
typedef struct hpd_adapter hpd_adapter_t;
typedef struct hpd_device hpd_device_t;
typedef struct hpd_service hpd_service_t;
typedef struct hpd_parameter hpd_parameter_t;
typedef struct hpd_request hpd_request_t;
typedef struct hpd_response hpd_response_t;
typedef struct hpd_value hpd_value_t;
typedef struct hpd_pair hpd_pair_t;
typedef struct ev_loop hpd_ev_loop_t;
typedef struct hpd_adapter_id hpd_adapter_id_t;
typedef struct hpd_device_id hpd_device_id_t;
typedef struct hpd_service_id hpd_service_id_t;
typedef struct hpd_parameter_id hpd_parameter_id_t;

/**
 * Value to be used in len parameters on \0 terminated strings
 */
/// [HPD_NULL_TERMINATED]
#define HPD_NULL_TERMINATED -1
/// [HPD_NULL_TERMINATED]

/**
 * Errors that can be returned from functions.
 */
/// [hpd_error_t]
typedef enum hpd_error {
    HPD_E_SUCCESS = 0,  //< The operation was successful.
    HPD_E_UNKNOWN = 1,  //< An unknown error
    HPD_E_NULL,         //< Null pointer error.
    HPD_E_ALLOC,        //< Memory allocation error.
    HPD_E_STATE,        //< An object is in an invalid state.
    HPD_E_ARGUMENT,     //< An argument is invalid.
    HPD_E_NOT_UNIQUE,   //< An argument is not unique.
    HPD_E_NOT_FOUND,    //< The object could not be found.
} hpd_error_t;
/// [hpd_error_t]

/**
 * On failure data should be left as NULL.
 */
/// [hpd_module_def_t functions]
typedef hpd_error_t (*hpd_create_f)    (void **data, hpd_module_t *context);
typedef hpd_error_t (*hpd_destroy_f)   (void *data);
typedef hpd_error_t (*hpd_start_f)     (void *data, hpd_t *hpd);
typedef hpd_error_t (*hpd_stop_f)      (void *data, hpd_t *hpd);
// TODO Document that this should return HPD_E_ARGUMENT if name is not known
typedef hpd_error_t (*hpd_parse_opt_f) (void *data, const char *name, const char *arg);
/// [hpd_module_def_t functions]

/// [hpd_action_f]
typedef hpd_error_t (*hpd_action_f) (hpd_request_t *req); //< Action function for handling requests on services.
/// [hpd_action_f]

/// [hpd_free_f]
typedef hpd_error_t (*hpd_free_f) (void *data); //< Free function, used to free user supplied data.
/// [hpd_free_f]

/// Response function for handing responses on requests.
/// [Application API Callbacks]
typedef hpd_error_t (*hpd_response_f) (hpd_response_t *res);
/// Value callback for listeners
typedef hpd_error_t (*hpd_value_f) (void *data, const hpd_service_id_t *service, const hpd_value_t *val);
/// Device callback for listeners
typedef hpd_error_t (*hpd_device_f) (void *data, const hpd_device_id_t *device);
/// [Application API Callbacks]

/// [hpd_module_def_t]
typedef struct hpd_module_def {
    hpd_create_f on_create;
    hpd_destroy_f on_destroy;
    hpd_start_f on_start;
    hpd_stop_f on_stop;
    hpd_parse_opt_f on_parse_opt;
} hpd_module_def_t;
/// [hpd_module_def_t]

/**
 * Methods that a service supports. Note that these should always start with zero and have increments of one (used as
 * indices in arrays).
 */
/// [hpd_method_t]
typedef enum hpd_method {
    HPD_M_NONE = -1, //< Method that does not exist, must be first below valid methods
    HPD_M_GET = 0,   //< HTTP GET like method
    HPD_M_PUT,       //< HTTP PUT like method
    HPD_M_COUNT      //< Last
} hpd_method_t;
/// [hpd_method_t]

/// [hpd_status_t]
/**
 * HTTP status codes according to http://www.w3.org/Protocols/rfc2616/rfc2616.html
 *
 * Categories:
 *     Informational 1xx
 *     Successful 2xx
 *     Redirection 3xx
 *     Client Error 4xx
 *     Server Error 5xx
 */
#define HPD_HTTP_STATUS_CODE_MAP(XX) \
    XX(100, Continue) \
    XX(101, Switching Protocols) \
    XX(200, OK) \
    XX(201, Created) \
    XX(202, Accepted) \
    XX(203, Non-Authoritative Information) \
    XX(204, No Content) \
    XX(205, Reset Content) \
    XX(206, Partial Content) \
    XX(300, Multiple Choices) \
    XX(301, Moved Permanently) \
    XX(302, Found) \
    XX(303, See Other) \
    XX(304, Not Modified) \
    XX(305, Use Proxy) \
    XX(306, (Unused)) \
    XX(307, Temporary Redirect) \
    XX(400, Bad Request) \
    XX(401, Unauthorized) \
    XX(402, Payment Required) \
    XX(403, Forbidden) \
    XX(404, Not Found) \
    XX(405, Method Not Allowed) \
    XX(406, Not Acceptable) \
    XX(407, Proxy Authentication Required) \
    XX(408, Request Timeout) \
    XX(409, Conflict) \
    XX(410, Gone) \
    XX(411, Length Required) \
    XX(412, Precondition Failed) \
    XX(413, Request Entity Too Large) \
    XX(414, Request-URI Too Long) \
    XX(415, Unsupported Media Type) \
    XX(416, Requested Range Not Satisfiable) \
    XX(417, Expectation Failed) \
    XX(500, Internal Server Error) \
    XX(501, Not Implemented) \
    XX(502, Bad Gateway) \
    XX(503, Service Unavailable) \
    XX(504, Gateway Timeout) \
    XX(505, HTTP Version Not Supported)

typedef enum hpd_status {
#define XX(num, str) HPD_S_##num = num,
    HPD_HTTP_STATUS_CODE_MAP(XX)
#undef XX
} hpd_status_t;
/// [hpd_status_t]

/// [hpd_log_level_t]
typedef enum hpd_log_level {
    HPD_L_ERROR,
    HPD_L_WARN,
    HPD_L_INFO,
    HPD_L_DEBUG,
    HPD_L_VERBOSE,
} hpd_log_level_t;
/// [hpd_log_level_t]

/**
 * Default attribute key.
 *
 * Used in adapters to describe the network that they are connected to.
 */
/// [Default keys]
static const char * const HPD_ATTR_NETWORK  = "network";

/**
 * Default attribute key for descriptions.
 *
 * Used as a human readable description of the adapter, device, service, or parameter.
 */
static const char * const HPD_ATTR_DESC     = "description";

/**
 * Default attribute key for vendor ids.
 *
 * Usually set to a short vendor name for devices where appropriately.
 */
static const char * const HPD_ATTR_VENDOR   = "vendor";

/**
 * Default attribute key for product ids.
 *
 * Typically used on devices only.
 */
static const char * const HPD_ATTR_PRODUCT  = "product";

/**
 * Default attribute key for versions.
 *
 * Example candidates for this attribute are product versions, firmware versions, and protocol versions.
 */
static const char * const HPD_ATTR_VERSION  = "version";

/**
 * Default attribute key for locations.
 *
 * Physical location of the object. Could be an address, the name of a room, or even more detailed. Typically only set
 * on devices, when their location can be known by the adapter.
 */
static const char * const HPD_ATTR_LOCATION = "location";

/**
 * Default attribute key for types.
 *
 * On services it typically describes a category, e.g. temperature, lamp, etc.
 * On parameters it typically describes the datatype, e.g. int16, float, string, enum.
 */
static const char * const HPD_ATTR_TYPE     = "type";

/**
 * Default attribute key for units.
 *
 * The unit can both be set on services and parameters and describes the unit of the value, e.g. C, m, kg, m/s, etc.
 */
static const char * const HPD_ATTR_UNIT     = "unit";

/**
 * Default attribute key.
 *
 * Set on a parameter to indicate a maximum possible value where appropriate.
 */
static const char * const HPD_ATTR_MAX      = "max";

/**
 * Default attribute key.
 *
 * Set on a parameter to indicate a minimum possible value where appropriate.
 */
static const char * const HPD_ATTR_MIN      = "min";

/**
 * Default attribute key.
 *
 * Set on a parameter to indicate a increment between values, typically used for integers only. Step = 1 is assumed,
 * if not set.
 */
static const char * const HPD_ATTR_STEP     = "step";

/**
 * Default attribute key.
 *
 * Set on a parameter to indicate a scale factor that have to be multiplied with the value to match the given unit. 1
 * is assumed when not given.
 */
static const char * const HPD_ATTR_SCALE    = "scale";

/**
 * Default attribute key.
 *
 * Set on a parameter to the list of possible values for enumeration types.
 */
static const char * const HPD_ATTR_VALUES   = "values";
/// [Default keys]

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_TYPES_H

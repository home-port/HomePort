/*Copyright 2011 Aalborg University. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed*/

#ifndef HOMEPORT_HPD_DATAMANAGER_H
#define HOMEPORT_HPD_DATAMANAGER_H

typedef struct Configuration Configuration;
typedef struct Adapter Adapter;
typedef struct Device Device;
typedef struct Service Service;
typedef struct Parameter Parameter;
typedef struct Request Request;
typedef struct Listener Listener;

typedef       enum error_code ErrorCode;

typedef void (*free_f)             (void *data);
typedef void (*val_cb)             (Service *service, void *data, const char *val, size_t len);
typedef void (*val_err_cb)         (Service *service, void *data, ErrorCode code, const char *val, size_t len);
typedef void (*dev_cb)             (void *data, Device *device);
typedef void (*serviceGetFunction) (Service* service, Request req);
typedef void (*servicePutFunction) (Service* service, Request req, const char *put_value, size_t len);

// TODO Usage of HTTP error codes in here, but HomePort may no longer use HTTP at all...

// HTTP status codes according to
// http://www.w3.org/Protocols/rfc2616/rfc2616.html
#define HTTP_STATUS_CODE_MAP(XX) \
   XX(200,200 OK) \
   XX(201,201 Created) \
   XX(303,303 See Other) \
   XX(400,400 Bad Request) \
   XX(404,404 Not Found) \
   XX(405,405 Method Not Allowed) \
   XX(406,406 Not Acceptable) \
   XX(408,408 Request Timeout) \
   XX(415,415 Unsupported Media Type) \
   XX(500,500 Internal Server Error) \
   XX(504,504 Gateway Timeout)

/// HTTP status codes
/**
 *  According to RFC 2616, see
 *  http://www.w3.org/Protocols/rfc2616/rfc2616.html
 */
enum error_code
{
#define XX(num, str) ERR_##num = num,
    HTTP_STATUS_CODE_MAP(XX)
#undef XX
};

struct Configuration
{
    // Navigational members
    Adapter  *adapter_head;
    Listener *listener_head;
};

struct Adapter
{
    // Navigational members
    Configuration *configuration;
    Device        *device_head;
    Adapter       *next;
    Adapter       *prev;
    Listener      *listener_head;
    // Data members
    char          *id;
    char          *network;
    // User data
    free_f         free_data;
    void          *data;
};

struct Device
{
    // Navigational members
    Adapter *adapter;
    Service *service_head; /**<The first Service of the Service List*/
    Device  *next;
    Device  *prev;
    // Internal state
    char    attached;
    // Data members
    char    *description;  /**<The Device description*/
    char    *id;           /**<The Device ID*/
    char    *vendorId;     /**<The ID of the vendor*/
    char    *productId;    /**<The ID of the product*/
    char    *version;      /**<The Device version*/
    char    *location;     /**<The location of the Device*/
    char    *type;         /**<The Device type*/
    // User data
    free_f   free_data;
    void    *data;
};

struct Service
{
    // Navigational members
    Device             *device;
    Parameter          *parameter;   /**<The first Parameter of the Parameter List*/
    Service            *next;
    Service            *prev;
    Listener           *listener_head;
    // Data members
    char               *description; /**<The Service description*/
    char               *type;        /**<The Service type*/
    char               *unit;        /**<The unit provided by the Service*/
    serviceGetFunction  getFunction; /**<A pointer to the GET function of the Service*/
    servicePutFunction  putFunction; /**<A pointer to the PUT function of the Service*/
    char               *id;          /**<The Service ID*/
    // User data
    free_f              free_data;
    void               *data;        /**<Pointer used for the used to store its data*/
};

struct Parameter
{
    char *max;    /**<The maximum value of the Parameter*/
    char *min;    /**<The minimum value of the Parameter*/
    char *scale;  /**<The Scale of the Parameter*/
    char *step;   /**<The Step of the values of the Parameter*/
    char *type;   /**<The Type of values for the Parameter*/
    char *unit;   /**<The Unit of the values of the Parameter*/
    char *values; /**<The possible values for the Parameter*/
};

struct Request {
    val_err_cb on_response;
    void *data;
};

struct Listener {
    enum { SERVICE_LISTENER, CONFIGURATION_LISTENER, ADAPTER_LISTENER } type;
    // Navigational members
    union {
        Service  *service;
        Configuration *configuration;
        Adapter *adapter;
    };
    Listener *next;
    Listener *prev;
    // Internal state
    char      subscribed;
    // Data members
    union {
        struct {
            val_cb    on_change;
        };
        struct {
            dev_cb    on_attach;
            dev_cb    on_detach;
        };
    };
    // User data
    void          *data;
    free_f         on_free;
};

#endif //HOMEPORT_HPD_DATAMANAGER_H

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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <mxml.h>
#include <jansson.h>

typedef struct Configuration Configuration;
typedef struct Adapter       Adapter;
typedef struct Device        Device;
typedef struct Service       Service;
typedef struct Parameter     Parameter;

typedef void   (*serviceGetFunction) (Service* service, void *request);
typedef size_t (*servicePutFunction) (Service* service, char *buffer, size_t max_buffer_size, char *put_value);

/**
 * The structure Configuration containing all the Attributes that a Configuration possesses
 */
struct Configuration
{
   // Navigational members
   Adapter *adapter_head;
};

/**
 * The structure Adapter containing all the Attributes that an Adapter possesses
 */
struct Adapter
{
   // Navigational members
   Configuration *configuration;
   Device        *device_head;
   Adapter       *next;
   Adapter       *prev;
   // Data members
   char          *id;
   char          *network;
   // User data
   void          *data;
};

/**
 * The structure Device containing all the Attributes that a Device possess
 */
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
   void    *data;
};

/**
 * The structure Service containing all the Attributes that a Service possess
 */
struct Service
{
   // Navigational members
   Device             *device;
   Parameter          *parameter;   /**<The first Parameter of the Parameter List*/
   Service            *next;
   Service            *prev;
   // Data members
   char               *description; /**<The Service description*/
   int                 isActuator;  /**<Determine if the service is an actuator or a sensro */
   char               *type;        /**<The Service type*/
   char               *unit;        /**<The unit provided by the Service*/
   serviceGetFunction  getFunction; /**<A pointer to the GET function of the Service*/
   servicePutFunction  putFunction; /**<A pointer to the PUT function of the Service*/
   char               *id;          /**<The Service ID*/
   char               *uri;         /**<The Service URI*/
   // User data
   void               *data;        /**<Pointer used for the used to store its data*/
};

/**
 * The structure Service containing all the Attributes that a Parameter possess
 */
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

/* Function to handle configurations */
Configuration *configurationNew();
void           configurationFree(Configuration *config);
Adapter       *configurationFindAdapter(Configuration *configuration, char *adapter_id);
mxml_node_t   *configurationToXml(Configuration *configuration, mxml_node_t *parent);
json_t        *configurationToJson(Configuration *configuration);

/* Function to handle adapters */
Adapter     *adapterNew          (Configuration *configuration, const char *network, void *data);
void         adapterFree         (Adapter *adapter);
Device      *adapterFindDevice   (Adapter *adapter, char *device_id);

/* Function to handle devices */
Device*      deviceNew           (Adapter *adapter, const char *description, const char *vendorId, const char *productId,
                                  const char *version, const char *location, const char *type, void *data);
void         deviceFree          (Device *device); 
Service     *deviceFindService   (Device *device, char *service_id);

/* Function to handle services */
Service*     serviceNew         (Device *device, const char *description, int isActuator, const char *type, const char *unit,
                                 serviceGetFunction getFunction, servicePutFunction putFunction, Parameter *parameter, void* data); 
void         serviceFree        (Service *service);
int          serviceGenerateUri (Service *service);

/* Function to handle parameters */
Parameter* parameterNew  (const char *max, const char *min, const char *scale, const char *step,
                          const char *type, const char *unit, const char *values);
void       parameterFree (Parameter *parameter);

#endif

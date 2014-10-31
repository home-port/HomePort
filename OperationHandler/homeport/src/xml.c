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

#include "xml.h"
#include "hpd_configuration.h"
#include <time.h>
#include <mxml.h>

/**
 *  * Simple timestamp function
 *   *
 *    * @return Returns the timestamp
 *     */
static char *
timestamp ( void )
{
  time_t ltime;
  ltime = time(NULL);
  const struct tm *timeptr = localtime(&ltime);

  static char wday_name[7][3] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static char mon_name[12][3] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static char result[25];

  sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
      wday_name[timeptr->tm_wday],
      mon_name[timeptr->tm_mon],
      timeptr->tm_mday, timeptr->tm_hour,
      timeptr->tm_min, timeptr->tm_sec,
      1900 + timeptr->tm_year);
  return result;
}

char *
xmlGetConfiguration(HomePort *homeport)
{
   char *res;
   mxml_node_t *xml = mxmlNewXML("1.0");
   configurationToXml(homeport->configuration, xml);
   res = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
   mxmlDelete(xml);
   return res;
}

char*
xmlGetState(char *state)
{
  mxml_node_t *xml;
  mxml_node_t *stateXml;

  xml = mxmlNewXML("1.0");
  stateXml = mxmlNewElement(xml, "value");
  mxmlElementSetAttr(stateXml, "timestamp", timestamp());
  mxmlNewText(stateXml, 0, state);

  char* return_value = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
  mxmlDelete(xml);

  return return_value;
}

char*
xmlParseState(char *xml_value)
{
  mxml_node_t *xml;
  mxml_node_t *node;

  xml = mxmlLoadString(NULL, xml_value, MXML_TEXT_CALLBACK);
  if(xml == NULL)
  {
    printf("XML value format uncompatible with HomePort\n");
    return NULL;
  }

  node = mxmlFindElement(xml, xml, "value", NULL, NULL, MXML_DESCEND);
  if(node == NULL || node-> child == NULL || node->child->value.text.string == NULL)
  {
    mxmlDelete(xml);
    printf("No \"value\" in the XML file\n");
    return NULL;
  }

  char *state = malloc(sizeof(char)*(strlen(node->child->value.text.string)+1));
  strcpy(state, node->child->value.text.string);

  mxmlDelete(xml);

  return state;
}



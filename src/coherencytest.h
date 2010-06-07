/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
 * implied. See the License for the specific language governing permissions and limitations under the 
 * License. 
 */

#ifndef _COHERENCYTEST_H_
#define _COHERENCYTEST_H_

#ifdef WIN32
#include "Windows.h"
#endif 

#include <libxml/xmlschemas.h>
#include <libxml/schemasInternals.h>
#include <libxml/schematron.h>
#include <libxml/xmlreader.h>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <time.h>
using namespace std;

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
#define MAX_LOG_BUFFER 1024
#define MAX_NAME_SIZE 512 


#include <dae.h> 
#include <dae/daeSIDResolver.h>
#include <dae/daeErrorHandler.h>
#include <dom/domTypes.h>
#include <dom/domCOLLADA.h>
#include <dom/domConstants.h>
#include <dom/domElements.h>
#include <dom/domProfile_GLES.h>
#include <dom/domProfile_GLSL.h>
#include <dom/domProfile_CG.h>
#include <dom/domProfile_COMMON.h>
#include <dom/domFx_include_common.h>


#ifndef stricmp
inline int stricmp(const char *s1, const char *s2)
{
  char f, l;

  do 
  {
    f = ((*s1 <= 'Z') && (*s1 >= 'A')) ? *s1 + 'a' - 'A' : *s1;
    l = ((*s2 <= 'Z') && (*s2 >= 'A')) ? *s2 + 'a' - 'A' : *s2;
    s1++;
    s2++;
  } while ((f) && (f == l));

  return (int) (f - l);
}

#endif 

class CoherencyTestErrorHandler : public daeErrorHandler
{
public:
	CoherencyTestErrorHandler();
	virtual ~CoherencyTestErrorHandler();

public:
	void handleError( daeString msg );
	void handleWarning( daeString msg );
};

#endif //_COHERENCYTEST_H_


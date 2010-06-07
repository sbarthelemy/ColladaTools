#ifndef __DOMTEST_H__
#define __DOMTEST_H__

#include <iostream>
#include <dae.h>
#include <dom.h>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <time.h>

#include "dae.h" 
#include "dae/daeSIDResolver.h"
#include "dae/daeErrorHandler.h"
#include "dom/domTypes.h"
#include "dom/domCOLLADA.h"
#include "dom/domConstants.h"
#include "dom/domElements.h"
#include "dom/domProfile_GLES.h"
#include "dom/domProfile_GLSL.h"
#include "dom/domProfile_CG.h"
#include "dom/domProfile_COMMON.h"
#include "dom/domFx_include_common.h"




#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
#define MAX_LOG_BUFFER 1024
#define MAX_NAME_SIZE 512 




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





#endif


/*
The MIT License

Copyright 2006 Sony Computer Entertainment Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
List of Checks
Check_links		It checks if all id are valid reference and if we can resolveElement and getElement 
				from a link    
Check_unique_id	It checks if all Ids in each document are unique     
Check_counts	It checks number counts are correctly set, eg. 
				skin vertex count should be = mesh vertex count     
				accessor has the right count on arrays from stride and counts.
Check_files		It checks if the image files, cg/fx files, and other non-dae files that the document 
				referenced exist     
Check_textures  It checks if the textures are correctly defined/used (image, surface, sampler, 
				instancing...)
				<texture> shouldn't directly reference to <image> id.
				it should reference <newparam>'s sid, and have <sampler2D> with <source> reference to
				another <newparam>'s sid that have <surface> with <init_from> refernce to <image> id.

Check_URI		It checks if the URI are correct. It should also check for unescaped spaces because 
				a xml validator won't catch the problem. Reference http://www.w3.org/TR/xmlschema-2/#anyURI
Check_schema	It checks if the document validates against the Schema   
Check_inputs	It checks if the required number of input elements are present and that they have the 
				correct semantic values for their sources.   
Check_skin		It will check if values in name_array should only reference to an existing SID, 
				and values in IDREF_array should only reference to an existing ID  
Check_InstanceGeometry It checks if all Intance_geometry has bind_material that has a correct matching 
				bind_material between symbol and target
Check_Controller It checks if skin have same number of vertices weight as the vertices number of geometry.
                It checks if morph have same number of vertices from source geometry as number of vertices 
				in all other target geometry.
Check_Float_array It checks if NaN, INF, -INF exist in all the float array        
Check_sid		It checks if a sid is a valid sid   
CHECK_morph     It checks if a morph have same number of targets and target_weights
                It checks if all targets have the same number of vertices.
*/

#include "coherencytest.h"

#define VERSION_NUMBER "1.3"

string file_name, log_file;
string output_file_name = "";
FILE * file;
FILE * log;
bool quiet;
domUint fileerrorcount  = 0;
string xmlschema_file =  "http://www.collada.org/2005/11/COLLADASchema.xsd";

void PRINTF(const char * str)
{
	if (str==0) return;
	if (quiet == false)
		printf("%s",str);
	if (file)
		fwrite(str, sizeof(char), strlen(str), file);
	if (log)
		fwrite(str, sizeof(char), strlen(str), log);
}

int VERBOSE;
void print_name_id(domElement * element);
domUint CHECK_error(domElement * element, bool b,  const char * message= NULL);
void CHECK_warning(domElement * element, bool b, const char *message = NULL);
domUint CHECK_uri(const xsAnyURI & uri);
domUint CHECK_count(domElement * element, domInt expected, domInt result, const char *message = NULL);
bool CHECK_fileexist(const char * filename);
domUint CHECK_file(domElement *element, xsAnyURI & fileuri);
domUint	GetMaxOffsetFromInputs(domInputLocalOffset_Array & inputs);
domUint CHECK_Triangles(domTriangles *triangles);
domUint CHECK_Polygons(domPolygons *polygons);
domUint CHECK_Polylists(domPolylist *polylist);
domUint CHECK_Tristrips(domTristrips *tristrips);
domUint CHECK_Trifans(domTrifans *trifans);
domUint CHECK_Lines(domLines *lines);
domUint CHECK_Linestrips(domLinestrips *linestrips);
domUint CHECK_Geometry(domGeometry *geometry);
domUint CHECK_InstanceGeometry(domInstance_geometry * instance_geometry);
domUint CHECK_Controller(domController *controller);
domUint CHECK_InstanceElementUrl(daeDatabase *db, daeInt instanceElementID);
domUint GetSizeFromType(xsNMTOKEN type);
domUint CHECK_Source(domSource * source);
//	void _XMLSchemaValidityErrorFunc(void* ctx, const char* msg, ...);
//	void _XMLSchemaValidityWarningFunc(void* ctx, const char* msg, ...);
domUint CHECK_validateDocument(_xmlDoc *LXMLDoc);
domUint CHECK_xmlfile(daeString filename);

domUint CHECK_counts (DAE *input, int verbose = 0);
domUint CHECK_links (DAE *input, int verbose = 0);
domUint CHECK_files (DAE *input, int verbose = 0);
domUint CHECK_unique_id (DAE *input, int verbose = 0);
domUint CHECK_texture (DAE *input, int verbose = 0);
domUint CHECK_schema (DAE *input, int verbose = 0);
domUint CHECK_skin (DAE *input, int verbose = 0);
domUint CHECK_inputs (domInputLocal_Array & inputs, const char * semantic);
domUint CHECK_inputs (domInputLocalOffset_Array & inputs, const char * semantic);
domUint CHECK_float_array (DAE *input, int verbose = 0);
domUint CHECK_Circular_Reference (DAE *input, int verbose = 0);
domUint CHECK_Index_Range (domElement * elem, domListOfUInts & listofint, domUint index_range, domUint offset, domUint maxoffset, int verbose = 0);
domUint CHECK_Index_Range (domElement * elem, domListOfInts & listofint, domUint index_range, domUint offset, domUint maxoffset, int verbose = 0);
domUint CHECK_sid(DAE *input, int verbose = 0);
domUint CHECK_morph(DAE *input, int verbose = 0);

const char VERSION[] = 
"Coherencytest version " VERSION_NUMBER "\n"
"The MIT License\n"
"\n"
"Copyright 2009 Sony Computer Entertainment Inc.\n"
"\n"
"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
"of this software and associated documentation files (the \"Software\"), to deal\n"
"in the Software without restriction, including without limitation the rights\n"
"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
"copies of the Software, and to permit persons to whom the Software is\n"
"furnished to do so, subject to the following conditions:\n"
"\n"
"The above copyright notice and this permission notice shall be included in\n"
"all copies or substantial portions of the Software.\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\n"
"THE SOFTWARE.\n";

const char USAGE[] =
"Usage: coherencytest filename.dae ... [OPTION]...\n"
" option:                           \n"
" filename.dae				- check collada file filename.dae, filename.dae should be a url format\n"
" -log filename.log         - log warnings and errors in filename.log          \n"
" -check SCHEMA COUNTS ..   - check SCHEMA and COUNTS only, test all if not specify any\n"
"                             available checks:              \n"
"                             SCHEMA						\n"
"                             UNIQUE_ID 					\n"
"                             COUNTS						\n"
"                             LINKS							\n"
"                             TEXTURE						\n"
"                             FILES							\n"
"                             SKIN							\n"
"                             FLOAT_ARRAY					\n"
"                             CIRCULR_REFERENCE				\n"
"                             INDEX_RANGE					\n"
"                             SID							\n"
"                             MORPH							\n"
" -ignore SCHEMA COUNTS ..  - ignore SCHEMA and COUNTS only, test all if not specify any\n"
" -quiet -q                 - disable printfs and MessageBox\n"
" -version                  - print version and copyright information\n"
" -help,-usage              - print usage information\n"
" -xmlschema schema.xsd     - use your own version of schema.xsd to do schema check\n"
"                           - the defualt schema is \"http://www.collada.org/2005/11/COLLADASchema.xsd\"\n"
" -ctf,                     - loging report for ctf\n";

std::map<string, bool> checklist;
int main(int Argc, char **argv)
{
	std::vector<string> file_list;
	int err = 0;
	log = 0;
	bool checkall = true;
	domUint totalerrorcount = 0;
	quiet = false;

	for (int i=1; i<Argc; i++)
	{	
		if (stricmp(argv[i], "-log") == 0)
		{
			i++;
			if (i <= Argc)
				log_file = argv[i];
			log = fopen(log_file.c_str(), "a");
		} else if (stricmp(argv[i], "-check") == 0)
		{
			i++;
			if (i >= Argc)
				break;
			while (argv[i][0]!='-')
			{
				checkall = false;
				for (size_t j=0; j<strlen(argv[i]); j++)
					argv[i][j] = toupper(argv[i][j]);
				checklist[argv[i]] = true;
				i++;
				if (i >= Argc)
					break;
			}
			i--;
		} else if (stricmp(argv[i], "-ignore") == 0)
		{
			checkall = false;
			checklist["SCHEMA"]				= true;
			checklist["UNIQUE_ID"]			= true;
			checklist["COUNTS"]				= true;
			checklist["LINKS"]				= true;
			checklist["TEXTURE"]			= true;
			checklist["FILES"]				= true;
			checklist["SKIN"]				= true;
			checklist["FLOAT_ARRAY"]		= true;
			checklist["CIRCULR_REFERENCE"]	= true;
			checklist["INDEX_RANGE"]		= true;
			checklist["SID"]				= true;
			checklist["MORPH"]				= true;
			i++;
			if (i >= Argc)
				break;
			while (argv[i][0]!='-')
			{
				for (size_t j=0; j<strlen(argv[i]); j++)
					argv[i][j] = toupper(argv[i][j]);
				checklist[argv[i]] = false;
				i++;
				if (i >= Argc)
					break;
			}
			i--;
		} else if (stricmp(argv[i], "-version") == 0)
		{
			printf(VERSION);
			return 0;
		} else if (stricmp(argv[i], "-quiet") == 0 || stricmp(argv[i], "-q") == 0)
		{
			quiet = true;
		} else if (stricmp(argv[i], "-help") == 0 || stricmp(argv[i], "-usage") == 0)
		{
			printf(USAGE);
			return 0;
		} else if (stricmp(argv[i], "-ctf") == 0)
		{
			i++;
			if (i <= Argc)
				log_file = argv[i];
			log = fopen(log_file.c_str(), "w");
			quiet = true;
		} else if (stricmp(argv[i], "-xmlschema") == 0)
		{
			i++;
			if (i <= Argc)
				xmlschema_file = argv[i];
		} else 
		{
			file_list.push_back(argv[i]);
		}

		
	}
	if (file_list.size() == 0)
	{
		printf(USAGE);
		return 0;
	}
	
	for (size_t i=0; i<file_list.size(); i++)
	{
		file_name = file_list[i];

		char str[MAX_LOG_BUFFER];

		time_t current_time = time(NULL);
		tm * local_time = localtime(&current_time);
		char * str_time = asctime(local_time);

		if (log && quiet == false) 
		{
			sprintf(str, "BEGIN CHECK %s %s\n", file_name.c_str(), str_time);
			if (log) fwrite(str, sizeof(char), strlen(str), log);
		}

		output_file_name = file_name + string(".log");

		if (isalpha(file_name[0]) && file_name[1]==':' && file_name[2]=='\\')
		{
			file_name = string("/") + file_name;
			for (unsigned int i=0; i<file_name.size(); i++)
			{
				if (file_name[i] =='\\')
					file_name[i] = '/';
			}
		}

		fileerrorcount  = 0;
		file = fopen(output_file_name.c_str(), "w+");

		DAE * dae = new DAE;
		CoherencyTestErrorHandler * errorHandler = new CoherencyTestErrorHandler();
		daeErrorHandler::setErrorHandler(errorHandler);
		err = dae->load(file_name.c_str());
		if (err != 0) {
			if (quiet == false)
			{
				printf("DOM Load error = %d\n", (int) err);
				printf("filename = %s\n", file_name.c_str());
#ifdef WIN32				
				MessageBox(NULL, "Collada Dom Load Error", "Error", MB_OK);
#endif
			}
			return err;
		}

		if (checkall)
		{
			checklist["SCHEMA"]				= true;
			checklist["UNIQUE_ID"]			= true;
			checklist["COUNTS"]				= true;
			checklist["LINKS"]				= true;
			checklist["TEXTURE"]			= true;
			checklist["FILES"]				= true;
			checklist["SKIN"]				= true;
			checklist["FLOAT_ARRAY"]		= true;
			checklist["CIRCULR_REFERENCE"]	= true;
			checklist["INDEX_RANGE"]		= true;
			checklist["SID"]				= true;
			checklist["MORPH"]				= true;
		}

		if (checklist["SCHEMA"])
			fileerrorcount  += CHECK_schema(dae);
		if (checklist["UNIQUE_ID"])
			fileerrorcount  += CHECK_unique_id(dae);
		if (checklist["COUNTS"])
			fileerrorcount  += CHECK_counts(dae);
		if (checklist["LINKS"])
			fileerrorcount  += CHECK_links(dae);
		if (checklist["TEXTURE"])
			fileerrorcount  += CHECK_texture(dae);
		if (checklist["FILES"])
			fileerrorcount += CHECK_files(dae);
		if (checklist["SKIN"])
			fileerrorcount  += CHECK_skin(dae);
		if (checklist["FLOAT_ARRAY"])
			fileerrorcount  += CHECK_float_array(dae);
		if (checklist["CIRCULR_REFERENCE"])
			fileerrorcount  += CHECK_Circular_Reference (dae);
		if (checklist["SID"])
			fileerrorcount  += CHECK_sid (dae);
		if (checklist["MORPH"])
			fileerrorcount  += CHECK_morph (dae);
			
		if (file) fclose(file);
		delete errorHandler;
		delete dae;
		if (fileerrorcount  == 0)
			remove(output_file_name.c_str());

		if (log && quiet == false) 
		{
			sprintf(str, "END CHECK %s with %d errors\n\n", file_name.c_str(), (int) fileerrorcount);
			fwrite(str, sizeof(char), strlen(str), log);
		}
		totalerrorcount += fileerrorcount ;
	}
	if (log) fclose(log);
	return (int) totalerrorcount;
}

void print_name_id(domElement * element)
{
	domElement * e = element;
	while ( e->getID() == NULL)
		e = e->getParentElement();
	char temp[MAX_LOG_BUFFER];
	sprintf(temp, "(type=%s,id=%s)", e->getTypeName(), e->getID());
	PRINTF(temp);
}
domUint CHECK_error(domElement * element, bool b,  const char * message)
{
	if (b == false) {
		PRINTF("ERROR: ");
		if (element) print_name_id(element);
		if (message) PRINTF(message);
		return 1;
	}
	return 0;
}
void CHECK_warning(domElement * element, bool b, const char *message)
{
	if (b == false) {
		PRINTF("WARNING: ");
		print_name_id(element);
		if (message) PRINTF(message);
	}
}
domUint CHECK_uri(const xsAnyURI & uri)
{
//	uri.resolveElement();
	if (uri.getElement() == NULL)
	{
		char temp[MAX_LOG_BUFFER];
		sprintf(temp, "ERROR: CHECK_uri Failed uri=%s not resolved\n",uri.getURI());
		PRINTF(temp);
		return 1;
	}

	return 0;//CHECK_escape_char(uri.getOriginalURI());
}
domUint CHECK_count(domElement * element, domInt expected, domInt result, const char *message)
{
	if (expected != result)
	{
		char temp[MAX_LOG_BUFFER];
		sprintf(temp, "ERROR: CHECK_count Failed: expected=%d, result=%d", (int) expected, (int) result);
		PRINTF(temp);
		print_name_id(element);
		if (message) PRINTF(message);
		else PRINTF("\n");
		return 1;
	}
	return 0;
}
bool CHECK_fileexist(const char * filename)
{
	xmlTextReader * reader = xmlReaderForFile(filename, 0, 0);
	if (!reader)
		return false;
	return true;
}
domUint CHECK_file(domElement *element, xsAnyURI & fileuri)
{
	daeURI * uri = element->getDocumentURI();
	string TextureFilePrefix = uri->pathDir();

	// Build a path using the scene name ie: images/scene_Textures/boy.tga
	daeChar newTexName[MAX_NAME_SIZE]; 	
 	sprintf(newTexName, "%s%s", TextureFilePrefix.c_str(), fileuri.getURI() ); 
	
	// Build a path for the Shared texture directory ie: images/Shared/boy.tga
	daeChar sharedTexName[MAX_NAME_SIZE];
	sprintf(sharedTexName, "%sShared/%s",TextureFilePrefix.c_str(), fileuri.pathFile().c_str() );

	if (!CHECK_fileexist(fileuri.getURI()))
		if(!CHECK_fileexist(newTexName))
			if(!CHECK_fileexist(sharedTexName))
			{
				char temp[MAX_LOG_BUFFER];
				sprintf(temp, "ERROR: CHECK_file failed, %s not found\n", fileuri.getURI());
				PRINTF(temp);
				return 1;
			}
	return 0;
}

domUint	GetMaxOffsetFromInputs(domInputLocalOffset_Array & inputs)
{
	domUint maxoffset = 0;
	domUint count = (domUint) inputs.getCount();
	for(size_t i=0; i< count ;i++)
	{
		domUint thisoffset  = inputs[i]->getOffset();
		if (maxoffset < thisoffset) maxoffset = thisoffset;
	}
	return maxoffset + 1;
}

domUint GetIndexRangeFromInput(domInputLocalOffset_Array &input_array, domUint offset, domUint & error)
{
	char message[1024];
	for (size_t j=0; j<input_array.getCount(); j++)
	{
		if (input_array[j]->getOffset() == offset)
		{
			if (stricmp(input_array[j]->getSemantic(), "VERTEX") == 0)
			{	// vertex
				domVertices * vertices = (domVertices*)(domElement*) input_array[j]->getSource().getElement();
				if (vertices) 
				{
					domInputLocal_Array & inputs = vertices->getInput_array();
					for (size_t i=0; i<inputs.getCount(); i++)
					{
						if (stricmp(inputs[i]->getSemantic(), "POSITION") == 0)
						{
							domSource * source = (domSource*)(domElement*) inputs[i]->getSource().getElement();
							if (source)
							{
								domSource::domTechnique_common * technique_common = source->getTechnique_common();
								if (technique_common)
								{
									domAccessor * accessor = technique_common->getAccessor();
									if (accessor)
										return accessor->getCount();
								}
							}
						}
					}
				}
			} else { // non-vertex
				domSource * source = (domSource*)(domElement*) input_array[j]->getSource().getElement();
				if (source->getElementType() != COLLADA_TYPE::SOURCE)
				{
					daeString semantic_str = input_array[j]->getSemantic();
					daeString source_str = input_array[j]->getSource().getOriginalURI();
					sprintf(message, "input with semantic=%s source=source_str is not referencing to a source\n", semantic_str, source_str);
					CHECK_error(input_array[j], false, message);
					error++;
					continue;
				}
				if (source)
				{
					domSource::domTechnique_common * technique_common = source->getTechnique_common();
					if (technique_common)
					{
						domAccessor * accessor = technique_common->getAccessor();
						if (accessor)
							return accessor->getCount();
					}
				}
			}
		}
	}
	sprintf(message, "Thera are no input with offset=%d, can't complete Index_Range\n", offset);
	CHECK_error(NULL, false, message);
	error++;
	return 0;
}
domUint CHECK_Triangles(domTriangles *triangles)
{
	domUint errorcount = 0;
	domUint count = triangles->getCount();
	domInputLocalOffset_Array & inputs = triangles->getInput_array();
	errorcount += CHECK_inputs(inputs, "VERTEX");
	domUint maxoffset = GetMaxOffsetFromInputs(inputs);
	domPRef p = triangles->getP();
	domListOfUInts & ints = p->getValue();

	// check count
	errorcount += CHECK_count(triangles, 3 * count * maxoffset, (domInt) ints.getCount(),
		                      "triangles, count doesn't match\n");

	// check index range
	for (domUint offset=0; offset<maxoffset; offset++)
	{
		domUint index_range = GetIndexRangeFromInput(inputs, offset, errorcount);
		errorcount += CHECK_Index_Range (triangles, ints, index_range, offset, maxoffset);
	}
	return errorcount;
}
domUint CHECK_Polygons(domPolygons *polygons)
{
	domUint errorcount = 0;
	domUint count = polygons->getCount();
	domInputLocalOffset_Array & inputs = polygons->getInput_array();
	errorcount += CHECK_inputs(inputs, "VERTEX");
	domUint maxoffset = GetMaxOffsetFromInputs(inputs);
	domP_Array & parray = polygons->getP_array();
	domPolygons::domPh_Array & pharray = polygons->getPh_array();

	// check count
	errorcount += CHECK_count(polygons, count, (domInt) parray.getCount() + pharray.getCount(),
		                      "polygons, count doesn't match\n");

	// check index range
	for (size_t i=0; i<parray.getCount(); i++)
	{
		domListOfUInts & ints = parray[i]->getValue();
		for (domUint offset=0; offset<maxoffset; offset++)
		{
			domUint index_range = GetIndexRangeFromInput(inputs, offset, errorcount);
			errorcount += CHECK_Index_Range (polygons, ints, index_range, offset, maxoffset);
		}
	}
	return errorcount;
}
domUint CHECK_Polylists(domPolylist *polylist)
{
	domUint errorcount = 0;
	domUint count = polylist->getCount();
	domInputLocalOffset_Array & inputs = polylist->getInput_array();
	errorcount += CHECK_inputs(inputs, "VERTEX");
	domUint maxoffset = GetMaxOffsetFromInputs(inputs);
	domPRef p = polylist->getP();
	domPolylist::domVcountRef vcount = polylist->getVcount();

	// check vcount
	errorcount += CHECK_count(polylist, count, (domInt) vcount->getValue().getCount(),
		                      "polylists, count doesn't match\n");
	// check p count
	domUint vcountsum = 0;
	for (size_t i=0; i<count; i++)
	{
		vcountsum += vcount->getValue()[i];
	}
	errorcount += CHECK_count(polylist, (domInt) p->getValue().getCount(), vcountsum * maxoffset,
		                      "polylists, total vcount and p count doesn't match\n");

	// check index range
	for (domUint offset=0; offset<maxoffset; offset++)
	{
		domUint index_range = GetIndexRangeFromInput(inputs, offset, errorcount);
		errorcount += CHECK_Index_Range (polylist, p->getValue(), index_range, offset, maxoffset);
	}
	return errorcount;
}
domUint CHECK_Tristrips(domTristrips *tristrips)
{
	domUint errorcount = 0;
	domUint count = tristrips->getCount();
	domInputLocalOffset_Array & inputs = tristrips->getInput_array();
	errorcount += CHECK_inputs(inputs, "VERTEX");
	domUint maxoffset = GetMaxOffsetFromInputs(inputs);
	domP_Array & parray = tristrips->getP_array();

	// check vcount
	errorcount += CHECK_count(tristrips, count, (domInt) parray.getCount(),
		                      "tristrips, count doesn't match\n");
	// check p count
	for (size_t i=0; i<count; i++)
	{
		errorcount += CHECK_count(tristrips, 3 * maxoffset <= parray[i]->getValue().getCount(), 1,
			                      "tristrips, this p has less than 3 vertices\n");		
		errorcount += CHECK_count(tristrips, (domInt) parray[i]->getValue().getCount() % maxoffset, 0,
			                      "tristrips, this p count is not in multiple of maxoffset\n");		
	}

	// check index range
	for (size_t i=0; i<parray.getCount(); i++)
	{
		domListOfUInts & ints = parray[i]->getValue();
		for (domUint offset=0; offset<maxoffset; offset++)
		{
			domUint index_range = GetIndexRangeFromInput(inputs, offset, errorcount);
			errorcount += CHECK_Index_Range (tristrips, ints, index_range, offset, maxoffset);
		}
	}
	return errorcount;
}
domUint CHECK_Trifans(domTrifans *trifans)
{
	domUint errorcount = 0;
	domUint count = trifans->getCount();
	domInputLocalOffset_Array & inputs = trifans->getInput_array();
	errorcount += CHECK_inputs(inputs, "VERTEX");
	domUint maxoffset = GetMaxOffsetFromInputs(inputs);
	domP_Array & parray = trifans->getP_array();

	// check vcount
	errorcount += CHECK_count(trifans, count, (domInt) parray.getCount(),
		                      "trifan, count doesn't match\n");
	// check p count
	for (size_t i=0; i<count; i++)
	{
		errorcount += CHECK_count(trifans, 3 * maxoffset <= parray[i]->getValue().getCount(), 1,
			                      "trifan, this p has less than 3 vertices\n");		
		errorcount += CHECK_count(trifans, (domInt) parray[i]->getValue().getCount() % maxoffset, 0,
			                      "trifan, this p count is not in multiple of maxoffset\n");		
	}

	// check index range
	for (size_t i=0; i<parray.getCount(); i++)
	{
		domListOfUInts & ints = parray[i]->getValue();
		for (domUint offset=0; offset<maxoffset; offset++)
		{
			domUint index_range = GetIndexRangeFromInput(inputs, offset, errorcount);
			errorcount += CHECK_Index_Range (trifans, ints, index_range, offset, maxoffset);
		}
	}
	return errorcount;
}
domUint CHECK_Lines(domLines *lines)
{
	domUint errorcount = 0;
	domUint count = lines->getCount();
	domInputLocalOffset_Array & inputs = lines->getInput_array();
	errorcount = CHECK_inputs(inputs, "VERTEX");
	domUint maxoffset = GetMaxOffsetFromInputs(inputs);
	domP * p = lines->getP();
	// check p count
	errorcount += CHECK_count(lines, 2 * count * maxoffset, (domInt) p->getValue().getCount(),
		                      "lines, count doesn't match\n");

	// check index range
	for (domUint offset=0; offset<maxoffset; offset++)
	{
		domUint index_range = GetIndexRangeFromInput(inputs, offset, errorcount);
		errorcount += CHECK_Index_Range (lines, p->getValue(), index_range, offset, maxoffset);
	}
	return errorcount;
}

domUint CHECK_Linestrips(domLinestrips *linestrips)
{
	domUint errorcount = 0;
	domUint count = linestrips->getCount();
	domInputLocalOffset_Array & inputs = linestrips->getInput_array();
	errorcount += CHECK_inputs(inputs, "VERTEX");
	domUint maxoffset = GetMaxOffsetFromInputs(inputs);
	domP_Array & parray = linestrips->getP_array();

	// check p count
	errorcount += CHECK_count(linestrips, count, (domInt) parray.getCount(),
		                      "linestrips, count doesn't match\n");
	// check inputs
	for (size_t i=0; i<count; i++)
	{
		errorcount += CHECK_count(linestrips, 2 * maxoffset <= parray[i]->getValue().getCount(), 1,
			                  "linestrips, this p has less than 2 vertices\n");		
		errorcount += CHECK_count(linestrips, (domInt) parray[i]->getValue().getCount() % maxoffset, 0,
			                  "linestrips, this p is not in mutiple of maxoffset\n");		
	}

	// check index range
	for (size_t i=0; i<parray.getCount(); i++)
	{
		domListOfUInts & ints = parray[i]->getValue();
		for (domUint offset=0; offset<maxoffset; offset++)
		{
			domUint index_range = GetIndexRangeFromInput(inputs, offset, errorcount);
			errorcount += CHECK_Index_Range (linestrips, ints, index_range, offset, maxoffset);
		}
	}
	return errorcount;
}

domUint CHECK_Geometry(domGeometry *geometry)
{
	domUint errorcount = 0;
	domMesh * mesh = geometry->getMesh();
	if (mesh == NULL)
		return 0;

	// check vertices
	domVertices *vertices = mesh->getVertices();
	CHECK_error(geometry, vertices != NULL, "geometry, no vertices in this mesh\n");
	if (vertices)
	{
		domInputLocal_Array & inputs = vertices->getInput_array();
		errorcount += CHECK_inputs(inputs, "POSITION");
	}
	// triangles
	domTriangles_Array & triangles = mesh->getTriangles_array();
	for (size_t i=0; i<triangles.getCount(); i++)
	{
		errorcount += CHECK_Triangles(triangles[i]);
	}
	// polygons
	domPolygons_Array & polygons = mesh->getPolygons_array();
	for (size_t i=0; i<polygons.getCount(); i++)
	{
		errorcount += CHECK_Polygons(polygons[i]);
	}
	// polylist
	domPolylist_Array & polylists = mesh->getPolylist_array();
	for (size_t i=0; i<polylists.getCount(); i++)
	{
		errorcount += CHECK_Polylists(polylists[i]);
	}
	// tristrips
	domTristrips_Array & tristrips = mesh->getTristrips_array();
	for (size_t i=0; i<tristrips.getCount(); i++)
	{
		errorcount += CHECK_Tristrips(tristrips[i]);
	}
	// trifans
	domTrifans_Array & trifans = mesh->getTrifans_array();
	for (size_t i=0; i<trifans.getCount(); i++)
	{
		errorcount += CHECK_Trifans(trifans[i]);
	}
	// lines
	domLines_Array & lines = mesh->getLines_array();
	for (size_t i=0; i<lines.getCount(); i++)
	{
		errorcount += CHECK_Lines(lines[i]);
	}
	// linestrips
	domLinestrips_Array & linestrips = mesh->getLinestrips_array();
	for (size_t i=0; i<linestrips.getCount(); i++)
	{
		errorcount += CHECK_Linestrips(linestrips[i]);
	}
	return errorcount;
}

domUint CHECK_material_symbols(domGeometry * geometry, domBind_material * bind_material)
{
	domUint errorcount = 0;
	std::set<string> material_symbols;
	if (bind_material == NULL) {
		PRINTF("ERROR: CHECK_material_symbols failed no bind materials in this instance\n");
		return 1;
	}
	if (bind_material->getTechnique_common() == NULL) {
		PRINTF("ERROR: CHECK_material_symbols failed ");
		print_name_id(bind_material);
		PRINTF(" no technique_common in bind materials.\n");
		return 0;
	}
	domInstance_material_Array & imarray = bind_material->getTechnique_common()->getInstance_material_array();
	for (size_t i=0; i<imarray.getCount(); i++)
	{
		material_symbols.insert(string(imarray[i]->getSymbol()));		
	}

	if (geometry == NULL) return errorcount;
	domMesh * mesh = geometry->getMesh();
	if (mesh) {
		domTriangles_Array & triangles = mesh->getTriangles_array();
		for (size_t i=0; i<triangles.getCount(); i++)
		{
			daeString material_group = triangles[i]->getMaterial();
			if (material_group)
				CHECK_warning(bind_material, 
						material_symbols.find(material_group) != material_symbols.end(),
						"binding not found for material symbol\n");
		}
		domPolygons_Array & polygons = mesh->getPolygons_array();
		for (size_t i=0; i<polygons.getCount(); i++)
		{
			daeString material_group = polygons[i]->getMaterial();
			if (material_group)
				CHECK_warning(bind_material, 
						material_symbols.find(material_group) != material_symbols.end(),
						"binding not found for material symbol\n");
		}
		domPolylist_Array & polylists = mesh->getPolylist_array();
		for (size_t i=0; i<polylists.getCount(); i++)
		{
			daeString material_group = polylists[i]->getMaterial();
			if (material_group)
				CHECK_warning(bind_material, 
						material_symbols.find(material_group) != material_symbols.end(),
						"binding not found for material symbol\n");
		}
		domTristrips_Array & tristrips = mesh->getTristrips_array();
		for (size_t i=0; i<tristrips.getCount(); i++)
		{
			daeString material_group = tristrips[i]->getMaterial();
			if (material_group)
				CHECK_warning(bind_material, 
						material_symbols.find(material_group) != material_symbols.end(),
						"binding not found for material symbol\n");
		}
		domTrifans_Array & trifans = mesh->getTrifans_array();
		for (size_t i=0; i<trifans.getCount(); i++)
		{
			daeString material_group = trifans[i]->getMaterial();
			if (material_group)
				CHECK_warning(bind_material, 
						material_symbols.find(material_group) != material_symbols.end(),
						"binding not found for material symbol\n");
		}
		domLines_Array & lines = mesh->getLines_array();
		for (size_t i=0; i<lines.getCount(); i++)
		{
			daeString material_group = lines[i]->getMaterial();
			if (material_group)
				CHECK_warning(bind_material, 
						material_symbols.find(material_group) != material_symbols.end(),
						"binding not found for material symbol\n");
		}
		domLinestrips_Array & linestrips = mesh->getLinestrips_array();
		for (size_t i=0; i<linestrips.getCount(); i++)
		{
			daeString material_group = linestrips[i]->getMaterial();
			if (material_group)
				CHECK_warning(bind_material, 
						material_symbols.find(material_group) != material_symbols.end(),
						"binding not found for material symbol\n");
		}
	}
	return errorcount;
}

domUint CHECK_InstanceGeometry(domInstance_geometry * instance_geometry)
{
	domUint errorcount = 0;

	xsAnyURI & uri = instance_geometry->getUrl();
	domGeometry * geometry = (domGeometry *) (domElement*) uri.getElement();

	domBind_material * bind_material = instance_geometry->getBind_material();

	errorcount += CHECK_error(instance_geometry, bind_material!=0, "bind_material not exist\n");
	errorcount += CHECK_material_symbols(geometry, bind_material);

	return errorcount;
}

domUint GetVertexCountFromGeometry(domGeometry * geometry)
{
	domMesh * mesh = geometry->getMesh();
	if (mesh)
	{
		domVertices * vertices = mesh->getVertices();
		if (vertices)
		{
			domInputLocal_Array & inputs = vertices->getInput_array();
			for (size_t i=0; i<inputs.getCount(); i++)
			{
				if (stricmp(inputs[i]->getSemantic(), "POSITION") == 0)
				{
					domSource * source = (domSource*) (domElement*) inputs[i]->getSource().getElement();
					if(source)
					{
						domFloat_array * float_array = source->getFloat_array();
						if (float_array)
							return float_array->getCount();
					}
				}
			}
		}
	}
	PRINTF("ERROR: Can't get Vertices Count from geometry, something wrong here\n");
	return 0;
}

domUint CHECK_Controller(domController *controller)
{
	domUint errorcount = 0;
	domSkin * skin = controller->getSkin();
	if (skin)
	{
		xsAnyURI & uri = skin->getSource();

		domElement * element = uri.getElement();
		if (element == 0)
		{
			errorcount += CHECK_error(skin, element != 0, "can't resolve skin source\n");
			return errorcount;
		}
		daeString type_str = element->getTypeName();

		if (stricmp(type_str, "geometry") == 0)
		{	// skin is reference directly to geometry
			// get vertex count from skin
			domSkin::domVertex_weights * vertex_weights = skin->getVertex_weights();
			domUint vertex_weights_count = vertex_weights->getCount();
			domGeometry * geometry = (domGeometry*) (domElement*) uri.getElement();
			domMesh * mesh = geometry->getMesh();				
			if (mesh)
			{	// get vertex count from geometry
				domVertices * vertices = mesh->getVertices();
				CHECK_error(geometry, vertices != NULL, "geometry, vertices in this mesh\n");
				if (vertices)
				{
					xsAnyURI src = vertices->getInput_array()[0]->getSource();
					domSource * source = (domSource*) (domElement*) src.getElement();
					domUint vertices_count = source->getTechnique_common()->getAccessor()->getCount();
					errorcount += CHECK_count(controller, vertices_count, vertex_weights_count,
											"controller, vertex weight count != mesh vertex count\n");
				}
			}	// TODO: it could be convex_mesh and spline
			domUint vcount_count = (domUint) vertex_weights->getVcount()->getValue().getCount();
			errorcount += CHECK_count(controller, vcount_count, vertex_weights_count,
									  "controller, vcount count != vertex weight count\n");	
			domInputLocalOffset_Array & inputs = vertex_weights->getInput_array();
			domUint sum = 0;
			for (size_t i=0; i<vcount_count; i++)
			{
				sum += vertex_weights->getVcount()->getValue()[i];
			}
			errorcount += CHECK_count(controller, sum * inputs.getCount(), (domInt) vertex_weights->getV()->getValue().getCount(), 
									  "controller, total vcount doesn't match with numbers of v\n");

			// check index range on <v>
			domListOfInts & ints = vertex_weights->getV()->getValue();
			domUint maxoffset = GetMaxOffsetFromInputs(inputs);
			for (size_t j=0; j<maxoffset; j++)
			{
				domUint index_range = GetIndexRangeFromInput(inputs, j, errorcount);
				CHECK_Index_Range(skin, ints, index_range, j, maxoffset);
			}
		}
	}
	domMorph * morph = controller->getMorph();
	if (morph)
	{
		domUint source_geometry_vertices_count = 0;
		xsAnyURI & uri = morph->getSource();
		domElement * element = uri.getElement();
		if (element == 0)
		{
			errorcount++;
			PRINTF("ERROR: MORPH Source base mesh element does not resolve\n");
			return errorcount;
		}
		daeString type_str = element->getTypeName();
		if (stricmp(type_str, "geometry") == 0)
		{
			domGeometry * source_geometry = (domGeometry *) element;
			source_geometry_vertices_count = GetVertexCountFromGeometry(source_geometry);
		}
		domInputLocal_Array & inputs = morph->getTargets()->getInput_array();
		for (size_t i=0; i<inputs.getCount(); i++)
		{
			if(stricmp(inputs[i]->getSemantic(), "MORPH_TARGET") == 0)
			{
				domSource * source = (domSource*) (domElement*) inputs[i]->getSource().getElement();
				domIDREF_array * IDREF_array = source->getIDREF_array();
				if(IDREF_array)
				{
					xsIDREFS & ifrefs = IDREF_array->getValue();
					for (size_t j=0; j<ifrefs.getCount(); j++)
					{
						domElement * element = ifrefs[j].getElement();
						domGeometry * target_geometry = (domGeometry*) element;
						domUint target_geo_vertices_count = GetVertexCountFromGeometry(target_geometry);
						if (source_geometry_vertices_count !=target_geo_vertices_count)
						{
							errorcount++;
							PRINTF("ERROR: MORPH Target vertices counts != MORPH Source vertices counts\n");
						}
					}
				}
			}
		}
	}
	return errorcount;
}

domUint CHECK_InstanceElementUrl(daeDatabase *db, daeInt instanceElementID) {
	domUint errorcount = 0;
	vector<daeElement*> elements = db->typeLookup(instanceElementID);
	for (size_t i = 0; i < elements.size(); i++)
		errorcount += CHECK_uri(daeURI(*elements[i], elements[i]->getAttribute("url")));
	return errorcount;
}

domUint GetSizeFromType(xsNMTOKEN type)
{
	if (stricmp(type, "bool2")==0)
		return 2;
	else if (stricmp(type, "bool3")==0)
		return 3;
	else if (stricmp(type, "bool4")==0)
		return 4;
	else if (stricmp(type, "int2")==0)
		return 2;
	else if (stricmp(type, "int3")==0)
		return 3;
	else if (stricmp(type, "int4")==0)
		return 4;
	else if (stricmp(type, "float2")==0)
		return 2;
	else if (stricmp(type, "float3")==0)
		return 3;
	else if (stricmp(type, "float4")==0)
		return 4;
	else if (stricmp(type, "float2x2")==0)
		return 4;
	else if (stricmp(type, "float3x3")==0)
		return 9;
	else if (stricmp(type, "float4x4")==0)
		return 16;
	return 1;
}

domUint CHECK_Source(domSource * source)
{
	domUint errorcount = 0;
	// check if this is a source with children
	daeTArray<daeSmartRef<daeElement> >  children;
	source->getChildren(children);
	if (children.getCount() <= 0) return 0;
	// prepare technique_common 
	domSource::domTechnique_common * technique_common = source->getTechnique_common();
	domAccessor * accessor = 0;
	domUint accessor_count = 0;
	domUint accessor_stride = 0;
	domUint accessor_size = 0;
	domUint accessor_offset = 0;
	domUint array_count = 0;
	domUint array_value_count = 0;
	
	if (technique_common)
	{
		accessor = technique_common->getAccessor();
		if (accessor)
		{
			accessor_count = accessor->getCount();
			accessor_stride = accessor->getStride();
			accessor_offset = accessor->getOffset();
			domParam_Array & param_array = accessor->getParam_array();
			for(size_t i=0; i<param_array.getCount(); i++)
			{
				xsNMTOKEN type = param_array[i]->getType();
				accessor_size += GetSizeFromType(type);
			}
			errorcount += CHECK_error(source, accessor_size <= accessor_stride, "total size of all params > accessor stride!\n");
			errorcount += CHECK_error(source, accessor_size + accessor_offset <= accessor_stride, "total size of all params + offset > accessor stride!\n");
		}
	}

	if (accessor)
	{
		domElement * element = (domElement*) accessor->getSource().getElement();
		if (element == NULL)
		{
			errorcount += CHECK_error(source, element!=NULL, "accessor source can not resolve!\n");
			return errorcount;
		}

		COLLADA_TYPE::TypeEnum type = element->getElementType();

		// float_array
		if (type == COLLADA_TYPE::FLOAT_ARRAY)
		{
			domFloat_array * float_array = (domFloat_array *) element;
			array_count = float_array->getCount(); 
			array_value_count = (domUint) float_array->getValue().getCount(); 
		}
		
		// int_array
		if (type == COLLADA_TYPE::INT_ARRAY)
		{
			domInt_array * int_array = (domInt_array *) element;
			array_count = int_array->getCount(); 
			array_value_count = (domUint) int_array->getValue().getCount(); 
		} 
		
		// bool_array
		if (type == COLLADA_TYPE::BOOL_ARRAY)
		{
			domBool_array * bool_array = (domBool_array *) element;
			array_count = bool_array->getCount(); 
			array_value_count = (domUint) bool_array->getValue().getCount(); 
		}
		
		// idref_array
		if (type == COLLADA_TYPE::IDREF_ARRAY)
		{
			domIDREF_array * idref_array = (domIDREF_array *) element;
			array_count = idref_array->getCount(); 
			array_value_count = (domUint) idref_array->getValue().getCount(); 
		}
		
		// name_array
		if (type == COLLADA_TYPE::NAME_ARRAY)
		{
			domName_array * name_array = (domName_array *) element;
			array_count = name_array->getCount(); 
			array_value_count = (domUint) name_array->getValue().getCount(); 
		}  
	}

	errorcount += CHECK_count(source, array_count, array_value_count,
			                    "array count != number of name in array value_count\n");
	if (accessor)
	{
		errorcount += CHECK_count(source, array_count, accessor_count * accessor_stride, 
								"accessor_stride >= accessor_size but array_count != accessor_count * accessor_stride\n");
	}
	
	return errorcount ;
}

domUint CHECK_counts (DAE *input, int verbose)
{
//	checklist = new std::map<daeString, domElement *>;
	domInt error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();

	// check geometry
	daeInt count = (daeInt) db->getElementCount(NULL, "geometry", file_name.c_str() );
	for (daeInt i=0; i<count; i++)
	{
		domGeometry *geometry;
		error = db->getElement((daeElement**)&geometry, i, NULL, "geometry", file_name.c_str());
		errorcount += CHECK_Geometry(geometry);			
	}
	// check controller
	count = (daeInt)db->getElementCount(NULL, "controller", file_name.c_str() );
	for (daeInt i=0; i<count; i++)
	{
		domController *controller;
		error = db->getElement((daeElement**)&controller, i, NULL, "controller", file_name.c_str());
		errorcount += CHECK_Controller(controller);			
	}
	// check instance_geometry
	count = (daeInt)db->getElementCount(NULL, "instance_geometry", file_name.c_str() );
	for (daeInt i=0; i<count; i++)
	{
		domInstance_geometry *instance_geometry;
		error = db->getElement((daeElement**)&instance_geometry, i, NULL, "instance_geometry", file_name.c_str() );
		errorcount += CHECK_InstanceGeometry(instance_geometry);
	}
	// check source
	count = (daeInt)db->getElementCount(NULL, "source", file_name.c_str() );
	for (daeInt i=0; i<count; i++)
	{
		domSource *source;
		error = db->getElement((daeElement**)&source, i, NULL, "source", file_name.c_str() );
		errorcount += CHECK_Source(source);
	}
	return errorcount; 
}

domUint CHECK_links (DAE *input, int verbose)
{
	domInt error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();

	// check links
	daeInt count = (daeInt)db->getElementCount(NULL, "accessor", file_name.c_str() );
	for (daeInt i=0; i<count; i++)
	{
		domAccessor *accessor;
		error = db->getElement((daeElement**)&accessor, i, NULL, "accessor", file_name.c_str() );
		xsAnyURI & uri = accessor->getSource();
		errorcount += CHECK_uri(uri);
	}
	count = (daeInt)db->getElementCount(NULL, "channel", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domChannel *channel;
		error = db->getElement((daeElement**)&channel, i, NULL, "channel", file_name.c_str());
		xsAnyURI & uri = channel->getSource();
		errorcount += CHECK_uri(uri);
	}
	count = (daeInt)db->getElementCount(NULL, "IDREF_array", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domIDREF_array *IDREF_array;
		error = db->getElement((daeElement**)&IDREF_array, i, NULL, "IDREF_array", file_name.c_str());
		for (size_t j=0; j<IDREF_array->getCount(); j++) 
		{
			daeIDRef idref = IDREF_array->getValue()[j];
			idref.resolveElement();
			domElement * element = idref.getElement();
			if (element == NULL)
			{
				char temp[MAX_LOG_BUFFER];
				sprintf(temp, "IDREF_array value %s not referenced\n", idref.getID());
				PRINTF(temp);
				errorcount += CHECK_error(IDREF_array, element!=NULL, temp);
			}
		}
	}
	count = (daeInt)db->getElementCount(NULL, "input", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domInputLocalOffset *input;
		error = db->getElement((daeElement**)&input, i, NULL, "input", file_name.c_str());
		xsAnyURI & uri = input->getSource();
		errorcount += CHECK_uri(uri);
	}

	count = (daeInt)db->getElementCount(NULL, "skeleton", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domInstance_controller::domSkeleton *skeleton;
		error = db->getElement((daeElement**)&skeleton, i, NULL, "skeleton", file_name.c_str());
		xsAnyURI & uri = skeleton->getValue();
		errorcount += CHECK_uri(uri);
	}
	count = (daeInt)db->getElementCount(NULL, "skin", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domSkin *skin;
		error = db->getElement((daeElement**)&skin, i, NULL, "skin", file_name.c_str());
		xsAnyURI & uri = skin->getSource();
		errorcount += CHECK_uri(uri);
	}
	// physics
/*	for (size_t i=0; i<db->getElementCount(NULL, "program", NULL); i++)
	{
		domProgram *program;
		error = db->getElement((daeElement**)&program, i, NULL, "program");
		xsAnyURI & uri = program->getSource();
		errorcount += CHECK_uri(uri);
	}
*/
	count = (daeInt)db->getElementCount(NULL, "instance_rigid_body", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domInstance_rigid_body *instance_rigid_body;
		error = db->getElement((daeElement**)&instance_rigid_body, i, NULL, "instance_rigid_body", file_name.c_str());
		xsAnyURI & uri = instance_rigid_body->getTarget();
		errorcount += CHECK_uri(uri);
	}
	count = (daeInt)db->getElementCount(NULL, "ref_attachment", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domRigid_constraint::domRef_attachment *ref_attachment;
		error = db->getElement((daeElement**)&ref_attachment, i, NULL, "ref_attachment", file_name.c_str());
		xsAnyURI & uri = ref_attachment->getRigid_body();
		errorcount += CHECK_uri(uri);
	}

	// FX, todo: color_target, connect_param, depth_target, param, stencil_target
	count = (daeInt)db->getElementCount(NULL, "instance_material", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domInstance_material *instance_material;
		error = db->getElement((daeElement**)&instance_material, i, NULL, "instance_material", file_name.c_str());
		xsAnyURI & uri = instance_material->getTarget();
		errorcount += CHECK_uri(uri);
	}


	// urls
	daeInt instance_elements[] = {
		domInstanceWithExtra::ID(), // instance_animation, instance_visual_scene, instance_physics_scene
		domInstance_camera::ID(),
		domInstance_controller::ID(),
		domInstance_geometry::ID(),
		domInstance_light::ID(),
		domInstance_node::ID(),
		domInstance_effect::ID(),
		domInstance_force_field::ID(),
		domInstance_physics_material::ID(),
		domInstance_physics_model::ID()
	};
	domUint instance_elements_max = sizeof(instance_elements)/sizeof(daeInt);
	
	for (size_t i=0; i<instance_elements_max ; i++) 
	{
		errorcount += CHECK_InstanceElementUrl(db, instance_elements[i]);
	}
	return errorcount;
}


domUint CHECK_files (DAE *input, int verbose)
{
	domInt error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();

	// files
	daeInt count = (daeInt) db->getElementCount(NULL, "image", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domImage *image;
		error = db->getElement((daeElement**)&image, i, NULL, "image", file_name.c_str());
		domImage::domInit_from * init_from	= image->getInit_from();
		domImage::domData * data			= image->getData();
		errorcount += CHECK_error(image, init_from || data, "image, exactly one of the child element <data> or <init_from> must occur\n");
		if (init_from)
		{
			xsAnyURI & uri = init_from->getValue();
			errorcount += CHECK_file(init_from, uri);
		}
	}
	count = (daeInt) db->getElementCount(NULL, "include", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domFx_include_common *include;
		error = db->getElement((daeElement**)&include, i, NULL, "include", file_name.c_str());
		xsAnyURI & uri = include->getUrl();
		errorcount += CHECK_file(include, uri);
	}
	return errorcount;
}

domUint CHECK_escape_char(daeString str)
{
	domUint errorcount = 0;
	size_t len = strlen(str);
	for(size_t i=0; i<len; i++)
	{
		switch(str[i])
		{
		case ' ':
		case '#':
		case '$':
		case '%':
		case '&':
		case '/':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '@':
		case '[':
		case '\\':
		case ']':
		case '^':
		case '`':
		case '{':
		case '|':
		case '}':
		case '~':
			char temp[1024];
			sprintf(temp, "ERROR: string '%s' contains non-escaped char '%c'\n", str, str[i]);
			PRINTF(temp);
			errorcount++;
		default:
			continue;
		}
	}
	return errorcount;
}

domUint CHECK_unique_id (DAE *input, int verbose)
{
	std::pair<std::set<string>::iterator, bool> pair;
	std::set<string> ids;
	domInt error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();
	daeInt count = (daeInt) db->getElementCount(NULL, NULL, NULL);
	for (daeInt i=0; i<count; i++)
	{
		domElement *element;
		error = db->getElement((daeElement**)&element, i, NULL, NULL, NULL);
		daeString id = element->getID();
		if (id == NULL) continue;

		errorcount += CHECK_escape_char(id);

		daeString docURI = element->getDocumentURI()->getURI();
		// check if there is second element with the same id.
		error = db->getElement((daeElement**)&element, 1, id, NULL, docURI);
		if (error == DAE_OK)
		{
			errorcount++;
			char temp[MAX_LOG_BUFFER];
			sprintf(temp, "ERROR: Unique ID conflict id=%s, docURI=%s\n", id, docURI);
			PRINTF(temp);
		}
	}
	return errorcount;
}

domEffect *TextureGetEffect(domCommon_color_or_texture_type::domTexture *texture)
{
	for (domElement * element = texture; element; element = element->getParentElement())
	{
		if (element->getTypeName())
			if (stricmp(element->getTypeName(), "effect") == 0)
				return (domEffect *)element;
	}
	return NULL;
}

domUint CHECK_texture (DAE *input, int verbose)
{
	std::pair<std::set<string>::iterator, bool> pair;
	std::set<string> ids;
	domInt error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();

	daeInt count = (daeInt) db->getElementCount(NULL, "texture", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domCommon_color_or_texture_type::domTexture *texture;
		error = db->getElement((daeElement**)&texture, i, NULL, "texture", file_name.c_str());
		xsNCName texture_name = texture->getTexture();
		char * target = new char[ strlen( texture_name ) + 3 ];
		strcpy( target, "./" );
		strcat( target, texture_name );
		domEffect * effect = TextureGetEffect(texture);
		if (effect==NULL) 
			continue;
		daeSIDResolver sidRes( effect, target );
		delete[] target;
		target = NULL;
		if ( sidRes.getElement() != NULL )
		{	// it is doing the right way
			continue;
		}
		daeIDRef ref(texture_name);
		ref.setContainer(texture);
		ref.resolveElement();
		daeElement * element = ref.getElement();
		if (element)
		{	// it is directly linking the image
			char temp[MAX_LOG_BUFFER];
			sprintf(temp, "ERROR: CHECK_texture failed, texture=%s is direct linking to image\n", texture_name);
			PRINTF(temp);
		} else {
			char temp[MAX_LOG_BUFFER];
			sprintf(temp, "ERROR: CHECK_texture failed, texture=%s is not link to anything\n", texture_name);
			PRINTF(temp);
		}
	}
	return errorcount;
}

void _XMLSchemaValidityErrorFunc(void* ctx, const char* msg, ...)
{
	va_list      LVArgs;
	char         LTmpStr[MAX_LOG_BUFFER];    // FIXME: this is not buffer-overflow safe
	memset(LTmpStr,0,MAX_LOG_BUFFER);

//	DAEDocument* LDAEDocument = (DAEDocument*)ctx;
//	xmlSchemaValidCtxt* LXMLSchemaValidContext = (xmlSchemaValidCtxt*) ctx;
//	xmlNode*     LXMLNode = xmlSchemaValidCtxtGetNode(ctx);
//	xmlDoc *	 doc = (xmlDoc *) ctx;

	va_start(LVArgs, msg);
	vsprintf(LTmpStr, msg, LVArgs);
	va_end(LVArgs);
//	PRINTF("%s:%d  Schema validation error:\n%s", LDAEDocument->Name, xmlGetLineNo(LXMLNode), LTmpStr);
//	PRINTF("CHECK_schema Error msg=%c ctx=%p\n", msg, ctx);
	char temp[MAX_LOG_BUFFER];
	memset(temp,0,MAX_LOG_BUFFER);
	sprintf(temp, "ERROR: CHECK_schema Error   msg=%s", LTmpStr);
 	PRINTF(temp);
	fileerrorcount++;
}


void _XMLSchemaValidityWarningFunc(void* ctx, const char* msg, ...)
{
	va_list      LVArgs;
	char         LTmpStr[MAX_LOG_BUFFER];    // FIXME: this is not buffer-overflow safe
	memset(LTmpStr,0,MAX_LOG_BUFFER);
//	DAEDocument* LDAEDocument = (DAEDocument*)ctx;
//	xmlNode*     LXMLNode = xmlSchemaValidCtxtGetNode(LDAEDocument->XMLSchemaValidContext);
//	xmlDoc *	 doc = (xmlDoc *) ctx;

	va_start(LVArgs, msg);
	vsprintf(LTmpStr, msg, LVArgs);
	va_end(LVArgs);
//	PRINTF("%s:%d  Schema validation warning:\n%s", LDAEDocument->Name, xmlGetLineNo(LXMLNode), LTmpStr);
	char temp[MAX_LOG_BUFFER];
	memset(temp,0,MAX_LOG_BUFFER);
	sprintf(temp, "ERROR: CHECK_schema Warning msg=%s", LTmpStr);
	PRINTF(temp);
	fileerrorcount++;
}

//void dae_ValidateDocument(DAEDocument* LDAEDocument, xmlDocPtr LXMLDoc)
domUint CHECK_validateDocument(xmlDocPtr LXMLDoc)
{
//	const char * dae_SchemaURL = "C:\\svn\\COLLADA_DOM\\doc\\COLLADASchema.xsd";
//	const char * dae_SchemaURL = "http://www.collada.org/2005/11/COLLADASchema.xsd";
	const char * dae_SchemaURL = xmlschema_file.c_str();

	xmlSchemaParserCtxt*  Ctxt = xmlSchemaNewDocParserCtxt(LXMLDoc);
	xmlSchemaParserCtxt*    LXMLSchemaParserCtxt = xmlSchemaNewParserCtxt(dae_SchemaURL);

	if(LXMLSchemaParserCtxt)
	{
		xmlSchema*    LXMLSchema = xmlSchemaParse(LXMLSchemaParserCtxt);

		if(LXMLSchema)
		{
			xmlSchemaValidCtxt*    LXMLSchemaValidContext = xmlSchemaNewValidCtxt(LXMLSchema);

			if(LXMLSchemaValidContext)
			{	
				int    LSchemaResult;

//				LDAEDocument->XMLSchemaValidContext = LXMLSchemaValidContext;

/*				xmlSchemaSetParserErrors(LXMLSchemaParserCtxt,
							_XMLSchemaValidityErrorFunc,
							_XMLSchemaValidityWarningFunc,
							LXMLDoc);
*/
//				globalpointer = this;
				xmlSchemaSetValidErrors(LXMLSchemaValidContext,
							_XMLSchemaValidityErrorFunc,
							_XMLSchemaValidityWarningFunc,
							LXMLDoc);

				LSchemaResult = xmlSchemaValidateDoc(LXMLSchemaValidContext, LXMLDoc);
//				switch(LSchemaResult)
//				{
//				case 0:    PRINTF("Document validated\n");break;
//				case -1:   PRINTF("API error\n");break;
//				default:   PRINTF("Error code: %d\n", LSchemaResult);break;
//				}

//			LDAEDocument->XMLSchemaValidContext = NULL;
			xmlSchemaFreeValidCtxt(LXMLSchemaValidContext);
			}

			xmlSchemaFree(LXMLSchema);
		}
		else {
			char temp[MAX_LOG_BUFFER];
			sprintf(temp, "ERROR: Could not parse schema from %s\n", dae_SchemaURL);
			PRINTF(temp);
		}
		xmlSchemaFreeParserCtxt(LXMLSchemaParserCtxt);
	}
	else {
		char temp[MAX_LOG_BUFFER];
		sprintf(temp, "ERROR: Could not load schema from '%s\n", dae_SchemaURL);
		PRINTF(temp);
	}
	return 0;
}


domUint CHECK_xmlfile(daeString filename)
{
	xmlDocPtr doc;		
	doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
		char temp[MAX_LOG_BUFFER];
		sprintf(temp, "ERROR: Failed to parse %s\n", filename);
		PRINTF(temp);
		return 1;
    } else {
		// load okay
		CHECK_validateDocument(doc);
		/* free up the parser context */
		xmlFreeDoc(doc);
	}
	return 0;
}

domUint CHECK_schema (DAE *input, int verbose)
{
	domInt errorcount = 0;
	daeDatabase* db = input->getDatabase();
	daeInt count = (daeInt) db->getDocumentCount();
	for (daeInt i=0; i<count; i++)
	{
		daeDocument* doc = db->getDocument(i);
		daeString xmldoc = doc->getDocumentURI()->getURI();
		errorcount += CHECK_xmlfile(xmldoc);
	}
	return errorcount;
}

domUint CHECK_inputs (domInputLocalOffset_Array & inputs, const char * semantic)
{
	domUint count = (domUint) inputs.getCount();
	for (size_t i=0; i<count; i++) 
	{
		if (stricmp(semantic, inputs[i]->getSemantic()) == 0)
			return 0;
	}
	PRINTF("ERROR: CHECK inputs Failed ");
	print_name_id(inputs[0]);
	char temp[MAX_LOG_BUFFER];
	sprintf(temp, " input with semantic=%s not found\n", semantic);
	PRINTF(temp);
	return 1;
}

domUint CHECK_inputs (domInputLocal_Array & inputs, const char * semantic)
{
	domUint count = (domUint) inputs.getCount();
	for (size_t i=0; i<count; i++) 
	{
		if (stricmp(semantic, inputs[i]->getSemantic()) == 0)
			return 0;
	}
	PRINTF("ERROR: CHECK inputs Failed ");
	print_name_id(inputs[0]);
	char temp[MAX_LOG_BUFFER];
	sprintf(temp, " input with semantic=%s not found\n", semantic);
	PRINTF(temp);
	return 1;
}

domUint CHECK_skin (DAE *input, int verbose)
{
	std::vector<domNode *> nodeset;
	domInt error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();

	daeInt count = (daeInt) db->getElementCount(NULL, "instance_controller", file_name.c_str());
	for (daeInt i=0; i<count; i++)
	{
		domInstance_controller *instance_controller;
		error = db->getElement((daeElement**)&instance_controller, i, NULL, "instance_controller", file_name.c_str());

		// get all skeletons
		domInstance_controller::domSkeleton_Array & skeleton_array = instance_controller->getSkeleton_array();
		domUint skeleton_count = (domUint) skeleton_array.getCount();
		for (size_t i=0; i<skeleton_count; i++)
		{
			domNode * node = (domNode*) (domElement*) skeleton_array[i]->getValue().getElement();
			nodeset.push_back(node);
		}

		//get joints from skin
		domController * controller = (domController*) (domElement*) instance_controller->getUrl().getElement();
		if (controller==NULL) continue;

		domBind_material * bind_material = instance_controller->getBind_material();
		errorcount += CHECK_error(instance_controller, bind_material!=0, "bind_material not exist\n");

		domMorph * morph = controller->getMorph();
		if (morph)
		{
			domGeometry * geometry = (domGeometry*) (domElement*) morph->getSource().getElement();
			errorcount += CHECK_material_symbols(geometry, bind_material);
		}

		domSkin * skin = controller->getSkin();
		if (skin)
		{
			domElement * element = (domElement*) skin->getSource().getElement();
			if (element == NULL) 
			{
				errorcount += CHECK_error(controller, false, "Skin source not found\n");				
				continue;
			}
			COLLADA_TYPE::TypeEnum type = element->getElementType();
			switch(type)
			{
			case COLLADA_TYPE::GEOMETRY:
				{
				domGeometry * geometry = (domGeometry*) element;
				errorcount += CHECK_material_symbols(geometry, bind_material);
				break;
				}
			case COLLADA_TYPE::MORPH:
				{
				domMorph * morph = (domMorph*) element;
				domGeometry * geometry = (domGeometry*) (domElement*) morph->getSource().getElement();
				errorcount += CHECK_material_symbols(geometry, bind_material);
				break;
				}
			default:
				break;
			}

			// get source of name_array or IDREF_array
			domSource * joint_source = NULL;
			domSource * inv_bind_matrix_source = NULL;
			domSkin::domJoints * joints = skin->getJoints();
			if (joints==NULL) continue;
			domInputLocal_Array &input_array = joints->getInput_array();
			for(size_t i=0; i<input_array.getCount(); i++)
			{
				if (stricmp(input_array[i]->getSemantic(), "JOINT") == 0)
				{
					joint_source = (domSource *) (domElement*) input_array[i]->getSource().getElement();
					if (joint_source) continue;
				} else if (stricmp(input_array[i]->getSemantic(), "INV_BIND_MATRIX") == 0)
				{
					inv_bind_matrix_source = (domSource *) (domElement*) input_array[i]->getSource().getElement();
					if (inv_bind_matrix_source) continue;					
				}
			}
			if (joint_source == NULL) continue;
			if (inv_bind_matrix_source == NULL) continue;

			//check count of joint source and inv_bind_matrix_source
			domSource::domTechnique_common * techique_common = NULL;
			domAccessor * accessor = NULL;
			domUint joint_count = 0;
			domUint inv_bind_matrix_count = 0;
			techique_common = joint_source->getTechnique_common();
			if (techique_common) 
			{
				accessor = techique_common->getAccessor();
				if (accessor)
					domUint joint_count = accessor->getCount();
			}
			techique_common = inv_bind_matrix_source->getTechnique_common();
			if (techique_common) 
			{
				accessor = techique_common->getAccessor();
				if (accessor)
					domUint inv_bind_matrix_count = accessor->getCount();
			}
			errorcount += CHECK_count(skin, joint_count, inv_bind_matrix_count,
				"WARNING, joint count and inv bind matrix count does not match.\n");

			//name_array
			domName_array * name_array = joint_source ->getName_array();
			domIDREF_array * idref_array = joint_source ->getIDREF_array();
			if (name_array)
			{
				domListOfNames &list_of_names = name_array->getValue();
				domUint name_count = (domUint) list_of_names.getCount();
				for (domUint j=0; j<name_count; j++)
				{
					char jointpath[MAX_PATH];
					strcpy( jointpath, "./" );
					strcat( jointpath, list_of_names[(size_t)j] );
					domElement * e = NULL;
					for (size_t k=0; k<nodeset.size(); k++)
					{
						daeSIDResolver sidRes( nodeset[k], jointpath );
						e = sidRes.getElement();
						if (e) break;
					}
					if (e==NULL) // this joint name is not match with any sid of skeleton nodes
					{
						char tempstr[MAX_PATH];
						sprintf(tempstr, "instance_controller, can't find node with sid=%s of controller=%s\n", list_of_names[(size_t)j], controller->getId());
						errorcount += CHECK_error(instance_controller, false, tempstr);								
					}
				}
			} else if (idref_array)
			{
				xsIDREFS & list_of_idref = idref_array->getValue();
				domUint idref_count = (domUint) list_of_idref.getCount();
				for (domUint j=0; j<idref_count; j++)
				{
					list_of_idref[(size_t)j].resolveElement();
					domElement * element = list_of_idref[(size_t)j].getElement();
					if (element == 0)
					{
						char tempstr[MAX_PATH];
						sprintf(tempstr, "skin, idref=%s can't resolve\n", list_of_idref[(size_t)j].getID());
						errorcount +=CHECK_error(joint_source, false, tempstr);
					}
				}
			} else { // name_array and IDREF_array not found
				errorcount +=CHECK_error(skin, false, "skin, both name_array and IDREF_array are not found");
			}			
		} else // TODO: for other non-skin controllers
		{
		}
	}

	return errorcount;
}

domUint CHECK_float_array (DAE *input, int verbose)
{
	domInt count = 0;
	domInt error = 0;
	domInt errorcount=0;
	daeDatabase *db = input->getDatabase();
	count = (daeInt) db->getElementCount(NULL, COLLADA_ELEMENT_FLOAT_ARRAY, file_name.c_str());
	for (int i=0; i<count; i++)
	{
		domFloat_array * float_array;
		error = db->getElement((daeElement**)&float_array, i, NULL, COLLADA_ELEMENT_FLOAT_ARRAY, file_name.c_str());
		domListOfFloats & listoffloats = float_array->getValue();
		for (size_t j=0; j<listoffloats.getCount(); j++)
		{
			if ( (listoffloats[j] == 0x7f800002) ||
				 (listoffloats[j] == 0x7f800000) ||
				 (listoffloats[j] == 0xff800000) )
			{
				errorcount++;
				PRINTF("ERROR: Float_array contain either -INF, INF or NaN\n");
			}	 
		}
	}
	return errorcount;
}

enum eVISIT
{
    NOT_VISITED = 0,
    VISITING,
    VISITED,
};

domUint CHECK_node (domNode * parent_node, std::map<domNode*,eVISIT> * node_map)
{
	domUint errorcount = 0;
//	eVISIT result = (*node_map)[parent_node];
	switch((*node_map)[parent_node])
	{
	case VISITING:// means we are visiting this node again "circular reference detected.
		PRINTF("ERROR: circular reference detected.\n");
		return 1; 
	case VISITED: // means we already visited this node.
		return 0; 
	default:      // means we are visiting this node the first time.
		(*node_map)[parent_node] = VISITING;
		break;
	}

	domNode_Array & node_array = parent_node->getNode_array();
	for (size_t i=0; i<node_array.getCount(); i++)
	{
		domNode * node = node_array[i];
		if (node) errorcount += CHECK_node(node, node_map);
	}
	
	domInstance_node_Array & instance_node_array = parent_node->getInstance_node_array();
	for (size_t i=0; i<instance_node_array.getCount(); i++)
	{
		domNode * node = (domNode*) (domElement*) instance_node_array[i]->getUrl().getElement();
		if (node) errorcount += CHECK_node(node, node_map);
	}

/*	domInstance_controller_Array instance_controller_array & parent_node->getInstance_controller_array();
	for (size_t i=0; i<instance_controller_array.getCount(); i++)
	{
		instance_controller_array[i]->getSkeleton_array()
		domNode * node = (domNode*) (domElement*) ->getUrl().getElement();
		CHECK_node(node);
	}
*/
	
	(*node_map)[parent_node] = VISITED;
	return errorcount;
}

domUint CHECK_Circular_Reference (DAE *input, int verbose)
{
	domInt errorcount=0;

	std::map<domNode*,eVISIT> node_map;
	domCOLLADA * collada = input->getDom(file_name.c_str());
	domCOLLADA::domScene * scene = collada->getScene();
	if (scene == NULL) return 0;
	domInstanceWithExtra * instance_scene = scene->getInstance_visual_scene();
	if (instance_scene == NULL) return 0;
	domVisual_scene * visual_scene = (domVisual_scene*) (domElement*) instance_scene->getUrl().getElement();
	if (visual_scene == NULL) return 0;
	domNode_Array & node_array = visual_scene->getNode_array();
	for (size_t i=0; i<node_array.getCount(); i++)
	{
		domNode * node = node_array[i];
		CHECK_node(node, &node_map);
	}

	return errorcount;
}

// check if an index is indexing out of range
domUint CHECK_Index_Range (domElement * elem, domListOfUInts & listofint, domUint index_range, domUint offset, domUint maxoffset, int verbose)
{
	if (checklist["INDEX_RANGE"] == false) return 0;
	domInt errorcount=0;
	if (index_range == 0) 
	{
		errorcount += CHECK_error(elem, index_range != 0, "index_range == 0, can't complete CHECK_Index_Range index range checks if we are indexing out of range of a source\n");
		return errorcount;
	}
	char message[1024];
	for (size_t i=(size_t)offset; i<listofint.getCount(); i=(size_t)i+(size_t)maxoffset)
	{
		if (listofint[i] >= index_range)
		{
			sprintf(message, "Index out of range, index=%d < ranage=%d\n", (int) listofint[i], (int) index_range);
			errorcount += CHECK_error(elem, listofint[i] < index_range, message);			
		}
	}
	return errorcount;
}
domUint CHECK_Index_Range (domElement * elem, domListOfInts & listofint, domUint index_range, domUint offset, domUint maxoffset, int verbose)
{
	if (checklist["INDEX_RANGE"] == false) return 0;
	domInt errorcount=0;
	if (index_range == 0) 
	{
		errorcount += CHECK_error(elem, index_range != 0, "index_range == 0, can't complete CHECK_Index_Range\n");
		return errorcount;
	}
	char message[1024];
	for (size_t i=(size_t)offset; i<listofint.getCount(); i=(size_t)i+(size_t)maxoffset)
	{
		if (listofint[i] >= (domInt) index_range)
		{
			sprintf(message, "Index out of range, index=%d < ranage=%d\n", (int) listofint[i], (int) index_range);
			errorcount += CHECK_error(elem, listofint[i] < (domInt) index_range, message);			
		}
	}
	return errorcount;
}

domUint CHECK_validSid(domElement * elem, daeString sid)
{
	size_t len = strlen(sid);
	for (size_t i=0; i<len; i++)
	{
		switch(sid[i])
		{
//		case '.':	// . is still allowed in 1.4
		case '/':
		case '(':
		case ')':
			{
				string message = string("Sid=") + string(sid) + string(", is not a valid sid\n");
				CHECK_error(NULL, false, message.c_str());
			}
			return 1;
		default:
			break;
		}
	}
	return 0;
}

domUint CHECK_sid(DAE *input, int verbose)
{
	domInt error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();	

	{
		daeString element_name = "node";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domNode *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "color";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domCommon_color_or_texture_type_complexType::domColor *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_animation";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstanceWithExtra *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_camera";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_camera *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_controller";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_controller *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_geometry";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_geometry *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_light";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_light *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_node";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_node *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_visual_scene";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstanceWithExtra *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "lookat";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domLookat *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "matrix";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domMatrix *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
/*	{
		daeString element_name = "param";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domParam *obj; // there many domParam, which one should I use?
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
*/	{
		daeString element_name = "rotate";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domRotate *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "scale";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domScale *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "skew";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domSkew*obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "translate";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domTranslate *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_force_field";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_force_field *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_physics_matrial";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_physics_material *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_physics_model";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_physics_model *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_physics_scene";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstanceWithExtra *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_rigid_body";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_rigid_body *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "rigid_body";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domRigid_body *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "code";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domFx_code_profile *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "include";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domFx_include_common *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_effect";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_effect *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "instance_material";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domInstance_material *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "fx_newparam_common";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domFx_newparam_common *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "common_newparam_type";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domFx_newparam_common *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "fx_newparam";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domFx_newparam_common *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "glsl_newparam";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domGlsl_newparam *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "cg_newparam";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domGles_newparam *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "gles_newparam";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domCg_newparam *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	{
		daeString element_name = "pass";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domProfile_GLES::domTechnique::domPass *objGLES;
			error = db->getElement((daeElement**)&objGLES, i, NULL, element_name, file_name.c_str() );
			if (objGLES) if (objGLES->getSid()) errorcount += CHECK_validSid(objGLES, objGLES->getSid());
			domProfile_CG::domTechnique::domPass *objCG;
			error = db->getElement((daeElement**)&objCG, i, NULL, element_name, file_name.c_str() );
			if (objCG) if (objCG->getSid()) errorcount += CHECK_validSid(objCG, objCG->getSid());
			domProfile_CG::domTechnique::domPass *objGLSL;
			error = db->getElement((daeElement**)&objGLSL, i, NULL, element_name, file_name.c_str() );
			if (objGLSL) if (objGLSL->getSid())	errorcount += CHECK_validSid(objGLSL, objGLSL->getSid());
		}
	}
	{
		daeString element_name = "sampler_state";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domGles_sampler_state *objGLES;
			error = db->getElement((daeElement**)&objGLES, i, NULL, element_name, file_name.c_str() );
			if (objGLES) if (objGLES->getSid()) errorcount += CHECK_validSid(objGLES, objGLES->getSid());
		}
	}
	{// TODO: Law: This code is turned off because I can't fix a objGLES->getSid() problem that happens in release mode only
/*		daeString element_name = "technique";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domProfile_CG::domTechnique *objCG;
			error = db->getElement((daeElement**)&objCG, i, NULL, element_name, file_name.c_str() );
			if (objCG) if (objCG->getSid()) errorcount += CHECK_validSid(objCG, objCG->getSid());

			domProfile_GLES::domTechnique *objGLES;
			error = db->getElement((daeElement**)&objGLES, i, NULL, element_name, file_name.c_str() );
			if (objGLES) if (objGLES->getSid()) errorcount += CHECK_validSid(objGLES, objGLES->getSid());

			domProfile_GLSL::domTechnique *objGLSL;
			error = db->getElement((daeElement**)&objGLSL, i, NULL, element_name, file_name.c_str() );
			if (objGLSL) if (objGLSL->getSid())	errorcount += CHECK_validSid(objGLSL, objGLSL->getSid());
		}
*/	}
	{
		daeString element_name = "texture_pipeline";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domGles_texture_pipeline_complexType *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(NULL, obj->getSid());
		}
	}
	{
		daeString element_name = "texture_unit";
		daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
		for (daeInt i=0; i<count; i++)
		{
			domGles_texture_unit *obj;
			error = db->getElement((daeElement**)&obj, i, NULL, element_name, file_name.c_str() );
			if (obj)
			if (obj->getSid())
				errorcount += CHECK_validSid(obj, obj->getSid());
		}
	}
	return errorcount;
}

CoherencyTestErrorHandler::CoherencyTestErrorHandler() {
}

CoherencyTestErrorHandler::~CoherencyTestErrorHandler() {
}

void CoherencyTestErrorHandler::handleError( daeString msg ) {
	char temp[MAX_LOG_BUFFER];
	memset(temp,0,MAX_LOG_BUFFER);
	sprintf(temp, "ERROR: DOM Error Handler msg=%s", msg);
 	PRINTF(temp);
	fileerrorcount++;
}

void CoherencyTestErrorHandler::handleWarning( daeString msg ) {
	char temp[MAX_LOG_BUFFER];
	memset(temp,0,MAX_LOG_BUFFER);
	sprintf(temp, "WARNING: DOM Warning Handler msg=%s", msg);
 	PRINTF(temp);
	fileerrorcount++;
}

domUint CHECK_morph(DAE *input, int verbose)
{
	domUint error = 0;
	domUint errorcount = 0;
	daeDatabase *db = input->getDatabase();	

	daeString element_name = "morph";
	daeInt count = (daeInt)db->getElementCount(NULL, element_name, file_name.c_str() );
	for (daeInt i=0; i<count; i++)
	{
		domMorph *morph = 0;
		error = db->getElement((daeElement**)&morph, i, NULL, element_name, file_name.c_str() );
		if (morph)
		{
			domMorph::domTargetsRef targets = morph->getTargets();
			domInputLocal_Array & inputs = targets->getInput_array();
			domUint morph_weight_count = 0;
			domUint morph_target_count = 0;

			for (size_t i=0; i<inputs.getCount(); i++)
			{
				const char * semantic = inputs[i]->getSemantic();
				domSource * source = (domSource*) (domElement*) inputs[i]->getSource().getElement();
				if (source && semantic)
				{
					if (stricmp(semantic, "MORPH_TARGET")==0)
					{
						domIDREF_array * idref_array = source->getIDREF_array();
						morph_target_count = idref_array->getCount();
						xsIDREFS & idrefs = source->getIDREF_array()->getValue();

						domUint last_vertices_counts = 0;
						for (size_t j=0; j<morph_target_count; j++) // for each target geometry
						{
							domGeometry * geometry = (domGeometry*) (domElement*) idrefs[j].getElement();
							errorcount += CHECK_error(geometry, geometry != NULL, "morph target not found\n");
							if (geometry==NULL)	continue;
							domMesh * mesh = geometry->getMesh();
							if (mesh==NULL)	continue;
							domVertices * vertices= mesh->getVertices();
							if (vertices==NULL)	continue;
							for (size_t k=0; k<vertices->getInput_array().getCount(); k++)
							{
								if (stricmp(vertices->getInput_array()[k]->getSemantic(), "POSITION")==0)
								{
									domSource * source = (domSource*) (domElement*) vertices->getInput_array()[k]->getSource().getElement();
									if (source)
									{
										domUint vertices_counts = source->getFloat_array()->getCount();
										if (last_vertices_counts == 0) 
											last_vertices_counts = vertices_counts;
										else 
											errorcount += CHECK_error(idref_array, last_vertices_counts == vertices_counts, "morph target vertices count doesn't match\n");
										break;
									}
								}
							}
						}
					} else if (stricmp(semantic, "MORPH_WEIGHT")==0)
					{
						morph_weight_count = source->getFloat_array()->getCount();
					}
				}
			}

			errorcount += CHECK_error(morph, morph_target_count != 0, "Morph: target count can't be zero\n");				
			errorcount += CHECK_error(morph, morph_weight_count != 0, "Morph: weight count can't be zero\n");				
			errorcount += CHECK_error(morph, morph_weight_count == morph_target_count, "Morph: target count and weight count doesn't match\n");				
		}
	}

	return errorcount;
}


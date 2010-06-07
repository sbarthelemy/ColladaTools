#include "daeFilter.h"



using namespace std;


enum eRemove {
	REMOVE_INFO = 0,
	REMOVE_ANIM,
	REMOVE_VISU,
	REMOVE_PHYSICS,
	REMOVE_TOTAL
};


const char *removeArgs[REMOVE_TOTAL] = 
{
	"-noinfo",
	"-noanim",
	"-novisu",
	"-nophys"
};


const char *args[] = 
{
	"domtest.exe",
	"astro.dae",
	//"-o",
	//"test.dae",
	//"-noinfo",
	"-noanim",
	//"-novisu"
};

void nodeRemove(daeElement *root, const char *nodename);




class DaeVector
{
    public:
    int n, nmax;
    daeElement **vec;

    DaeVector(void);
    void add(daeElement *e);
    void remove(daeElement *e);
    void remove(DaeVector *dv);
};

DaeVector::DaeVector(void)
{
    n = 0;
    nmax = 32;
    vec = (daeElement**)malloc(sizeof(daeElement*)*nmax);
}

void DaeVector::add(daeElement *e)
{
    // Ajouter que si pas déjà présent...
    for (int i=0; i<n; i++)
    {
        if (e == vec[i]) return;
    }
    
    if (n == nmax){
        nmax *= 2;
        vec = (daeElement**)realloc(vec, sizeof(daeElement*)*nmax);
    }
    
    vec[n++] = e;
}

void DaeVector::remove(daeElement *e)
{
    // Ajouter que si pas déjà présent...
    for (int i=0; i<n; i++)
    {
        if (e == vec[i]) {
            n--;
            vec[i] = vec[n];
            return;
        }
    }
}

void DaeVector::remove(DaeVector *dv)
{
    for (int i=0; i<dv->n; i++)
    {
        remove(dv->vec[i]);
    }
}
			
			
			
void addToVector(DaeVector *dv, daeElement *node);
			

int main (int argc, char * const argv[]) {


	std::vector<char *> file_list;	// Input files
	std::vector<char *> file_out;	// Output filenames
	char outname[MAX_NAME_SIZE];
	bool toRemove[REMOVE_TOTAL];
	
	DaeVector *toDel = new DaeVector();
	DaeVector *toKeep = new DaeVector();
	
	
	for (int i=0; i<REMOVE_TOTAL; i++){
		toRemove[i] = false;
	}
	
	
	if (argc == 1){
		printf("using default debug arguments\n");
		argc = 3;//7;
		argv = (char* const *)args;
	}

	// Parse arguments
	for (int i=1; i<argc; i++){
		// Filename given ?
		if (argv[i][0] != '-'){
			// Add file to stack
			file_list.push_back(argv[i]);
		} else {
			// Process options
			if (stricmp(argv[i], "-o") == 0){
				// Output filename found
				if (i + 1 == argc) {
					break;
				} else {
					i++;
					file_out.push_back(argv[i]);
				}
			} else {
				for (int j=0; j<REMOVE_TOTAL; j++){
					if (stricmp(argv[i], removeArgs[j]) == 0){
						toRemove[j] = true;
					}
				}
			}
		}	
	}

	if (file_list.size() == 0) {
		printf("No file given…\n");
		return 0;
	}

	DAE *dae = new DAE;

	for (int i=0; i<file_list.size(); i++){
			
		//domCOLLADA* root = dae.open(file_list[i]);
		daeElement *root = (daeElement*)dae->open(file_list[i]);

		if (!root) {
			cout << "Failed to import " << file_list[i] << "\n";
			break;
		} 
		
		
		// Remove all unnecessary info...
		if (toRemove[REMOVE_INFO]){
			nodeRemove(root, "asset");
		}

		if (toRemove[REMOVE_ANIM]){
		    addToVector(toDel, root->getDescendant("library_animations"));
			nodeRemove(root, "library_animations");
		} else {
		    addToVector(toKeep, root->getDescendant("library_animations"));
		}
		
		if (toRemove[REMOVE_PHYSICS]){
			nodeRemove(root, "library_physics_scenes");
		}
		
		if (toRemove[REMOVE_VISU]){
			nodeRemove(root, "library_lights");
			nodeRemove(root, "library_images");
			nodeRemove(root, "library_materials");
			nodeRemove(root, "library_effects");
			
			// Avant de virer la visu, recuperer la liste des noeuds de geometrie...
            addToVector(toDel, root->getDescendant("library_visual_scenes"));
			
			nodeRemove(root, "library_visual_scenes");
			
		} else {
		    addToVector(toKeep, root->getDescendant("library_visual_scenes"));
		}
		
		// Filtrer la geometrie...
		toDel->remove(toKeep);
		for (int n=0; n<toDel->n; n++)
		{
		    root->removeFromParent(toDel->vec[n]);
		}
		

		// Find correct output name
		if (i < file_out.size()){
			strcpy(outname, file_out[i]);
		} else {
			int len = strlen(file_list[i]);
			strcpy(outname, file_list[i]);
			
			if (len > 4){
				// Remove extension and add -filter.dae
				outname[strlen(outname) - 4] = 0;
			}

			strcat(outname, "-new.dae");
		}
		
		
		dae->add(outname);
		dae->setRoot(outname, (domCOLLADA*)root);
		dae->write(outname);
	}
	
	
    return 0;
}




void nodeRemove(daeElement *root, const char *nodename)
{
	daeElement* node = root->getDescendant(nodename);
	
	if (node){
		root->removeFromParent(node);
		printf("Removed %s\n", nodename);
	} else printf("%s not found\n", nodename);
	
	/*
	 daeElement* node = root->getDescendant("library_animations");
	 if (!node)
	 cout << "No nodes found\n";
	 else
	 cout << "library_animations found" << endl;
	 //cout << "node id: " << node->getAttribute("id") << endl;
	 
	 if (!root->removeFromParent(node)){
	 cout << "failed to remove node..." << endl;
	 }
	 */
}




void addToVector(DaeVector *dv, daeElement *node)
{
    if (node == NULL) return;
    
    // Chercher les instance_geometry
    daeTArray< daeSmartRef<domInstance_geometry> > array = node->getChildrenByType<domInstance_geometry>();
    
    //printf("Found %i instance\n", array.getCount());
    
    for (int n=0; n<array.getCount(); n++){
        //std::string s = array[n]->getAttribute("url");
        //printf(" => %s\n", s.c_str());
        
        domGeometry *geometry = daeSafeCast< domGeometry >( array[n]->getUrl().getElement() );
        
        dv->add(geometry);
    }

    daeTArray< daeSmartRef<daeElement> > nodes = node->getChildren();
    
    //printf("Found %i nodes\n", nodes.getCount());
    
    // Parcours des nodes récursivement
    for (int n=0; n<nodes.getCount(); n++){
        addToVector(dv, nodes[n]);
    }
    
    
    //domGeometry *geometry = daeSafeCast< domGeometry >( instance_geometry->getUrl().getElement() );
        
}


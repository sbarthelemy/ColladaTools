#include <math.h>
#include "daeFilter.h"

#include <fstream>
#include <iostream>
//#include <cstdlib>
#include <ctime>
#include <string>
#include "avreaderh5.h"


const double POS_FACTOR = 0.1;

using namespace av;



#define ABS(a)  (((a) < 0) ? -(a) : (a))
#define EPSILON 0.00001

void matrix2euler(Matrix mat, double *ax, double *ay, double *az)
{
    if ((ABS(mat[0]) < EPSILON) && (ABS(mat[1]) < EPSILON)){
        *az = 0.0;
        *ay = atan2(-mat[0 + 2*4], mat[0]);
        *ax = atan2(-mat[2 + 1*4], mat[1 + 1*4]);
    } else {
        *az = atan2(mat[0 + 1*4], mat[0]);
        double sz = sin(*az);
        double cz = cos(*az);
        *ay = atan2(-mat[0 + 2*4], cz*mat[0] + sz*mat[0 + 1*4]);
        *ax = atan2(sz*mat[2] - cz*mat[2 + 1*4], cz*mat[1 + 1*4] - sz*mat[1]);
    }
    *ax = *ax *57.2957795f;
    *ay = *ay *57.2957795f;
    *az = *az *57.2957795f;
}

class SourceExporter
{
    private :
        char *mName;
        int mCount, mStride;
        domAnimation *mRoot;
        domSource *mSource;
        domFloat_array *mFA;
        domName_array *mNA;
        domTechnique *mTech;
        domAccessor *mAccess;

    public :
        SourceExporter(domAnimation *root, int stride, int isName, const char *name, const char *type);
        ~SourceExporter();
        void add(float val);
        void add(float val0, float val1, float val2);
        void add(const char *val);
        void add(const char *val0, const char *val1, const char *val2);
        void addAccessorParam(const char *name, const char *type);
};




SourceExporter::SourceExporter(domAnimation *root, int stride, int isName, const char *name, const char *type)
{
    char tmp[1024];
    char tmp2[1024];
    char buffer[32];
    sprintf(tmp, "%s-%s", name, type);
    mName = strdup(tmp);
    mRoot = root;
    mStride = stride;

    mSource = (domSource*)mRoot->add("source");
    mSource->setId(tmp);
    strcat(tmp, "-array");

    if (isName){
        mNA = (domName_array*)mSource->add("Name_array");
        mNA->setId(tmp);
    } else {
        mFA = (domFloat_array*)mSource->add("float_array");
        mFA->setId(tmp);
    }

    mTech = (domTechnique*)mSource->add("technique_common");
    mAccess = (domAccessor*)mTech->add("accessor");

    sprintf(tmp2, "#%s", tmp);
    mAccess->setSource(tmp2);
    mAccess->setStride(mStride);

    mCount = 0;
}

SourceExporter::~SourceExporter()
{
    free(mName);
}

void SourceExporter::add(float val)
{
    mFA->getValue().append(val);
    mCount++;
    mFA->setCount(mCount);
    mAccess->setCount(mCount/mStride);
}

void SourceExporter::add(float val0, float val1, float val2)
{
    mFA->getValue().append3(val0, val1, val2);
    mCount+=3;
    mFA->setCount(mCount);
    mAccess->setCount(mCount/mStride);
}


void SourceExporter::add(const char *val)
{
    mNA->getValue().append(val);
    mCount++;
    mNA->setCount(mCount);
    mAccess->setCount(mCount/mStride);
}

void SourceExporter::add(const char *val0, const char *val1, const char *val2)
{
    mNA->getValue().append3(val0, val1, val2);
    mCount += 3;
    mNA->setCount(mCount);
    mAccess->setCount(mCount/mStride);
}


void SourceExporter::addAccessorParam(const char *name, const char *type)
{
    domParam *param = (domParam*)mAccess->add("param");
    param->setName(name);
    param->setType(type);
}


class AnimationExporter
{
    private :
        char *mName;
        domAnimation *mAnim;
        domLibrary_animations *mRoot;
        SourceExporter *mInput, *mInterpolation, *mOutput;
        domSampler *mSampler;

        void addInput(const char *semantic, const char *source);

    public :
        AnimationExporter(domLibrary_animations *root, int stride, const char *name, const char *type);
        ~AnimationExporter();
        void add(float timestamp, float val);
        void add(float timestamp, float val0, float val1, float val2);
};


AnimationExporter::AnimationExporter(domLibrary_animations *root, int stride, const char *name, const char *type)
{
    char tmp[1024];
    char buffer[1024];
    char target[1024];
    sprintf(tmp, "%s-%s", name, type);
    mName = strdup(tmp);
    mRoot = root;

    mAnim = (domAnimation*)mRoot->add("animation");
    mAnim->setId(mName);

    mInput = new SourceExporter(mAnim, 1, 0, tmp, "input");
    mInterpolation = new SourceExporter(mAnim, stride, 1, tmp, "interpolation");
    mOutput = new SourceExporter(mAnim, stride, 0, tmp, "output");

    // Ajouter les parametres...
    mInput->addAccessorParam("TIME", "float");

    if (stride == 3){
        const char *letters[3] = {"X", "Y", "Z"};

        for (int i=0; i<3; i++){
            mInterpolation->addAccessorParam(letters[i], "Name");
            mOutput->addAccessorParam(letters[i], "float");
        }

        // Target ?
        sprintf(target, "%s/%s", name, type);
    } else {
        mInterpolation->addAccessorParam("ANGLE", "Name");
        mOutput->addAccessorParam("ANGLE", "float");

        // Target ?
        sprintf(target, "%s/%s.ANGLE", name, type);
    }


    sprintf(buffer, "%s-sampler", tmp);
    mSampler = (domSampler*)mAnim->add("sampler");
    mSampler->setId(buffer);

    addInput("INPUT", "input");
    addInput("OUTPUT", "output");
    addInput("INTERPOLATION", "interpolation");

    domChannel *chan = (domChannel*)mAnim->add("channel");
    sprintf(buffer, "#%s-sampler", tmp);
    chan->setSource(buffer);
    chan->setTarget(target);
}


AnimationExporter::~AnimationExporter()
{
    free(mName);
}

void AnimationExporter::addInput(const char *semantic, const char *source)
{
    char buffer[1024];
    domInputLocal *i = (domInputLocal*)mSampler->add("input");

    sprintf(buffer, "#%s-%s", mName, source);
    i->setSemantic(semantic);
    i->setSource(buffer);
}


void AnimationExporter::add(float timestamp, float val)
{
    mInput->add(timestamp);
    mInterpolation->add("STEP");
    mOutput->add(val);
}

void AnimationExporter::add(float timestamp, float val0, float val1, float val2)
{
    mInput->add(timestamp);
    mInterpolation->add("STEP", "STEP", "STEP");
    mOutput->add(val0, val1, val2);
}


int main(int argc, char **argv )
{

    if (argc < 2)
    {
        exit(1);
    }

    char *file;     // Input files
    char *file_out; // Output filenames
    std::vector<const char *> elements;

    //std::basic_string<char> element;

    file = argv[3];

    DAE *dae = new DAE;
    //daeElement *root = (daeElement*)dae->open(file);

    domCOLLADA *root = (domCOLLADA*)dae->open(file);

    H5RandomReader myReader(argv[1], argv[2]);

    FrameData fd = myReader.getFrame(0);
    std::map <std::string, Matrix >::iterator it;
    for (it = fd.matrices.begin(); it != fd.matrices.end(); ++it){
            elements.push_back((it->first).c_str());
    }
    printf("Found %i elements... \n", elements.size() );

    domLibrary_animations *lib_anim;

    if (elements.size()!=0) {
        lib_anim = (domLibrary_animations*)root->add("library_animations");
    }

    //printf("Found %i steps...\n", (int)myReader.getNbSteps());

    for (int j=0; j<elements.size(); j++){
        double angle[3];
        AnimationExporter *rot[3];
        AnimationExporter *trans = new AnimationExporter(lib_anim, 3, elements[j], "translate");
        rot[2] = new AnimationExporter(lib_anim, 1, elements[j], "rotateZ");
        rot[1] = new AnimationExporter(lib_anim, 1, elements[j], "rotateY");
        rot[0] = new AnimationExporter(lib_anim, 1, elements[j], "rotateX");

        for (int i=0; i<myReader.getNbSteps(); i++){
            //printf("Step %i\n", i);
            fd = myReader.getFrame(i);

            std::map <std::string, Matrix >::iterator it;
            for (it = fd.matrices.begin(); it != fd.matrices.end(); ++it){
                if (elements[j] == (it->first).c_str()) {
                    //std::cout << "Key : " << it->first << std::endl;
                    matrix2euler(it->second, &angle[0], &angle[1], &angle[2]);
                    //td::cout << ax << "\t" << ay << "\t" << az << std::endl;

                    trans->add(myReader.getTime(i), it->second[3 + 0*4], it->second[3 + 1*4], it->second[3 + 2*4]);
                    for (int k=0; k<3; k++){
                        rot[k]->add(myReader.getTime(i), angle[k]);
                    }
                }
            }
        }
    }

    dae->add("test-new.dae");
    dae->setRoot("test-new.dae", (domCOLLADA*)root);
    dae->write("test-new.dae");

    return 0;
}

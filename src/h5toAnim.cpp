#include <math.h>
#include "daeFilter.h"

#include <fstream>
#include <iostream>
#include <ctime>
#include <string>
#include "avreaderh5.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace av;

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
        void add(float val0, float val1, float val2, float val3,
                 float val4, float val5, float val6, float val7,
                 float val8, float val9, float val10, float val11,
                 float val12, float val13, float val14, float val15);
        void add(const char *val);
        void add(const char *val0, const char *val1, const char *val2);
        void add(const char *val0, const char *val1, const char *val2, const char *val3,
                 const char *val4, const char *val5, const char *val6, const char *val7,
                 const char *val8, const char *val9, const char *val10, const char *val11,
                 const char *val12, const char *val13, const char *val14, const char *val15);
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

void SourceExporter::add(float val0, float val1, float val2, float val3,
                         float val4, float val5, float val6, float val7,
                         float val8, float val9, float val10, float val11,
                         float val12, float val13, float val14, float val15)
{
    mFA->getValue().append4(val0, val1, val2, val3);
    mFA->getValue().append4(val4, val5, val6, val7);
    mFA->getValue().append4(val8, val9, val10, val11);
    mFA->getValue().append4(val12, val13, val14, val15);
    mCount+=16;
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

void SourceExporter::add(const char *val0, const char *val1, const char *val2, const char *val3,
                         const char *val4, const char *val5, const char *val6, const char *val7,
                         const char *val8, const char *val9, const char *val10, const char *val11,
                         const char *val12, const char *val13, const char *val14, const char *val15)
{
	mNA->getValue().append4(val0, val1, val2, val3);
    mNA->getValue().append4(val4, val5, val6, val7);
    mNA->getValue().append4(val8, val9, val10, val11);
    mNA->getValue().append4(val12, val13, val14, val15);
    mCount += 16;
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
        void add(float timestamp,
                 float val0, float val1, float val2, float val3,
                 float val4, float val5, float val6, float val7,
                 float val8, float val9, float val10, float val11,
                 float val12, float val13, float val14, float val15);
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
    mInterpolation = new SourceExporter(mAnim, 1, 1, tmp, "interpolation");
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
    } else if (stride == 1) {
        mInterpolation->addAccessorParam("INTERPOLATION", "Name");
        mOutput->addAccessorParam("ANGLE", "float");

        // Target ?
        sprintf(target, "%s/%s.ANGLE", name, type);
    } else if (stride == 16) {
        mInterpolation->addAccessorParam("INTERPOLATION", "Name");
        mOutput->addAccessorParam("TRANSFORM", "float4x4");

        // Target ?
        sprintf(target, "%s/%s", name, type);
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

void AnimationExporter::add(float timestamp,
                            float val0, float val1, float val2, float val3,
                            float val4, float val5, float val6, float val7,
                            float val8, float val9, float val10, float val11,
                            float val12, float val13, float val14, float val15)
{
    mInput->add(timestamp);
/*    mInterpolation->add("STEP", "STEP", "STEP", "STEP",
                        "STEP", "STEP", "STEP", "STEP",
                        "STEP", "STEP", "STEP", "STEP",
                        "STEP", "STEP", "STEP", "STEP");
*/
    mInterpolation->add("STEP");
    mOutput->add(val0, val1, val2, val3,
                 val4, val5, val6, val7,
                 val8, val9, val10, val11,
                 val12, val13, val14, val15);
}


int main(int argc, char **argv )
{
    // describe command-line options
    std::string scene_file; // input collada scene filename
    std::string hdf5_file; // input hdf5 filename
    std::string hdf5_group;
    std::string file_out; // output collada animation filename
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("hdf5-file", po::value<std::string>(&hdf5_file),
         "HDF5 input data file")
        ("hdf5-group", po::value<std::string>(&hdf5_group)->default_value("/"),
         "subgroup within the HDF5 file")
        ("scene-file", po::value<std::string>(&scene_file),
         "collada input scene file")
        ("output-file,o",
         po::value<std::string>(&file_out)->default_value("out.dae"),
         "output file")
        ;
    po::positional_options_description p;
    p.add("hdf5-file", 1);
    p.add("scene-file", 1);
    p.add("output-file", 1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
            options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    std::vector<std::string> elements;
    DAE *dae = new DAE;
    //daeElement *root = (daeElement*)dae->open(scene_file);

    domCOLLADA *root = (domCOLLADA*)dae->open(scene_file);

    H5RandomReader myReader(hdf5_file, hdf5_group);

    FrameData fd = myReader.getFrame(0);
    std::map <std::string, Matrix >::iterator it;
    for (it = fd.matrices.begin(); it != fd.matrices.end(); ++it){
        elements.push_back(std::string(it->first));
    }
    //printf("Found %i elements... \n", elements.size() );

    domLibrary_animations *lib_anim;

    if (elements.size()!=0) {
        lib_anim = (domLibrary_animations*)root->add("library_animations");
    }

    //printf("Found %i steps...\n", (int)myReader.getNbSteps());

    for (int j=0; j<elements.size(); j++){
        AnimationExporter *matrix = new AnimationExporter(lib_anim, 16, elements[j].c_str(), "matrix");

        for (int i=0; i<myReader.getNbSteps(); i++){
            //printf("Step %i\n", i);
            fd = myReader.getFrame(i);

            std::map <std::string, Matrix >::iterator it;
            for (it = fd.matrices.begin(); it != fd.matrices.end(); ++it){
                if (elements[j] == (it->first).c_str()) {
                    //std::cout << "Key : " << it->first << std::endl;
                    //td::cout << ax << "\t" << ay << "\t" << az << std::endl;
                    matrix->add(myReader.getTime(i),
                        it->second[0 + 0*4], it->second[1 + 0*4], it->second[2 + 0*4], it->second[3 + 0*4],
                        it->second[0 + 1*4], it->second[1 + 1*4], it->second[2 + 1*4], it->second[3 + 1*4],
                        it->second[0 + 2*4], it->second[1 + 2*4], it->second[2 + 2*4], it->second[3 + 2*4],
                        it->second[0 + 3*4], it->second[1 + 3*4], it->second[2 + 3*4], it->second[3 + 3*4]);
                }
            }
        }
    }

    dae->add(file_out);
    dae->setRoot(file_out, (domCOLLADA*)root);
    dae->write(file_out);

    return 0;
}

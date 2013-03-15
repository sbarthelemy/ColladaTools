#include <math.h>
#include "daeFilter.h"

#include <fstream>
#include <iostream>
#include <ctime>
#include <string>
#include <avreader/avreader.h>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>

namespace po = boost::program_options;
using namespace av;

class SourceExporter
{
private :
  char *name_;
  int count_, stride_;
  domAnimation *root_;
  domSource *source_;
  domFloat_array *float_array_;
  domName_array *name_array_;
  domTechnique *technique_;
  domAccessor *accessor_;

public :
  SourceExporter(domAnimation *root, int stride, int isName, const char *name, const char *type);
  ~SourceExporter();
  void Add(float val);
  void Add(float val0, float val1, float val2);
  void Add(float val0, float val1, float val2, float val3,
           float val4, float val5, float val6, float val7,
           float val8, float val9, float val10, float val11,
           float val12, float val13, float val14, float val15);
  void Add(const char *val);
  void Add(const char *val0, const char *val1, const char *val2);
  void Add(const char *val0, const char *val1, const char *val2, const char *val3,
           const char *val4, const char *val5, const char *val6, const char *val7,
           const char *val8, const char *val9, const char *val10, const char *val11,
           const char *val12, const char *val13, const char *val14, const char *val15);
  void AddAccessorParam(const char *name, const char *type);
};

SourceExporter::SourceExporter(domAnimation *root, int stride, int isName, const char *name, const char *type)
{
  char tmp[1024];
  char tmp2[1024];
  char buffer[32];
  sprintf(tmp, "%s-%s", name, type);
  name_ = strdup(tmp);
  root_ = root;
  stride_ = stride;

  source_ = (domSource*)root_->add("source");
  source_->setId(tmp);
  strcat(tmp, "-array");

  if (isName) {
    name_array_ = (domName_array*)source_->add("Name_array");
    name_array_->setId(tmp);
  } else {
    float_array_ = (domFloat_array*)source_->add("float_array");
    float_array_->setId(tmp);
  }

  technique_ = (domTechnique*)source_->add("technique_common");
  accessor_ = (domAccessor*)technique_->add("accessor");

  sprintf(tmp2, "#%s", tmp);
  accessor_->setSource(tmp2);
  accessor_->setStride(stride_);

  count_ = 0;
}

SourceExporter::~SourceExporter()
{
  free(name_);
}

void SourceExporter::Add(float val)
{
  float_array_->getValue().append(val);
  count_++;
  float_array_->setCount(count_);
  accessor_->setCount(count_/stride_);
}

void SourceExporter::Add(float val0, float val1, float val2)
{
  float_array_->getValue().append3(val0, val1, val2);
  count_+=3;
  float_array_->setCount(count_);
  accessor_->setCount(count_/stride_);
}

void SourceExporter::Add(float val0, float val1, float val2, float val3,
                         float val4, float val5, float val6, float val7,
                         float val8, float val9, float val10, float val11,
                         float val12, float val13, float val14, float val15)
{
  float_array_->getValue().append4(val0, val1, val2, val3);
  float_array_->getValue().append4(val4, val5, val6, val7);
  float_array_->getValue().append4(val8, val9, val10, val11);
  float_array_->getValue().append4(val12, val13, val14, val15);
  count_+=16;
  float_array_->setCount(count_);
  accessor_->setCount(count_/stride_);
}

void SourceExporter::Add(const char *val)
{
  name_array_->getValue().append(val);
  count_++;
  name_array_->setCount(count_);
  accessor_->setCount(count_/stride_);
}

void SourceExporter::Add(const char *val0, const char *val1, const char *val2)
{
  name_array_->getValue().append3(val0, val1, val2);
  count_ += 3;
  name_array_->setCount(count_);
  accessor_->setCount(count_/stride_);
}

void SourceExporter::Add(const char *val0, const char *val1, const char *val2, const char *val3,
                         const char *val4, const char *val5, const char *val6, const char *val7,
                         const char *val8, const char *val9, const char *val10, const char *val11,
                         const char *val12, const char *val13, const char *val14, const char *val15)
{
  name_array_->getValue().append4(val0, val1, val2, val3);
  name_array_->getValue().append4(val4, val5, val6, val7);
  name_array_->getValue().append4(val8, val9, val10, val11);
  name_array_->getValue().append4(val12, val13, val14, val15);
  count_ += 16;
  name_array_->setCount(count_);
  accessor_->setCount(count_/stride_);
}


void SourceExporter::AddAccessorParam(const char *name, const char *type)
{
  domParam *param = (domParam*)accessor_->add("param");
  param->setName(name);
  param->setType(type);
}


class AnimationExporter
{
private :
  char *name_;
  domAnimation *anim_;
  domLibrary_animations *root_;
  SourceExporter *input_, *interpolation_, *output_;
  domSampler *sampler_;

  void AddInput(const char *semantic, const char *source);

public :
  AnimationExporter(domLibrary_animations *root, int stride, const char *name, const char *type);
  ~AnimationExporter();
  void Add(float timestamp, float val);
  void Add(float timestamp, float val0, float val1, float val2);
  void Add(float timestamp,
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
  name_ = strdup(tmp);
  root_ = root;

  anim_ = (domAnimation*)root_->add("animation");
  anim_->setId(name_);

  input_ = new SourceExporter(anim_, 1, 0, tmp, "input");
  interpolation_ = new SourceExporter(anim_, 1, 1, tmp, "interpolation");
  output_ = new SourceExporter(anim_, stride, 0, tmp, "output");

  // Ajouter les parametres...
  input_->AddAccessorParam("TIME", "float");

  if (stride == 3){
    const char *letters[3] = {"X", "Y", "Z"};
    for (int i=0; i<3; i++){
        interpolation_->AddAccessorParam(letters[i], "Name");
        output_->AddAccessorParam(letters[i], "float");
    }
    // Target ?
    sprintf(target, "%s/%s", name, type);
  } else if (stride == 1) {
    interpolation_->AddAccessorParam("INTERPOLATION", "Name");
    output_->AddAccessorParam("ANGLE", "float");

    // Target ?
    sprintf(target, "%s/%s.ANGLE", name, type);
  } else if (stride == 16) {
    interpolation_->AddAccessorParam("INTERPOLATION", "Name");
    output_->AddAccessorParam("TRANSFORM", "float4x4");

    // Target ?
    sprintf(target, "%s/%s", name, type);
  }


  sprintf(buffer, "%s-sampler", tmp);
  sampler_ = (domSampler*)anim_->add("sampler");
  sampler_->setId(buffer);

  AddInput("INPUT", "input");
  AddInput("OUTPUT", "output");
  AddInput("INTERPOLATION", "interpolation");

  domChannel *chan = (domChannel*)anim_->add("channel");
  sprintf(buffer, "#%s-sampler", tmp);
  chan->setSource(buffer);
  chan->setTarget(target);
}

AnimationExporter::~AnimationExporter()
{
  free(name_);
}

void AnimationExporter::AddInput(const char *semantic, const char *source)
{
  char buffer[1024];
  domInputLocal *i = (domInputLocal*)sampler_->add("input");

  sprintf(buffer, "#%s-%s", name_, source);
  i->setSemantic(semantic);
  i->setSource(buffer);
}


void AnimationExporter::Add(float timestamp, float val)
{
  input_->Add(timestamp);
  interpolation_->Add("STEP");
  output_->Add(val);
}

void AnimationExporter::Add(float timestamp, float val0, float val1, float val2)
{
  input_->Add(timestamp);
  interpolation_->Add("STEP", "STEP", "STEP");
  output_->Add(val0, val1, val2);
}

void AnimationExporter::Add(float timestamp,
                            float val0, float val1, float val2, float val3,
                            float val4, float val5, float val6, float val7,
                            float val8, float val9, float val10, float val11,
                            float val12, float val13, float val14, float val15)
{
  input_->Add(timestamp);
/*  mInterpolation->add("STEP", "STEP", "STEP", "STEP",
                        "STEP", "STEP", "STEP", "STEP",
                        "STEP", "STEP", "STEP", "STEP",
                        "STEP", "STEP", "STEP", "STEP");
*/
  interpolation_->Add("STEP");
  output_->Add(val0, val1, val2, val3,
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
  po::options_description desc(
      "Usage: h5toanim [options] hdf5-file scene-file output-file\n\n"
      "Allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("hdf5-file", po::value<std::string>(&hdf5_file)->required(),
       "HDF5 input data file")
      ("hdf5-group", po::value<std::string>(&hdf5_group)->default_value("/"),
       "subgroup within the HDF5 file")
      ("scene-file", po::value<std::string>(&scene_file)->required(),
       "collada input scene file")
      ("output-file,o",
       po::value<std::string>(&file_out)->default_value("out.dae"),
       "collada output animation file")
      ;
  po::positional_options_description pos;
  pos.add("hdf5-file", 1);
  pos.add("scene-file", 1);
  pos.add("output-file", 1);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos).run(), vm);
    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }
    po::notify(vm);
  } catch (boost::program_options::error) {
    std::cout << desc << "\n";
    return 1;
  }

  std::vector<std::string> elements;
  DAE *dae = new DAE;
  domCOLLADA *root = dae->open(scene_file);

  boost::scoped_ptr<RandomReader> myReader(MakeRandomReader(hdf5_file,
                                                            hdf5_group));

  FrameData ffd = myReader->getFrame(0);
  if (!ffd.tranform_matrices.empty() ||
      !ffd.rigid_tranform_matrices.empty() ||
      !ffd.translations.empty()) {
    domLibrary_animations *lib_anim =
        (domLibrary_animations*)root->add("library_animations");
    for (std::map <std::string, TransformMatrix>::iterator it
             = ffd.tranform_matrices.begin();
         it != ffd.tranform_matrices.end();
         ++it) {
      AnimationExporter exporter(lib_anim, 16, it->first.c_str(), "matrix");
      for (int i=0; i<myReader->getNbSteps(); ++i) {
        // Note: this is very inefficient: we copy the whole map but use a
        // single element
        FrameData fd = myReader->getFrame(i);
        TransformMatrix& data = fd.tranform_matrices[it->first];
        exporter.Add(myReader->getTime(i),
            data[0 + 0*4], data[1 + 0*4], data[2 + 0*4], data[3 + 0*4],
            data[0 + 1*4], data[1 + 1*4], data[2 + 1*4], data[3 + 1*4],
            data[0 + 2*4], data[1 + 2*4], data[2 + 2*4], data[3 + 2*4],
            data[0 + 3*4], data[1 + 3*4], data[2 + 3*4], data[3 + 3*4]);
      }
    }
  }
  dae->add(file_out);
  dae->setRoot(file_out, (domCOLLADA*)root);
  dae->write(file_out);
  return 0;
}

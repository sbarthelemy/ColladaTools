#include "avreaderh5.h"
#include <H5Cpp.h>
#include <vector>
#include <iostream>


namespace av {

herr_t IterInGroup(hid_t lid, const char *name, const H5L_info_t *linfo,
                   void *vectorOfObject) {
  std::vector<std::string> *VObj = (std::vector<std::string> *) vectorOfObject;
  VObj->push_back( name );
  return 0;
}

H5RandomReader::H5RandomReader(const std::string& file_name,
                               const std::string& group_path)
    throw (InvalidFileException):
    n_steps_(0),
    timedata_(NULL) {

  try {
    file_.openFile(file_name, H5F_ACC_RDONLY);}
  catch ( H5::FileIException ) {
    throw InvalidFileException("Cannot access file");}
  try {
    root_ = file_.openGroup(group_path);}
  catch ( H5::GroupIException ) {
    file_.close();
    throw InvalidFileException("Cannot access group");
  }

  // extract timeline. This is also necessary to get the nbSteps.
  try {
    timeline_ = root_.openDataSet("timeline");
    n_steps_ = timeline_.getSpace().getSimpleExtentNpoints();

    // Recuperer les donnees de la timeline
    timedata_ = new double[n_steps_];

    H5::DataSpace dSpace = timeline_.getSpace();

    for (int i=0; i<n_steps_; ++i){
      hsize_t count[1] = {1};
      hsize_t offset[1] = {i};
      dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
      hsize_t dims[1] = {1};
      H5::DataSpace memSpace(1, dims);
      timeline_.read(&timedata_[i], H5::PredType::NATIVE_DOUBLE, memSpace,
                     dSpace);
    }
  }
  catch (H5::DataSetIException) {
    root_.close();
    file_.close();
    throw InvalidFileException("Cannot access timeline dataset");
  }

  // extract objects names in the xpGroup

  try {
    transforms_ = root_.openGroup("transforms");
  }
  catch (H5::GroupIException) {
    file_.close();
    throw InvalidFileException("Cannot access transformsGroup");
  }

  // add exc
  std::vector<std::string>  names;
  H5Literate(transforms_.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, IterInGroup, &names);
  /*
   * extract data from object in xpGroup
   * these data can be of 3 types: matrix, translate or wrench
   * each data are saved in related map
   */
  for (unsigned int i=0; i<names.size(); i++){ //TODO: skip timeline
    H5::DataSet dSet = transforms_.openDataSet(names[i]);
    H5::DataSpace dSpace = dSet.getSpace();
    bool dimension_ok = false;
    if (dSpace.getSimpleExtentNdims()==3) {
      hsize_t dims[3];
      dSpace.getSimpleExtentDims (dims);
      if (dims[0] == n_steps_ && dims[1] == 4 && dims[2] == 4)
        dimension_ok = true;
    }
    if (dimension_ok) {
      transform_matrices_[names[i]] = dSet;
    } else {
      if (logging::warning){
        std::cerr << "Skipping dataset \"" << names[i] << "\" which has wrong dimensions. I was expecting (" << n_steps_ << ",4,4).\n";
      }
      dSet.close();
    }
  }
}

H5RandomReader::~H5RandomReader() {
  std::map< std::string, H5::DataSet > ::iterator im;
  for (im = transform_matrices_.begin(); im != transform_matrices_.end(); im++){
    im->second.close();
  }
  for (im = translations_.begin(); im != translations_.end(); im++){
    im->second.close();
  }
  for (im = wrenches_.begin(); im != wrenches_.end(); im++){
    im->second.close();
  }
  timeline_.close();
  root_.close();
  file_.close();

  delete[](timedata_);
}

FrameData H5RandomReader::getFrame(unsigned long step) const {
  FrameData fData;
  for (std::map<std::string, H5::DataSet>::const_iterator it =
           transform_matrices_.begin();
       it != transform_matrices_.end();
       ++it) {
    H5::DataSet dSet(it->second);
    double buff[16];
    H5::DataSpace dSpace(dSet.getSpace());
    hsize_t count[3] = {1, 4, 4};
    hsize_t offset[3] = {step, 0, 0};
    dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
    hsize_t dims[2] = {4, 4};
    H5::DataSpace memSpace(2, dims);
    dSet.read(buff, H5::PredType::NATIVE_DOUBLE, memSpace, dSpace);
    fData.tranform_matrices[it->first] = TransformMatrix(buff);
  }
  for (std::map<std::string, H5::DataSet>::const_iterator it =
           translations_.begin();
       it != translations_.end();
       ++it) {
    H5::DataSet dSet(it->second);
    double buff[3];
    H5::DataSpace dSpace(dSet.getSpace());
    hsize_t count[2] = {1, 3};
    hsize_t offset[2] = {step, 0};
    dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
    hsize_t dims[1] = {3};
    H5::DataSpace memSpace(1, dims);
    dSet.read(buff, H5::PredType::NATIVE_DOUBLE, memSpace, dSpace);
    fData.translations[it->first] = Translation(buff);
  }
  for (std::map<std::string, H5::DataSet>::const_iterator it =
           wrenches_.begin();
       it != wrenches_.end();
       ++it) {
    H5::DataSet dSet(it->second);
    double buff[6];
    H5::DataSpace dSpace(dSet.getSpace());
    hsize_t count[2] = {1, 6};
    hsize_t offset[2] = {step, 0};
    dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
    hsize_t dims[1] = {6};
    H5::DataSpace memSpace(1, dims);
    dSet.read(buff, H5::PredType::NATIVE_DOUBLE, memSpace, dSpace);
    fData.wrenches[it->first] = Wrench(buff);
  }
  return fData;
}

H5LastReader::H5LastReader(const std::string& file_name,
                           const std::string& group_path) :
    last_step_(0),
    reader_(file_name, group_path) {}

FrameData H5LastReader::getLatestFrame() {
  FrameData fData = reader_.getFrame(last_step_);
  ++last_step_;
  if (last_step_==reader_.getNbSteps()) {
    last_step_ = 0;
  }
  return fData;
}

RandomReader* MakeRandomReader(const std::string& file_name,
                               const std::string& group_path)
    throw (InvalidFileException) {
  return new H5RandomReader(file_name, group_path);
}

}

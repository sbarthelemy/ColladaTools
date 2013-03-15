#include "avreaderh5.h"
#include <H5Cpp.h>
#include <vector>
#include <iostream>


namespace av {

herr_t iterInGroup(hid_t lid, const char *name, const H5L_info_t *linfo, void *vectorOfObject) {
std::vector< std::string > *VObj =  (std::vector< std::string > *) vectorOfObject;
VObj->push_back( name );
return 0;
}

H5RandomReader::H5RandomReader(const std::string& fileName,
                               const std::string& groupPath) throw (InvalidFileException):
    nSteps(0),
    timedata(NULL) {

  try {
    file.openFile(fileName, H5F_ACC_RDONLY);}
  catch ( H5::FileIException ) {
    throw InvalidFileException("Cannot access file");}
  try {
    group = file.openGroup(groupPath);}
  catch ( H5::GroupIException ) {
    file.close();
    throw InvalidFileException("Cannot access group");
  }
  /*
   * extract timeline. This is also necessary to get the nbSteps.
   */

  try {
    timeline = group.openDataSet("timeline");
    nSteps = timeline.getSpace().getSimpleExtentNpoints();

    // Recuperer les donnees de la timeline
    timedata = new double[nSteps];

    H5::DataSpace dSpace = timeline.getSpace();

    for (int i=0; i<nSteps; ++i){
      hsize_t count[1] = {1};
      hsize_t offset[1] = {i};
      dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
      hsize_t dims[1] = {1};
      H5::DataSpace memSpace(1, dims);
      timeline.read(&timedata[i], H5::PredType::NATIVE_DOUBLE, memSpace, dSpace);
    }
  }
  catch ( H5::DataSetIException error ) {
    //error.printError();
    group.close();
    file.close();
    throw InvalidFileException("Cannot access timeline dataset");
  }


  if (logging::info)
    std::cerr << "Opened group \"" <<  fileName << groupPath << "\" which has " << nSteps << " steps.\n";

  /*
   * extract objects names in the xpGroup
   */
  printf("Transform time\n");

  try {
    transformsGroup = group.openGroup("transforms");
  }
  catch (H5::GroupIException) {
    file.close();
    throw InvalidFileException("Cannot access transformsGroup");
  }


  printf("Ok\n");

  // add exc
  std::vector<std::string>  names;
  H5Literate(transformsGroup.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, iterInGroup, &names);
  /*
   * extract data from object in xpGroup
   * these data can be of 3 types: matrix, translate or wrench
   * each data are saved in related map
   */
  for (unsigned int i=0; i<names.size(); i++){ //TODO: skip timeline
    H5::DataSet dSet = transformsGroup.openDataSet(names[i]);
    H5::DataSpace dSpace = dSet.getSpace();
    bool dimension_ok = false;
    if (dSpace.getSimpleExtentNdims()==3) {
      hsize_t dims[3];
      dSpace.getSimpleExtentDims (dims);
      if (dims[0] == nSteps && dims[1] == 4 && dims[2] == 4)
        dimension_ok = true;
    }
    if (dimension_ok) {
      matrices[names[i]] = dSet;
    } else {
      if (logging::warning){
        std::cerr << "Skipping dataset \"" << names[i] << "\" which has wrong dimensions. I was expecting (" << nSteps << ",4,4).\n";
      }
      dSet.close();
    }
  }
}

H5RandomReader::~H5RandomReader() {
  std::map< std::string, H5::DataSet > ::iterator im;
  for (im = matrices.begin(); im != matrices.end(); im++){
    im->second.close();
  }
  for (im = translates.begin(); im != translates.end(); im++){
    im->second.close();
  }
  for (im = wrenches.begin(); im != wrenches.end(); im++){
    im->second.close();
  }
  timeline.close();
  group.close();
  file.close();

  delete[](timedata);
}

FrameData H5RandomReader::getFrame(unsigned long step) const {
  FrameData fData;
  for (std::map< std::string, H5::DataSet > ::const_iterator im (matrices.begin());
       im != matrices.end();
       im++) {
    H5::DataSet dSet(im->second);
    double buff[16];
    H5::DataSpace dSpace(dSet.getSpace());
    hsize_t count[3] = {1, 4, 4};
    hsize_t offset[3] = {step, 0, 0};
    dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
    hsize_t dims[2] = {4, 4};
    H5::DataSpace memSpace(2, dims);
    dSet.read(buff, H5::PredType::NATIVE_DOUBLE, memSpace, dSpace);
    fData.tranform_matrices[im->first] = TransformMatrix(buff);
  }
  for (std::map< std::string, H5::DataSet > ::const_iterator im(translates.begin());
       im != translates.end();
       im++) {
    H5::DataSet dSet(im->second);
    double buff[3];
    H5::DataSpace dSpace(dSet.getSpace());
    hsize_t count[2] = {1, 3};
    hsize_t offset[2] = {step, 0};
    dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
    hsize_t dims[1] = {3};
    H5::DataSpace memSpace(1, dims);
    dSet.read(buff, H5::PredType::NATIVE_DOUBLE, memSpace, dSpace);
    fData.translations[im->first] = Translation(buff);
  }
  for (std::map< std::string, H5::DataSet > ::const_iterator im(wrenches.begin());
       im != wrenches.end();
       im++) {
    H5::DataSet dSet(im->second);
    double buff[6];
    H5::DataSpace dSpace(dSet.getSpace());
    hsize_t count[2] = {1, 6};
    hsize_t offset[2] = {step, 0};
    dSpace.selectHyperslab(H5S_SELECT_SET, count, offset);
    hsize_t dims[1] = {6};
    H5::DataSpace memSpace(1, dims);
    dSet.read(buff, H5::PredType::NATIVE_DOUBLE, memSpace, dSpace);
    fData.wrenches[im->first] = Wrench(buff);
  }
  return fData;
}

H5LastReader::H5LastReader(const std::string fileName, const std::string groupPath) :
    lastStep(0),
    reader(fileName, groupPath) {};

FrameData H5LastReader::getLatestFrame() {
  FrameData fData = reader.getFrame(lastStep);
  lastStep++;
  if (lastStep==reader.getNbSteps()) {
    lastStep = 0;
  }
  return fData;
}

RandomReader* MakeRandomReader(const std::string& file_name,
                               const std::string& group_path)
    throw (InvalidFileException) {
  return new H5RandomReader(file_name, group_path);
}

}

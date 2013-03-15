#ifndef AVREADERH5_H
#define AVREADERH5_H

/*!
 * \file arborisViewer.h
 * \brief Lecteur de musique de base
 * \author salini - barthélemy
 */

#include <iostream>
#include <string>
#include <map>
#include <H5Cpp.h>
#include <avreader/avreader.h>

namespace av {

class H5RandomReader : public RandomReader {
public:
  H5RandomReader(const std::string& file_name, const std::string& group_path)
      throw (InvalidFileException);
  ~H5RandomReader();
  FrameData getFrame(unsigned long step) const;
  unsigned long getNbSteps() const {return nSteps;};
  double getTime(unsigned long step) {return timedata[step];}

private:
  unsigned long nSteps;
  H5::H5File file;
  H5::Group group, transformsGroup;
  H5::DataSet timeline;
  double *timedata;
  std::map<std::string, H5::DataSet >  transform_matrices;
  std::map<std::string, H5::DataSet >  translations;
  std::map<std::string, H5::DataSet >  wrenches;
};

class H5LastReader : public LastReader {
public:
  H5LastReader(const std::string fileName, const std::string GroupPath);
  FrameData getLatestFrame();
  void start() {};
  void stop() {lastStep = 0;};

private:
  unsigned long lastStep;
  H5RandomReader reader;
};
}
#endif

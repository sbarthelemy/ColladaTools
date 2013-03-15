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
  unsigned long getNbSteps() const { return n_steps_; }
  double getTime(unsigned long step) { return timedata_[step]; }

private:
  unsigned long n_steps_;
  H5::H5File file_;
  H5::Group root_;
  H5::Group transforms_;
  H5::DataSet timeline_;
  double *timedata_;
  std::map<std::string, H5::DataSet> transform_matrices_;
  std::map<std::string, H5::DataSet> translations_;
  std::map<std::string, H5::DataSet> wrenches_;
};

class H5LastReader : public LastReader {
public:
  H5LastReader(const std::string& file_name, const std::string& group_path);
  FrameData getLatestFrame();
  void start() {}
  void stop() { last_step_ = 0; }

private:
  unsigned long last_step_;
  H5RandomReader reader_;
};
}
#endif

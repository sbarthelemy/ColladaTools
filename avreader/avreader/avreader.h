#ifndef AVREADER_H
#define AVREADER_H
#include <iostream>
#include <map>
#include <stdexcept>

namespace logging {
static const bool debug = true;
static const bool info = true;
static const bool warning = true;
static const bool error = true;
}

namespace av {

class TransformMatrix {
private:
  double data_[16];
public:
  TransformMatrix();
  TransformMatrix(double v0, double v1, double v2, double v3,
                  double v4, double v5, double v6, double v7,
                  double v8, double v9, double v10, double v11,
                  double v12, double v13, double v14, double v15);
  TransformMatrix(const double val[16]);
  double operator[](std::size_t index) const;
};

std::ostream& operator<<(std::ostream& os, const TransformMatrix& m);

class RigidTransformMatrix {
private:
  double data_[12];
public:
  RigidTransformMatrix();
  RigidTransformMatrix(double v0, double v1, double v2, double v3,
                       double v4, double v5, double v6, double v7,
                       double v8, double v9, double v10, double v11);
  RigidTransformMatrix(const double val[12]);
  double operator[](std::size_t index) const;
  operator TransformMatrix() const;
};


class Translation {
private:
  double data_[3];
public:
  Translation();
  Translation(double v0, double v1, double v2);
  Translation(const double val[3]);
  double operator[](const std::size_t index) const;
  operator TransformMatrix() const;
};

std::ostream& operator<< (std::ostream& os, const Translation& t);

class Wrench {
private:
  double data_[6];
public:
  Wrench();
  Wrench(const double val[6]);
  const double& operator[] (std::size_t index);
};

std::ostream& operator<<(std::ostream& os, const Wrench& w);

class FrameData {
public:
  unsigned long step;
  std::map <std::string, TransformMatrix> tranform_matrices;
  std::map <std::string, Translation> translations;
  std::map <std::string, Wrench> wrenches;
};

std::ostream& operator<< (std::ostream& os, const FrameData& fd);

class RandomReader {
public:
  virtual ~RandomReader() {}
  virtual unsigned long getNbSteps() const = 0;
  virtual FrameData getFrame(unsigned long step) const = 0;
  virtual double getTime(unsigned long step) = 0;
};

class LastReader {
public:
  virtual ~LastReader() {}
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual FrameData getLatestFrame() = 0;
};

class InvalidFileException : public std::runtime_error {
public:
  InvalidFileException(): std::runtime_error("Invalid scene file") {}
  InvalidFileException(const std::string& msg): std::runtime_error(msg) {}
};

RandomReader* MakeRandomReader(const std::string& file_name,
                               const std::string& group_path)
    throw (InvalidFileException);
}
#endif

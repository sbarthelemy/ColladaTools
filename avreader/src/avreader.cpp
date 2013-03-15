#include <avreader/avreader.h>
#include <sstream>
#include <map>
#include <assert.h>

namespace av {
TransformMatrix::TransformMatrix() {
  for (std::size_t i=0; i<16; i++)
    data_[i] = 0.;
}

TransformMatrix::TransformMatrix(
      double v0, double v1, double v2, double v3,
      double v4, double v5, double v6, double v7,
      double v8, double v9, double v10, double v11,
      double v12, double v13, double v14, double v15) {
  data_[0] = v0; data_[1] = v1; data_[2] = v2; data_[3] = v3;
  data_[4] = v4; data_[5] = v5; data_[6] = v6; data_[7] = v7;
  data_[8] = v8; data_[9] = v9; data_[10] = v10; data_[11] = v11;
  data_[12] = v12; data_[13] = v13; data_[14] = v14; data_[15] = v15;
}

TransformMatrix::TransformMatrix (const double val[16])
{
  for (std::size_t i=0; i<16; i++)
    data_[i] = val[i];
}

double TransformMatrix::operator[](const std::size_t index) const {
  assert (0 <= index);
  assert (index < 16);
  return data_[index];
}

std::ostream& operator<<(std::ostream& os, const TransformMatrix& m) {
  return os << m[0] << " " <<m[1] << " " << m[2] << " " << m[3] << "\n"
            << m[4] << " " <<m[5] << " " << m[6] << " " << m[7] << "\n"
            << m[8] << " " <<m[9] << " " << m[10] << " " << m[11] << "\n"
            << m[12] << " " <<m[13] << " " << m[14] << " " << m[15] << "\n";
}

Translation::Translation() {
  data_[0] = data_[1] = data_[2] = 0.;
}

Translation::Translation(double v0, double v1, double v2) {
  data_[0] = v0; data_[1] = v1; data_[2] = v2;
}

Translation::Translation(const double val[3]) {
  for (int i=0; i<3; i++)
    data_[i] = val[i];
}

double Translation::operator[](const std::size_t index) const {
  assert (0 <= index);
  assert (index < 3);
  return data_[index];
}

std::ostream& operator<<(std::ostream& os, const Translation& t) {
  return os<<t[0]<<" "<<t[1]<<" "<<t[2]<<std::endl;
}

Translation::operator TransformMatrix() const {
  return TransformMatrix(1., 0., 0., data_[0],\
                         0., 1., 0., data_[1],\
                         0., 0., 1., data_[2],\
                         0., 0., 0., 1.);
}

Wrench::Wrench() {
  for (int i=0; i<6; i++)
    data_[i] = 0;
}

Wrench::Wrench(const double val[6]) {
  for (int i=0; i<6; i++)
    data_[i] = val[i];
}

const double& Wrench::operator[](const std::size_t index) {
  assert (0 <= index);
  assert (index < 6);
  return data_[index];
}

std::ostream& operator<<(std::ostream& os, const Wrench& w) {
  return os << "display wrench not impremented yet!!" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const FrameData& fd) {

  os << "===================================================\n"
     << "                 Data of Frame " << fd.step << "\n"
     << "===================================================\n";

  std::map<std::string, TransformMatrix> ::const_iterator im;
  for (im = fd.tranform_matrices.begin();
       im !=fd.tranform_matrices.end();
       ++im) {
    os << (*im).first << " (tranform matrix):\n"
       << (*im).second << "\n";
  }
  std::map<std::string, Translation>::const_iterator it;
  for (it = fd.translations.begin(); it !=fd.translations.end(); ++it) {
    os << (*it).first << " (translation):\n"
       << (*it).second << "\n";
  }
  std::map<std::string, Wrench>::const_iterator iw;
  for (iw = fd.wrenches.begin(); iw !=fd.wrenches.end(); ++iw) {
    os << (*iw).first << " (wrench):\n"
       << (*iw).second << "\n";
  }
  return os;
}
}


#include "avreader.h"
#include <sstream>
#include <map>
#include <assert.h>

namespace av {
    Matrix::Matrix() {
        for (std::size_t i=0; i<16; i++)
            data[i] = 0.;
    }

    Matrix::Matrix (double v0, double v1, double v2, double v3,
                    double v4, double v5, double v6, double v7,
                    double v8, double v9, double v10, double v11,
                    double v12, double v13, double v14, double v15) {
        data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
        data[4] = v4; data[5] = v5; data[6] = v6; data[7] = v7;
        data[8] = v8; data[9] = v9; data[10] = v10; data[11] = v11;
        data[12] = v12; data[13] = v13; data[14] = v14; data[15] = v15;
    };

    Matrix::Matrix (const double val[16])
    {
        for (std::size_t i=0; i<16; i++)
            data[i] = val[i];
    }

    double Matrix::operator[] (const std::size_t index) const {
        assert (0 <= index);
        assert (index < 16);
        return data[index];
    }

    std::ostream& operator<<(std::ostream& os, Matrix m) {
        return os<<m[0]<<" "<<m[1]<<" "<<m[2]<<" "<<m[3]<<std::endl
                 <<m[4]<<" "<<m[5]<<" "<<m[6]<<" "<<m[7]<<std::endl
                 <<m[8]<<" "<<m[9]<<" "<<m[10]<<" "<<m[11]<<std::endl
                 <<m[12]<<" "<<m[13]<<" "<<m[14]<<" "<<m[15]<<std::endl;
    }

    Translate::Translate() {
        data[0] = data[1] = data[2] = 0.;
    }

    Translate::Translate(double v0, double v1, double v2) {
        data[0] = v0; data[1] = v1; data[2] = v2;
    }

    Translate::Translate (const double val[3]) {
        for (int i=0; i<3; i++)
            data[i] = val[i];
    }

    double Translate::operator[] (const std::size_t index) const {
        assert (0 <= index);
        assert (index < 3);
        return data[index];
    }

    std::ostream& operator<<(std::ostream& os, Translate t) {
        return os<<t[0]<<" "<<t[1]<<" "<<t[2]<<std::endl;
    }

    Translate::operator Matrix () const {
        return Matrix(1., 0., 0., data[0],\
                      0., 1., 0., data[1],\
                      0., 0., 1., data[2],\
                      0., 0., 0., 1.);
    }

    Wrench::Wrench() {
        for (int i=0; i<6; i++)
            data[i] = 0;
    }

    Wrench::Wrench(const double val[6]) {
        for (int i=0; i<6; i++)
            data[i] = val[i];
    }

    const double& Wrench::operator[] (const std::size_t index) {
        assert (0 <= index);
        assert (index < 6);
        return data[index];
    }

    std::ostream& operator<<(std::ostream& os, Wrench w) {
        return os<<"display wrench not impremented yet!!"<<std::endl;
    }

    std::ostream& operator<<(std::ostream& os, FrameData fd) {

        os<<"==================================================="<<std::endl;
        os<<"                 Data of Frame "<<fd.step<<std::endl;
        os<<"==================================================="<<std::endl;

        std::map < std::string, Matrix > ::iterator im;
        for ( im = fd.matrices.begin(); im !=fd.matrices.end(); im++) {
            os<<(*im).first<<" (matrix):"<<std::endl;
            os<<(*im).second<<std::endl;
        }
        std::map < std::string, Translate > ::iterator it;
        for ( it = fd.translates.begin(); it !=fd.translates.end(); it++) {
            os<<(*it).first<<" (translate):"<<std::endl;
            os<<(*it).second<<std::endl;
        }
        std::map < std::string, Wrench > ::iterator iw;
        for ( iw = fd.wrenches.begin(); iw !=fd.wrenches.end(); iw++) {
            os<<(*iw).first<<" (wrench):"<<std::endl;
            os<<(*iw).second<<std::endl;
        }
        return os;
    }
}


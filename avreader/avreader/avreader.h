#ifndef AVREADER_H
#define AVREADER_H
#include <iostream>
#include <map>

namespace logging {
    static const bool debug = true;
    static const bool info = true;
    static const bool warning = true;
    static const bool error = true;
}

namespace av {

    class Matrix {
    private:
        double data[16];
    public:
        Matrix ();
        Matrix (double v0, double v1, double v2, double v3,
                double v4, double v5, double v6, double v7,
                double v8, double v9, double v10, double v11,
                double v12, double v13, double v14, double v15);
        Matrix (const double val[16]);
        double operator[] (const std::size_t index) const;
    };

    std::ostream& operator<< (std::ostream& os,const Matrix m);

    class Translate {
    private:
        double data[3];
    public:
        Translate ();
        Translate (double v0, double v1, double v2);
        Translate (const double val[3]);
        double operator[] (const std::size_t index) const;
        operator Matrix () const;
    };

    std::ostream& operator<< (std::ostream& os, const Translate t);

    class Wrench{
    private:
        double data[6];
    public:
        Wrench();
        Wrench(const double val[6]);
        const double& operator[] (const std::size_t index);
    };

    std::ostream& operator<<(std::ostream& os, const Wrench w);

    class FrameData{
    public:
        unsigned long step;
        std::map <std::string, Matrix >     matrices;
        std::map <std::string, Translate >  translates;
        std::map <std::string, Wrench >     wrenches;
    };

    std::ostream& operator<< (std::ostream& os, const FrameData fd);

    class RandomReader {
    public:
        virtual ~RandomReader() {};
        virtual FrameData getFrame(unsigned long step) const = 0;
    };

    class LastReader {
    public:
        virtual ~LastReader() {};
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual FrameData getLatestFrame() = 0;
    };
}
#endif

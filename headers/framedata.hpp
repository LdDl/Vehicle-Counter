#pragma once
#ifndef FRAMEDATA_HPP
#define FRAMEDATA_HPP

#include "utils.hpp"

class FrameData {
public:
    FrameData();
    FrameData(const FrameData &src);
    struct MyDataFlatMembers {
        int64 timestamp = 0;
        int64 frameNum = 0;
    };
    MyDataFlatMembers vars;
    Mat firstFrame;
    Mat image, image_truesize, depth, bgr;

    FrameData &operator=(const FrameData &src){
        if (this == &src)
            return *this;
        Copy(src);
        return *this;
    }
    ~FrameData();
private:
    void Copy(const FrameData &src);
    void Clone(const FrameData &src);
};

#endif // FRAMEDATA_HPP

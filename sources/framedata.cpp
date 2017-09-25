#include "headers/framedata.hpp"

FrameData::FrameData() {}

FrameData::FrameData(const FrameData &src) {
    Clone(src);
}

FrameData::~FrameData() {
}

void FrameData::Copy(const FrameData &src) {
    vars = src.vars;
    image = src.image;
    firstFrame = src.firstFrame;
    image_truesize = src.image_truesize;
    depth = src.depth;
    bgr = src.bgr;
}

void FrameData::Clone(const FrameData &src) {
    vars = src.vars;
    src.image.copyTo(image);
    src.firstFrame.copyTo(firstFrame);
    src.image_truesize.copyTo(image_truesize);
    src.depth.copyTo(depth);
    src.bgr.copyTo(bgr);
}

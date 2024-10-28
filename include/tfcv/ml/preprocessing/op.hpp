#pragma once
#include "tfcv/defs.hpp"

namespace cv { class Mat; }

TFCV_NAMESPACE_WITH_BEGIN(ml)

class API Op
{
public:
    Op() = default;
    virtual ~Op() = default;

    virtual void run(cv::Mat&) const = 0;
};

TFCV_NAMESPACE_END
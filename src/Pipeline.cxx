#include "tfcv/ml/preprocessing/pipeline.hpp"
#include "tfcv/ml/preprocessing/ops.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

using namespace tfcv::ml;

tfcv::Image Pipeline::run(const tfcv::Image& image) const
{
    cv::Mat result = image.underlying_handle().clone();
    for (const auto& op : _ops)
    {
        op->run(result);
    }

    return tfcv::Image { std::move(result) };
}

void Pipeline::clear()
{
    _ops.clear();
}

// ---------------------------- Ops ----------------------------

void Rescaling::run(cv::Mat& image) const
{
    image.convertTo(image, CV_32F, _scale, _offset);
}

void Resizing::run(cv::Mat& image) const
{
    cv::resize(image, image, cv::Size(_width, _height), 0, 0, _interpolation);
}

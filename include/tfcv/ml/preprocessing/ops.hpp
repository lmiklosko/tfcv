#pragma once
#include "tfcv/defs.hpp"
#include "tfcv/ml/preprocessing/op.hpp"

TFCV_NAMESPACE_WITH_BEGIN(ml)

class API Rescaling : public Op
{
    double _scale;
    double _offset;
public:
    explicit Rescaling(double scale, double offset = 0.0) : _scale(scale), _offset(offset) {}

    void run(cv::Mat& image) const override;
};

class API Resizing : public Op
{
    int _width;
    int _height;
    int _interpolation;

public:
    Resizing(int width, int height, int interpolation = 0)
        : _width(width), _height(height), _interpolation(interpolation)
    {
    }

    void run(cv::Mat& image) const override;


    enum Interpolation {
        INTER_NEAREST = 0,
        INTER_LINEAR = 1,
        INTER_CUBIC = 2,
    };
};

TFCV_NAMESPACE_END
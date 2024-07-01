#include "tfcv/image.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


/* static */ void Image::__check(const cv::Mat& mat)
{
    if (mat.empty())
    {
        throw std::invalid_argument("Image::__check(): Image is empty");
    }
}



Image::Image(cv::Mat&& image)
        : mat(std::move(image))
{
    __check(mat);

    if ((mat.type() & CV_MAT_DEPTH_MASK) != CV_8U)
    {
        mat.convertTo(mat, CV_8U, 255.0);
    }
}

Image::Image(std::span<const std::byte> data) // NOLINT(*-explicit-constructor)
        : mat(cv::imdecode(cv::Mat(1, (int)data.size(), CV_8UC1, (void*)data.data()), cv::IMREAD_UNCHANGED))
{
    __check(mat);
}

Image::Image(const std::filesystem::path& imagePath) // NOLINT(*-explicit-constructor)
        : mat(cv::imread(imagePath.string(), cv::IMREAD_UNCHANGED))
{
    __check(mat);
}

Image Image::depthwiseContour(std::initializer_list<int> ignore_channels) const
{
    static auto makeMask = [](const cv::Mat& input, cv::Mat& output) -> void
    {
        std::vector<std::vector<cv::Point>> contour;
        cv::findContours(input, contour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        auto it = std::max_element(contour.begin(), contour.end(), [](const auto& a, const auto& b) {
            return cv::contourArea(a) < cv::contourArea(b);
        });
        auto index = std::distance(contour.begin(), it);

        output = cv::Mat::zeros(input.size(), CV_8UC1);
        cv::drawContours(output, contour, (int)index, cv::Scalar(255), cv::FILLED);
    };


    if (mat.channels() == 1)
    {
        cv::Mat out;
        makeMask(mat, out);

        return { std::move(out) };
    }
    else
    {
        std::vector<cv::Mat> channels;
        cv::split(mat, channels);

        for (int i = 0; i < channels.size(); ++i)
        {
            if (ignore_channels.end() != std::find(ignore_channels.begin(), ignore_channels.end(), i))
            {
                channels[i] = cv::Mat::zeros(mat.size(), channels[i].type());
                continue;
            }

            makeMask(channels[i], channels[i]);
        }

        cv::Mat out;
        cv::merge(channels, out);

        return { std::move(out) };
    }
}

void Image::crop(std::span<const cv::Point2f, 4> points)
{
    /* We need to order points in the following way:
     * 0 1
     * 3 2
     *
     * Top left point has the smallest sum of x and y coordinates and bottom right point has the largest sum of x and y coordinates
     * Top right point has the largest difference of x and y coordinates and bottom left point has the smallest difference of x and y coordinates
     */
    auto [minIt, maxIt] = std::minmax_element(points.begin(), points.end(),
                                              [](const auto& a, const auto& b) { return (a.x + a.y) < (b.x + b.y); });

    auto [minDiffIt, maxDiffIt] = std::minmax_element(points.begin(), points.end(),
                                                      [](const auto& a, const auto& b) { return (a.x - a.y) < (b.x - b.y); });

    const cv::Point2f src[] = { *minIt, *maxDiffIt, *maxIt, *minDiffIt };

    /* Width is the maximum distance between bottom-right and bottom-left points, while
     * height being the maximum distance between top-left and bottom-left points */
    auto width = (float)std::max(
            std::sqrt(std::pow(src[2].x - src[3].x, 2) + std::pow(src[2].y - src[3].y, 2)),
            std::sqrt(std::pow(src[1].x - src[0].x, 2) + std::pow(src[1].y - src[0].y, 2)));

    auto height = (float)std::max(
            std::sqrt(std::pow(src[1].x - src[2].x, 2) + std::pow(src[1].y - src[2].y, 2)),
            std::sqrt(std::pow(src[0].x - src[3].x, 2) + std::pow(src[0].y - src[3].y, 2)));

    const cv::Point2f dst[] = { {0, 0}, {width, 0}, {width, height}, {0, height} };

    /* Warp the perspective */
    cv::Mat M = cv::getPerspectiveTransform(src, dst);
    cv::warpPerspective(mat, mat, M, { (int)width, (int)height });
}

void Image::copyTo(void*& ptr, bool normalized, bool grayscale) const
{
    // TODO: What if our image is already grayscale?

    auto assign = normalized
        ? [](void*& ptr, uint8_t pixel) { *((float*)ptr) = static_cast<float>(pixel) / 255.0f; ptr = (float*)ptr + 1; }
        : [](void*& ptr, uint8_t pixel) { *((uint8_t*)ptr) = pixel; ptr = (uint8_t*)ptr + 1; };

    for (int i = 0; i < mat.rows; ++i)
    {
        for (int j = 0; j < mat.cols; ++j)
        {
            auto pixel = mat.at<cv::Vec3b>(i, j);
            if (grayscale)
            {
                /* Grayscale - linear approximation */
                assign(ptr, static_cast<uint8_t>(
                        0.114 * (double)pixel[0] +
                        0.587 * (double)pixel[1] +
                        0.299 * (double)pixel[2]
                ));
            }
            else
            {
                /* OpenCV uses BGR format ML needs RGB */
                assign(ptr, pixel[2]);
                assign(ptr, pixel[1]);
                assign(ptr, pixel[0]);
            }
        }
    }
}

Image Image::resize(int width, int height) const
{
    cv::Mat out;
    cv::resize(mat, out, { width, height });

    return { std::move(out) };
}

std::vector<uint8_t> Image::encode(std::string_view format) const
{
    std::vector<uchar> out;
    cv::imencode(format.data(), mat, out);

    return out;
}

int Image::width() const { return mat.cols; }
int Image::height() const { return mat.rows; }
int Image::channels() const { return mat.channels(); }

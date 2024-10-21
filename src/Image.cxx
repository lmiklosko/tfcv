#include "tfcv/image.hpp"
#include "tfcv/utility.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace tfcv;

class Image::impl
{
public:
    cv::Mat mat;

    impl(cv::Mat&& image)
            : mat(std::move(image))
    {
        __check(mat);

        if ((mat.type() & CV_MAT_DEPTH_MASK) != CV_8U)
        {
            mat.convertTo(mat, CV_8U, 255.0);
        }
    }

    impl(std::span<const std::byte> data, int width, int height, int channels) // NOLINT(*-explicit-constructor)
            : mat(cv::Size(width, height), typeOf(data.size(), { width, height}, channels), (void*)data.data())
    {
        __check(mat);

        if ((mat.type() & CV_MAT_DEPTH_MASK) != CV_8U)
        {
            mat.convertTo(mat, CV_8U, 255.0);
        }
    }

    impl(std::span<const std::byte> data) // NOLINT(*-explicit-constructor)
            : mat(cv::imdecode(cv::Mat(1, (int)data.size(), CV_8UC1, (void*)data.data()), cv::IMREAD_UNCHANGED))
    {
        __check(mat);
    }

    impl(const std::filesystem::path& imagePath) // NOLINT(*-explicit-constructor)
            : mat(cv::imread(imagePath.string(), cv::IMREAD_UNCHANGED))
    {
        __check(mat);
    }

    void crop(std::span<const std::pair<float, float>, 4> points)
    {
        /* We need to order points in the following way:
         * 0 1
         * 3 2
         *
         * Top left point has the smallest sum of x and y coordinates and bottom right point has the largest sum of x and y coordinates
         * Top right point has the largest difference of x and y coordinates and bottom left point has the smallest difference of x and y coordinates
         */
        auto [minIt, maxIt] = std::minmax_element(points.begin(), points.end(),
                                                  [](const auto& a, const auto& b) { return (a.first + a.second) < (b.first + b.second); });

        auto [minDiffIt, maxDiffIt] = std::minmax_element(points.begin(), points.end(),
                                                          [](const auto& a, const auto& b) { return (a.first - a.second) < (b.first - b.second); });

        const cv::Point2f src[] = {
                { minIt->first, minIt->second },
                { maxDiffIt->first, maxDiffIt->second },
                { maxIt->first, maxIt->second },
                { minDiffIt->first, minDiffIt->second }
        };

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

    void cropToFit(int channel)
    {
        int firstRow = mat.rows, lastRow = 0;
        int firstCol = mat.cols, lastCol = 0;

        // Find the bounding box of the non-zero channel pixels
        int channels = mat.channels();
        for (int i = 0; i < mat.rows; ++i) {
            const uchar* rowPtr = mat.ptr<uchar>(i);
            for (int j = 0; j < mat.cols; ++j) {
                if (rowPtr[j * channels + channel] != 0) { // check the desired channel
                    firstRow = std::min(firstRow, i);
                    lastRow = i;
                    firstCol = std::min(firstCol, j);
                    lastCol = std::max(lastCol, j);
                }
            }
        }

        cv::Rect roi(firstCol, firstRow, lastCol - firstCol, lastRow - firstRow);
        mat = mat(roi);
    }

    void toOneHot()
    {
        auto encodeOneHot = [](auto& pixel, const int* position) {
            auto idx = utility::argmax(std::span(pixel.val));

            std::memset(pixel.val, 0, sizeof(pixel.val));
            pixel.val[idx] = 255;
        };

        forEach(mat, encodeOneHot);
    }

    void toSparse()
    {
        cv::Mat out(mat.size(), CV_8UC1);

        auto encodeSparse = [&out](auto& pixel, const int* position) {
            auto idx = utility::argmax(std::span(pixel.val));
            *out.ptr(position) = idx;
        };

        forEach(mat, encodeSparse);

        mat = out;
    }

    impl channel(int channel) const
    {
        if (channel > mat.channels())
        {
            throw std::out_of_range("Image::channel(): Channel index out of range");
        }

        cv::Mat out;
        cv::extractChannel(mat, out, channel);

        return { std::move(out) };
    }

    void select(const impl& mask)
    {
        if (mat.size() != mask.mat.size())
        {
            throw std::invalid_argument("Image::select(): Image and mask sizes do not match");
        }

        cv::bitwise_and(mat, mask.mat, mat);
    }

    void addAlphaChannel(const impl &mask)
    {
        if (mat.size() != mask.mat.size())
        {
            throw std::invalid_argument("Image::addAlphaChannel(): Image and mask sizes do not match");
        }

        cv::cvtColor(mat, mat, cv::COLOR_BGR2BGRA);
        mat.setTo(0, mask.mat == 0);
    }

    impl depthwiseContour(std::initializer_list<int> ignore_channels) const
    {
        static auto makeMask = [](const cv::Mat& input, cv::Mat& output) -> void
        {
            std::vector<std::vector<cv::Point>> contour;
            cv::findContours(input, contour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            auto index = utility::argmax(contour, [](const auto& a, const auto& b) {
                return cv::contourArea(a) < cv::contourArea(b);
            });

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
                    channels[i].setTo(0);
                    continue;
                }

                makeMask(channels[i], channels[i]);
            }

            cv::Mat out;
            cv::merge(channels, out);

            return { std::move(out) };
        }
    }

    void copyTo(void*& ptr, bool normalized, bool grayscale) const
    {
        // TODO: What if our image is already grayscale?

        if (ptr == nullptr)
        {
            throw std::invalid_argument("Image::copyTo(): Destination pointer is null");
        }

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

    [[nodiscard]]
    impl resize(int width, int height) const
    {
        cv::Mat out;
        cv::resize(mat, out, { width, height });

        return { std::move(out) };
    }

    [[nodiscard]]
    std::vector<uint8_t> encode(std::string_view format) const
    {
        std::vector<uchar> out;
        cv::imencode(format.data(), mat, out);

        return out;
    }

private:
    template<typename Fn>
    static void forEach(cv::Mat& mat, Fn&& callable)
    {
        if      (mat.channels() == 3) mat.forEach<cv::Vec3b>(callable);
        else if (mat.channels() == 4) mat.forEach<cv::Vec4b>(callable);
        else if (mat.channels() == 5) mat.forEach<cv::Vec<uchar, 5>>(callable);
        else if (mat.channels() == 6) mat.forEach<cv::Vec<uchar, 6>>(callable);
        else if (mat.channels() == 7) mat.forEach<cv::Vec<uchar, 7>>(callable);
        else if (mat.channels() == 8) mat.forEach<cv::Vec<uchar, 8>>(callable);
        else if (mat.channels() == 9) mat.forEach<cv::Vec<uchar, 9>>(callable);
    }

    static int typeOf(std::size_t dataSize, const cv::Size& size, int channels)
    {
        auto div = dataSize / (size.area() * channels);

        if (div == 1) return CV_MAKETYPE(CV_8U, channels);
        if (div == 4) return CV_MAKETYPE(CV_32F, channels);
        else
            throw std::invalid_argument("Image::Image(): Unsupported data type");
    }

    static void __check(const cv::Mat& mat)
    {
        if (mat.empty())
        {
            throw std::invalid_argument("Image::__check(): Image is empty");
        }
    }
};

Image::Image(cv::Mat&& image)
        : pImpl(std::make_unique<impl>(std::move(image)))
{
}

Image::Image(std::shared_ptr<impl> pImpl)
        : pImpl(std::move(pImpl))
{
}

Image::Image(std::span<const std::byte> data, int width, int height, int channels)
        : pImpl(std::make_unique<impl>(data, width, height, channels))
{
}

Image::Image(std::span<const std::byte> data)
        : pImpl(std::make_unique<impl>(data))
{
}

Image::Image(const std::filesystem::path &imagePath)
        : pImpl(std::make_unique<impl>(imagePath))
{
}

Image::~Image() = default;

void Image::toOneHot()
{
    pImpl->toOneHot();
}

void Image::toSparse()
{
    pImpl->toSparse();
}

Image& Image::select(const Image& mask)
{
    pImpl->select(*mask.pImpl);
    return *this;
}

Image& Image::crop(std::span<const std::pair<float, float>, 4> points)
{
    pImpl->crop(points);
    return *this;
}

Image& Image::cropToFit(int channel)
{
    pImpl->cropToFit(channel);
    return *this;
}

Image &Image::addAlphaChannel(const Image &mask)
{
    pImpl->addAlphaChannel(*mask.pImpl);
    return *this;
}

Image Image::channel(int channel) const
{
    return Image(std::make_shared<impl>(std::move(pImpl->channel(channel))));
}

Image Image::resize(int width, int height) const
{
    return Image(std::make_shared<impl>(std::move(pImpl->resize(width, height))));
}

Image Image::depthwiseContour(std::initializer_list<int> ignore_channels) const
{
    return Image(std::make_shared<impl>(std::move(pImpl->depthwiseContour(ignore_channels))));
}

void Image::copyTo(void*& ptr, bool normalized, bool grayscale) const
{
    pImpl->copyTo(ptr, normalized, grayscale);
}

std::vector<uint8_t> Image::encode(std::string_view format) const
{
    return pImpl->encode(format);
}

std::size_t Image::countNonZero() const
{
    return cv::countNonZero(pImpl->mat);
}

int Image::width() const
{
    return pImpl->mat.cols;
}

int Image::height() const
{
    return pImpl->mat.rows;
}

int Image::channels() const
{
    return pImpl->mat.channels();
}

cv::Mat& Image::underlying_handle() const
{
    return pImpl->mat;
}
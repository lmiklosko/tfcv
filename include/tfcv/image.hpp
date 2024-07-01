#pragma once
#include "tfcv/defs.hpp"
#include "opencv2/core.hpp"

#include <filesystem>
#include <vector>
#include <span>
#include <memory>

class API Image
{
public:
    Image(cv::Mat&& mat); // NOLINT(*-explicit-constructor)

    /**
     * @brief Construct a new Image object from encoded image data
     * @param data Encoded image data (e.g. PNG, JPEG)
     */
    Image(std::span<const std::byte> data); // NOLINT(*-explicit-constructor)

    /**
     * @brief Load an image from a file
     * @param imagePath Path to the image file
     */
    Image(const std::filesystem::path& imagePath); // NOLINT(*-explicit-constructor)

    /**
     * Crops the image to the specified rectangle.
     *
     * @param rect
     * @return
     */
    void crop(std::span<const cv::Point2f, 4> rect);

    /**
     * Resizes the image to the specified dimensions.
     *
     * @param width
     * @param height
     */
    [[nodiscard]]
    Image resize(int width, int height) const;

    /**
     * Generates a mask of biggest contour for each channel separately.
     *
     * @param ignore_channels Channels to ignore. This will result in completely black channel. Default is none.
     * @return Image with biggest contour for each channel.
     */
    [[nodiscard]]
    Image depthwiseContour(std::initializer_list<int> ignore_channels = {}) const;

    /**
     * Copies the image data to a buffer.
     *
     * @param ptr Buffer to copy to
     * @param normalized Whether to normalize the data
     * @param grayscale Whether to convert the image to grayscale
     */
    void copyTo(void*& ptr, bool normalized, bool grayscale) const;

    /**
     * Encodes the image.
     *
     * @param format Image format to encode to
     * @return byte buffer containing encoded image
     */
    [[nodiscard]]
    std::vector<uint8_t> encode(std::string_view format = ".png") const;


    // ~~~~~~~~~~~~~~~~~~~~~~~~ Getters ~~~~~~~~~~~~~~~~~~~~~~~~

    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] int channels() const;

private:
    cv::Mat mat;

    static void __check(const cv::Mat& mat);
};
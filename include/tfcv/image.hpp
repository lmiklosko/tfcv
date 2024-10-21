#pragma once
#include "tfcv/defs.hpp"

#include <filesystem>
#include <vector>
#include <span>
#include <memory>

/* Forward declaration */
namespace cv { class API Mat; }

TFCV_NAMESPACE_BEGIN

class API Image
{
public:
    explicit Image(cv::Mat&& mat);

    /**
     * @brief Construct a new Image object from raw image data
     * @param data Raw image data
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels
     */
    Image(std::span<const std::byte> data, int width, int height, int channels);  // NOLINT(*-explicit-constructor)

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
     * @brief explicit destructor - required for pImpl idiom (some compilers do not allow it to be implicit)
     *
     */
    ~Image();

    /**
     * Converts the probability image to one-hot encoded image.
     */
    void toOneHot();

    /**
     * Converts the probability image to scce encoded image.
     */
    void toSparse();

    /**
     * Selects the specified area from the image.
     *
     * @param mask
     * @return self
     */
    Image& select(const Image& mask);

    /**
     * Crops the image to the specified rectangle.
     *
     * @param rect
     * @return
     */
    Image& crop(std::span<const std::pair<float, float>, 4> rect);

    /**
     * Crops image to the area of interest specified by channel.
     *
     * @details This function will crop the image to the smallest rectangle that contains all non-zero pixels
     * in the specified channel. The resulting image will have the same dimensions or smaller as the original image.
     *
     * @param channel
     */
    Image& cropToFit(int channel);

    /**
     * Adds alpha channel to the image.
     *
     * @param mask Mask to use as alpha channel
     */
    Image& addAlphaChannel(const Image& mask);

    /**
     * Extract channel from the image.
     *
     * @param channel
     * @return
     */
    [[nodiscard]]
    Image channel(int channel) const;

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

    [[nodiscard]]
    std::size_t countNonZero() const;

    // ~~~~~~~~~~~~~~~~~~~~~~~~ Getters ~~~~~~~~~~~~~~~~~~~~~~~~

    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] int channels() const;

    [[nodiscard]] cv::Mat& underlying_handle() const;

private:
    struct impl;
    std::shared_ptr<impl> pImpl;

    explicit Image(std::shared_ptr<impl> pImpl);
};

TFCV_NAMESPACE_END
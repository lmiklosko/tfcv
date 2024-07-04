#pragma once
#include "tfcv/defs.hpp"
#include "tfcv/image.hpp"
#include "tfcv/utility.hpp"
#include "tfcv/ml/processor.hpp"

#include <vector>
#include <span>
#include <string_view>
#include <opencv2/core.hpp>

TFCV_NAMESPACE_WITH_BEGIN(ml)

class Classifier
{
    std::unique_ptr<tfcv::ml::IProcessor> _interpreter;
    int num_classes;
public:
    /**
     * @brief Construct a new Pest Classifier object from model file
     * @param model_path
     */
    explicit Classifier(std::string_view model_path)
        : _interpreter(tfcv::ml::IProcessor::create(model_path))
        , num_classes(_interpreter->output_dims()[1])
    {
    }

    /**
     * @brief Construct a new Pest Classifier object from model stored in memory
     * @param model_data
     */
    explicit Classifier(std::span<const std::byte> model_data)
        : _interpreter(tfcv::ml::IProcessor::create(model_data))
        , num_classes(_interpreter->output_dims()[1])
    {
    }

    /**
     * @brief Classify the given images
     * @param images Images to classify
     * @throws std::invalid_argument if images are empty
     * @throws std::runtime_error if the model fails to invoke
     * @return Classes for every image in the set
     */
    [[nodiscard]]
    std::vector<uint8_t> classify(std::span<tfcv::Image> images) const
    {
        if (images.empty())
        {
            return {};
        }

        auto splits = tfcv::utility::split(images, 64);

        std::vector<uint8_t> out;
        for (const auto& it : splits)
        {
            auto output = reinterpret_cast<const uint8_t*>(_interpreter->run(it).data());

            for (int i = 0; i < it.size(); ++i)
            {
                auto maxIndex = std::max_element(output, output + num_classes) - output;
                out.emplace_back(static_cast<uint8_t>(maxIndex));

                output += num_classes;
            }
        }

        return out;
    }
};

TFCV_NAMESPACE_END

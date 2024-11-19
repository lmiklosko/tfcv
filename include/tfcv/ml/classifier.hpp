#pragma once
#include "processor.hpp"
#include "../defs.hpp"
#include "../image.hpp"
#include "../utility.hpp"

#include <vector>
#include <span>
#include <string_view>
#include <opencv2/core.hpp>

TFCV_NAMESPACE_WITH_BEGIN(ml)

class Classifier
{
    std::unique_ptr<IProcessor> _interpreter;
    int num_classes;
public:
    /**
     * @brief Construct a new Pest Classifier object from model file
     * @param model_path
     */
    explicit Classifier(std::string_view model_path)
        : _interpreter(IProcessor::create(model_path))
        , num_classes(_interpreter->output_dims()[1])
        , pipeline(_interpreter->pipeline)
    {
    }

    /**
     * @brief Construct a new Pest Classifier object from model stored in memory
     * @param model_data
     */
    explicit Classifier(std::span<const std::byte> model_data)
        : _interpreter(IProcessor::create(model_data))
        , num_classes(_interpreter->output_dims()[1])
        , pipeline(_interpreter->pipeline)
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
    std::vector<uint8_t> classify(std::span<tfcv::Image> images, size_t batch = 64) const
    {
        if (images.empty())
        {
            return {};
        }

        auto splits = tfcv::utility::split(images, batch);

        std::vector<uint8_t> out;
        for (const auto& it : splits)
        {
            auto data = _interpreter->run(it);
            if (_interpreter->output_type() == TensorType::FLOAT32)
            {
                auto output = reinterpret_cast<const float*>(data.data());
                for (int i = 0; i < it.size(); ++i)
                {
                    out.emplace_back(argmax(output, num_classes));
                    output += num_classes;
                }
            }
            else if (_interpreter->output_type() == TensorType::UINT8)
            {
                auto output = reinterpret_cast<const uint8_t*>(data.data());
                for (int i = 0; i < it.size(); ++i)
                {
                    out.emplace_back(argmax(output, num_classes));
                    output += num_classes;
                }
            }
        }

        return out;
    }

    Pipeline& pipeline;

private:
    template<typename T>
    [[nodiscard]] uint8_t argmax(const T* data, int size) const
    {
        return std::max_element(data, data + size) - data;
    }


};

TFCV_NAMESPACE_END

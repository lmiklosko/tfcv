#pragma once
#include "tfcv/defs.hpp"
#include "tfcv/image.hpp"
#include "tfcv/ml/preprocessing/pipeline.hpp"

#include <span>
#include <vector>
#include <cstdint>

namespace cv { class Mat; }

TFCV_NAMESPACE_WITH_BEGIN(ml)

    enum class TensorType
    {
        INVALID = 0,
        FLOAT32,
        INT32,
        UINT8,
        INT64,
        STRING,
        BOOL,
        INT16,
        COMPLEX64,
        INT8,
        FLOAT16,
        FLOAT64,
        COMPLEX128,
        UINT64,
        RESOURCE,
        VARIANT,
        UINT32,
        UINT16,
        INT4
    };

    class API IProcessor
    {
    public:
        virtual ~IProcessor() = default;

        /**
         * @brief Run the interpreter on the given input
         *
         * @param input Input data
         * @throws std::invalid_argument if input is empty
         * @throws std::runtime_error if the interpreter fails to invoke
         * @return Output data
         */
        [[nodiscard]] virtual std::span<const std::byte> run(std::span<const Image> input) const = 0;

        /**
         * @brief Get the input dimensions
         *
         * @return Input dimensions
         */
        [[nodiscard]] virtual std::span<int> input_dims() const noexcept = 0;

        [[nodiscard]] virtual TensorType input_type() const noexcept = 0;

        /**
         * @brief Get the output dimensions
         *
         * @return Output dimensions
         */
        [[nodiscard]] virtual std::span<int> output_dims() const noexcept = 0;

        [[nodiscard]] virtual TensorType output_type() const noexcept = 0;

        /**
         * @brief Construct new ML processor
         *
         * @param model_path Path to the model file
         * @return
         */
        static std::unique_ptr<IProcessor> create(std::string_view model_path);

        /**
         * @brief Construct new ML processor
         *
         * @param model_data Binary model data
         * @return
         */
        static std::unique_ptr<IProcessor> create(std::span<const std::byte> model_data);


        Pipeline pipeline;
    };

TFCV_NAMESPACE_END
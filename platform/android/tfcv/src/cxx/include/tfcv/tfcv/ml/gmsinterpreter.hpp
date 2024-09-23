#pragma once
#include "tfcv/defs.hpp"
#include "tfcv/ml/processor.hpp"

#include <string_view>

TFCV_NAMESPACE_WITH_BEGIN(ml)

    class API GmsInterpreter : public IProcessor
    {
    public:
        /**
         * @brief Construct a new Interpreter object
         * @param model_path Path to the model file
         */
        explicit GmsInterpreter(std::string_view model_path);

        /**
         * @brief Construct a new Interpreter object
         * @param model_data Model data
         */
        explicit GmsInterpreter(std::span<const std::byte> model_data);

        /**
         * @brief Run the interpreter on the given input
         * @param input Input data
         * @throws std::invalid_argument if input is empty
         * @throws std::runtime_error if the interpreter fails to invoke
         * @return Output data
         */
        [[nodiscard]]
        std::span<const std::byte> run(std::span<const Image> input) const override;

        /**
         * @brief Get the input dimensions
         * @return Input dimensions
         */
        [[nodiscard]]
        std::span<int> input_dims() const noexcept override;

        /**
         * @brief Get the output dimensions
         * @return Output dimensions
         */
        [[nodiscard]]
        std::span<int> output_dims() const noexcept override;


        ~GmsInterpreter() override;

    private:
        class LOCAL impl;
        std::unique_ptr<impl> pImpl;
    };

TFCV_NAMESPACE_END
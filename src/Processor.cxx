#include "tfcv/ml/processor.hpp"
#include "tfcv/ml/localinterpreter.hpp"

using namespace tfcv::ml;


OVERRIDABLE std::unique_ptr<IProcessor> IProcessor::create(std::string_view model_path)
{
    return std::make_unique<LocalInterpreter>(model_path);
}

OVERRIDABLE std::unique_ptr<IProcessor> IProcessor::create(std::span<const std::byte> model_data)
{
    return std::make_unique<LocalInterpreter>(model_data);
}
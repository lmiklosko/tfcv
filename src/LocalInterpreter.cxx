#include "tfcv/ml/localinterpreter.hpp"
#include "tfcv/utility.hpp"
#include "cxlog/GLog.hpp"

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/minimal_logging.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/delegates/coreml/coreml_delegate.h"

#include <thread>
#include <sstream>

using namespace tfcv::ml;
using namespace cxlog;

class LocalInterpreter::impl
{
    std::unique_ptr<tflite::FlatBufferModel> _model;
    std::unique_ptr<tflite::Interpreter> _interpreter;
    std::unique_ptr<TfLiteDelegate, void(*)(TfLiteDelegate*)> _delegate;

    std::shared_ptr<ILogger> _logger;

    mutable bool tensors_allocated = false;

public:
    explicit impl(std::string_view model_path)
        : _model(tflite::FlatBufferModel::BuildFromFile(model_path.data()))
        , _delegate(nullptr, nullptr)
        , _logger(gLogFactory->CreateLogger("TFInterpreter"))
    {
        build_interpreter();
    }

    explicit impl(std::span<const std::byte> model_data)
        : _model(tflite::FlatBufferModel::BuildFromBuffer(reinterpret_cast<const char*>(model_data.data()), model_data.size()))
        , _delegate(nullptr, nullptr)
        , _logger(gLogFactory->CreateLogger("TFInterpreter"))
    {
        build_interpreter();
    }

    [[nodiscard]] std::span<int> input_dims() const noexcept
    {
        return {
            _interpreter->input_tensor(0)->dims->data,
            static_cast<unsigned long>(_interpreter->input_tensor(0)->dims->size)
        };
    }

    [[nodiscard]] std::span<int> output_dims() const noexcept
    {
        return {
            _interpreter->output_tensor(0)->dims->data,
            static_cast<unsigned long>(_interpreter->output_tensor(0)->dims->size)
        };
    }

    [[nodiscard]] std::span<const std::byte> run(std::span<const Image> input, const Pipeline& pipeline) const
    {
        if (input.empty())
        {
            _logger->LogError("run(): Input data is empty");
            throw std::invalid_argument("TFInterpreter::impl::run(): Input data is empty");
        }

        auto dur_copy = utility::measure_time([&]{ copy_data(input, pipeline); });
        auto dur_invoke = utility::measure_time([&]{
            if (kTfLiteOk != _interpreter->Invoke())
            {
                _logger->LogError("run(): Failed to invoke interpreter");
                throw std::runtime_error("TFInterpreter::impl::run(): Failed to invoke interpreter");
            }
        });

        _logger->Log(LogLevel::Info, "Inference time - copy: {}, invoke: {}",
            dur_copy.count(),
            dur_invoke.count()
        );

        return {
            reinterpret_cast<const std::byte*>(_interpreter->output_tensor(0)->data.raw_const),
            _interpreter->output_tensor(0)->bytes // TODO: Might require less bytes depending on the input
        };
    }

private:
    void build_interpreter()
    {
        tflite::LoggerOptions::SetMinimumLogSeverity(tflite::LogSeverity::TFLITE_LOG_SILENT);
        _logger->LogDebug("Building interpreter");

        if (!_model)
        {
            _logger->LogCritical("Invalid model provided!");
            throw std::runtime_error("TFInterpreter::impl::build_interpreter(): Model could not be found!");
        }

        tflite::ops::builtin::BuiltinOpResolver resolver;
        tflite::InterpreterBuilder(*_model, resolver)(&_interpreter);
        if (!_interpreter)
        {
            _logger->LogCritical("Failed to build the interpreter");
            throw std::runtime_error("TFInterpreter::impl::build_interpreter(): Failed to build the interpreter");
        }

        add_delegates();

        if (_interpreter->input_tensor(0)->dims->size != 4 ||
            (_interpreter->input_tensor(0)->dims->data[3] != 3 && _interpreter->input_tensor(0)->dims->data[3] != 1) ||
            (_interpreter->input_tensor(0)->type != kTfLiteFloat32 && _interpreter->input_tensor(0)->type != kTfLiteUInt8))
        {
            throw std::runtime_error("TFInterpreter::impl::build_interpreter(): Unsupported input tensor format");
        }

        // ================= Specs =================
        _logger->Log(LogLevel::Info, "TF model loaded\n\tInput: {}\n\tOutput: {}",
            tf_info(_interpreter->input_tensor(0)),
            tf_info(_interpreter->output_tensor(0))
        );
    }

    void copy_data(std::span<const Image> span, const Pipeline& pipeline) const
    {
        auto [width, height, channels] = std::make_tuple(_interpreter->input_tensor(0)->dims->data[1], _interpreter->input_tensor(0)->dims->data[2], _interpreter->input_tensor(0)->dims->data[3]);
        if (_interpreter->input_tensor(0)->dims->data[0] < (int)span.size())
        {
            _logger->Log(LogLevel::Debug, "Resizing input tensor to [{}, {}, {}, {}]",
                         (int)span.size(), width, height, channels);

            if (kTfLiteOk != _interpreter->ResizeInputTensor(0, { (int)span.size(), width, height, channels }))
            {
                _logger->Log(LogLevel::Error, "Failed to resize input tensor");
                throw std::runtime_error("TFInterpreter::impl::copy_data(): Failed to resize input tensor");
            }

            if (kTfLiteOk != _interpreter->AllocateTensors())
            {
                _logger->Log(LogLevel::Error, "Failed to allocate tensors");
                throw std::runtime_error("TFInterpreter::impl::copy_data(): Failed to allocate tensors");
            }

            tensors_allocated = true;
        }
        else if (!tensors_allocated)
        {
            if (kTfLiteOk != _interpreter->AllocateTensors())
            {
                _logger->Log(LogLevel::Error, "Failed to allocate tensors");
                throw std::runtime_error("TFInterpreter::impl::copy_data(): Failed to allocate tensors");
            }
        }

        void *ptr = _interpreter->input_tensor(0)->data.raw;
        for (auto& orig : span)
        {
            auto mat = pipeline.run(orig);
            mat.copyTo(ptr);
        }
    }

    /**
     * Modifies graph with available delegates
     *
     * @details
     * For Android, NNAPI delegate is used if available, otherwise GPU delegate is used
     * For iOS, CoreML delegate is used
     * For other platforms, GPU delegate is used
     */
    void add_delegates()
    {
        _logger->LogDebug("Setting number of threads to {}", std::thread::hardware_concurrency());
        _interpreter->SetNumThreads((int)std::thread::hardware_concurrency());

        _logger->LogDebug("Adding delegates");

        /* Core ML Delegate will run under iOS only (iPhone/iPad) */
#ifdef __APPLE__
        #include <TargetConditionals.h>
 #if TARGET_OS_IPHONE
        TfLiteCoreMlDelegateOptions coremlOptions;
        coremlOptions.enabled_devices = TfLiteCoreMlDelegateEnabledDevices::TfLiteCoreMlDelegateDevicesWithNeuralEngine;

        auto coremlDelegate = TfLiteCoreMlDelegateCreate(&coremlOptions);
        if (coremlDelegate)
        {
            if (_interpreter->ModifyGraphWithDelegate(coremlDelegate) == kTfLiteOk)
            {
                _logger->Log(LogLevel::Info, "Using CoreML delegate");
                _delegate = { coremlDelegate, TfLiteCoreMlDelegateDelete };
                return;
            }
            else
            {
                TfLiteCoreMlDelegateDelete(coremlDelegate);
            }
        }
 #endif
        // TODO: Add Metal delegate
#endif

        /* Try NNAPI delegate. It should be available beginning api level 26+, however TF recommends min 27 */
//#ifdef __ANDROID__
//        auto nnapiDelegate = tflite::NnApiDelegate();
//        if (_interpreter->ModifyGraphWithDelegate(nnapiDelegate) == kTfLiteOk)
//        {
//            _logger->Log(LogLevel::Info, "Using NNAPI delegate");
//            return;
//        }
//#endif

        /* Every platform supports GPU delegate based on the OpenCL availability (might do nothing) */
        /* NOTE: GPU delegate has an issue with missing dependencies / sources (see https://github.com/tensorflow/tensorflow/issues/61312)
         * needs to be re-enabled once https://github.com/tensorflow/tensorflow/pull/61381 is merged */
#if !defined(__APPLE__) ///* REMOVE CODE AFTER THIS COMMENT BASED ON THE ABOVE */ && !defined(__ANDROID__)
        auto gpuOptions = TfLiteGpuDelegateOptionsV2Default();
        auto gpuDelegate = TfLiteGpuDelegateV2Create(&gpuOptions);
        if (gpuDelegate)
        {
            if (auto rv = _interpreter->ModifyGraphWithDelegate(gpuDelegate); rv == kTfLiteOk)
            {
                _logger->LogInfo("Using GPU delegate");
                _delegate = { gpuDelegate, TfLiteGpuDelegateV2Delete };
                return;
            }
            else
            {
                _logger->LogError("Failed to use GPU delegate, error code: {}", rv);
                TfLiteGpuDelegateV2Delete(gpuDelegate);
            }
        }
#endif

        _logger->LogWarning("No delegate available");
    }

    static std::string tf_info(TfLiteTensor* tensor) noexcept
    {
        std::ostringstream ss;
        ss << "[" << tensor->dims->data[0];
        for (int i = 1; i < tensor->dims->size; ++i)
            ss << ", " << tensor->dims->data[i];
        ss << "] " << TfLiteTypeGetName(tensor->type);
        return ss.str();
    }
};

// ================= TF::Interpreter =================

LocalInterpreter::LocalInterpreter(std::string_view model_path)
    : pImpl(std::make_unique<impl>(model_path))
{
}

LocalInterpreter::LocalInterpreter(std::span<const std::byte> model_data)
    : pImpl(std::make_unique<impl>(model_data))
{
}

std::span<const std::byte> LocalInterpreter::run(std::span<const Image> input) const
{
    return pImpl->run(input, this->pipeline);
}

std::span<int> LocalInterpreter::input_dims() const noexcept
{
    return pImpl->input_dims();
}

std::span<int> LocalInterpreter::output_dims() const noexcept
{
    return pImpl->output_dims();
}

LocalInterpreter::~LocalInterpreter() = default;
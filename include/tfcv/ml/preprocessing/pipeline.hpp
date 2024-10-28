#pragma once
#include "tfcv/defs.hpp"
#include "tfcv/image.hpp"
#include "tfcv/ml/preprocessing/op.hpp"

TFCV_NAMESPACE_WITH_BEGIN(ml)

class API Pipeline
{
    std::vector<std::unique_ptr<Op>> _ops;
public:
    Pipeline() = default;

    Pipeline& add(std::unique_ptr<Op> op)
    {
        _ops.push_back(std::move(op));
        return *this;
    }

    template<typename T, typename... Args>
    Pipeline& add(Args&&... args)
    {
        static_assert(std::is_base_of_v<Op, T>, "T must be derived from Op");
        static_assert(std::is_constructible_v<T, Args...>, "T must be constructible with Args");

        _ops.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return *this;
    }

    void clear();

    [[nodiscard]]
    tfcv::Image run(const tfcv::Image& image) const;
};

TFCV_NAMESPACE_END
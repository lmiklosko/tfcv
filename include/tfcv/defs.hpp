#pragma once

#define API __attribute__((visibility("default")))
#define LOCAL __attribute__((visibility("hidden")))
#define OVERRIDABLE __attribute__((weak))

#define TFCV_NAMESPACE_BEGIN namespace tfcv {
#define TFCV_NAMESPACE_WITH_BEGIN(name) namespace tfcv::name {
#define TFCV_NAMESPACE_END }
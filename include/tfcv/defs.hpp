#pragma once

#define API __attribute__((visibility("default")))
#define LOCAL __attribute__((visibility("hidden")))
#define OVERRIDABLE __attribute__((weak))
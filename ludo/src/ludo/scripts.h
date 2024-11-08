/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <functional>

#include "core.h"

namespace ludo
{
  using script = std::function<void(instance& instance)>; ///< A function that can be executed during various points in the lifecycle of an instance.

  // TODO something much better than this *global* hack
  extern std::vector<float> total_script_times;

  template<typename T>
  T* add(instance& instance, const std::function<void(ludo::instance& instance)>& init, const std::string& partition = "default");

  template<typename T, typename Arg1>
  T* add(instance& instance, const std::function<void(ludo::instance& instance, Arg1 arg1)>& init, Arg1 arg1, const std::string& partition = "default");

  template<typename T, typename Arg1, typename Arg2>
  T* add(instance& instance, const std::function<void(ludo::instance& instance, Arg1 arg1, Arg2 arg2)>& init, Arg1 arg1, Arg2 arg2, const std::string& partition = "default");

  template<typename T, typename Arg1, typename Arg2, typename Arg3>
  T* add(instance& instance, const std::function<void(ludo::instance& instance, Arg1 arg1, Arg2 arg2, Arg3 arg3)>& init, Arg1 arg1, Arg2 arg2, Arg3 arg3, const std::string& partition = "default");
}

#include "scripts.hpp"

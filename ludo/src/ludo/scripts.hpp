/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "data.h"
#include "scripts.h"

namespace ludo
{
  template<typename T>
  T* add(instance& instance, const std::function<void()>& init, const std::string& partition)
  {
    return add<T>(instance, [=](ludo::instance& instance) { init(); }, partition);
  }

  template<typename T, typename Arg1>
  T* add(instance& instance, const std::function<void(ludo::instance& instance, Arg1 arg1)>& init, Arg1 arg1, const std::string& partition)
  {
    return add<T>(instance, [=](ludo::instance& instance) { init(instance, arg1); }, partition);
  }

  template<typename T, typename Arg1, typename Arg2>
  T* add(instance& instance, const std::function<void(ludo::instance& instance, Arg1 arg1, Arg2 arg2)>& init, Arg1 arg1, Arg2 arg2, const std::string& partition)
  {
    return add<T>(instance, [=](ludo::instance& instance) { init(instance, arg1, arg2); }, partition);
  }

  template<typename T, typename Arg1, typename Arg2, typename Arg3>
  T* add(instance& instance, const std::function<void(ludo::instance& instance, Arg1 arg1, Arg2 arg2, Arg3 arg3)>& init, Arg1 arg1, Arg2 arg2, Arg3 arg3, const std::string& partition)
  {
    return add<T>(instance, [=](ludo::instance& instance) { init(instance, arg1, arg2, arg3); }, partition);
  }
}

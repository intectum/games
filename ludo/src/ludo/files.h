/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_FILES_H
#define LUDO_FILES_H

#include <string>

namespace ludo
{
  extern std::string asset_dir; ///< Read-only files packaged with the application
  extern std::string user_dir; ///< Read-write files specific to the current user
}

#endif // LUDO_FILES_H

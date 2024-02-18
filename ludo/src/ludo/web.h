/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_WEB_H
#define LUDO_WEB_H

#include <ludo/core.h>
#include <ludo/rendering.h>

namespace ludo
{
  struct LUDO_API web_browser
  {
    uint64_t id = 0; ///< The ID of the web browser.
  };

  LUDO_API void add_web(context& context);

  LUDO_API web_browser create_web_browser(texture& texture, const std::string& initial_page);

  LUDO_API void execute_javascript(texture& texture, web_browser& web_browser, const std::string& code);
}

#endif // LUDO_WEB_H

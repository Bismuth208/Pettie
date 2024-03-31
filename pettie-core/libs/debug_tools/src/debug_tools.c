#include "debug_tools.h"

void
init_debug_tools(void)
{
  init_debug_tools_arch();
  init_async_printf();
  init_debug_assist();
}

void
debug_tools_start(void)
{
  debug_assist_start();
}
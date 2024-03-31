# Debug Tools

Simple but yet effective Async printf() and Profiler!

Written in C and have small, if not tiny footprint.
Printf() - now with threads!

## How does it work?

This library is using redirected stdout where implementation is done by you (or not)!
It's doing printf in it's own thread with minimal impact on runtime App.

Main API functions are:
 - ASYNC_PRINTF
 - PROFILE_POINT

As a minor bonus this library can collect and print runtime stats from FreeRTOS.


## Project setup

For IAR Projects you have to do next steps:
 - add this folder to the project tree
 - CORTEX and/or ARM preprocessor definitions
 - add those additional include dirs:
```
$PROJ_DIR$\..\..\library\debug_tools\src
$PROJ_DIR$\..\..\library\debug_tools\src\arch
$PROJ_DIR$\..\..\library\debug_tools\src\arch\arm_cortex
```

    
For the ESP-IDF you have to do next steps:
 - add path to your CMakeLists.txt location of this component or put it into existing one


## Library usage

In very beginning of your program place:
```
init_debug_tools();
```


## Creating a configuration file

This library relays on import of 'debug_tools_conf.h' file into you project.

Example of the configuration file named as 'template_debug_tools_conf.h'.
This file all following and require info to create your own 'debug_tools_conf.h' file.

Note, what for ESP32 you have to describe configuration in Kconfig.projbuild in your project,
but import of the debug_tools_conf.h is mandatory!

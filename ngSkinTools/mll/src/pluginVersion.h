#pragma once

/**
 * version is defined when compiling as a global define
 */

#ifndef NGSKINTOOLS_PLUGIN_VERSION
#define NGSKINTOOLS_PLUGIN_VERSION "1.0"
#endif

#ifndef NGSKINTOOLS_PLUGIN_VENDOR
#define NGSKINTOOLS_PLUGIN_VENDOR "neglostyti.com"
#endif

/*
 * remapped from define to constant to have type-safety in code
 */
const char PLUGIN_VENDOR[] = NGSKINTOOLS_PLUGIN_VENDOR;
const char PLUGIN_VERSION[] = NGSKINTOOLS_PLUGIN_VERSION;

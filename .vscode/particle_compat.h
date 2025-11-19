/**
 * Particle Compatibility Header for IntelliSense
 * This file provides missing type definitions and macros for proper C++ IntelliSense parsing
 */

#ifndef PARTICLE_COMPAT_H
#define PARTICLE_COMPAT_H

// Define 'uint' as unsigned int (Particle convention)
#ifndef uint
typedef unsigned int uint;
#endif

// Define 'retained' attribute for backup RAM storage (STM32F2xx specific)
#ifndef retained
#define retained __attribute__((section(".backup_sram")))
#endif

// Define 'retained_system' for system retained variables
#ifndef retained_system
#define retained_system __attribute__((section(".backup_sram_system")))
#endif

// Ensure pinmap_defines.h is included for Electron pin definitions (D0-D7, etc.)
// Note: For Electron (PLATFORM_ID=10), use electron pinmap
#ifdef __INTELLISENSE__
// During IntelliSense parsing, explicitly include Electron pin definitions
// First ensure PLATFORM_ELECTRON_PRODUCTION is defined for conditional inclusion
#ifndef PLATFORM_ELECTRON_PRODUCTION
#define PLATFORM_ELECTRON 10
#define PLATFORM_ELECTRON_PRODUCTION 10
#endif
#include "pinmap_defines.h"
#elif PLATFORM_ID == 10
#include "pinmap_defines.h"
#endif

#endif // PARTICLE_COMPAT_H

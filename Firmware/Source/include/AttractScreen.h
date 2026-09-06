#ifndef _ATTRACT_SCREEN_H_
#define _ATTRACT_SCREEN_H_

#include <Arduino.h>

/** True while the attract demo loop is running (pauses minibadge scans / busywork). */
extern volatile bool demoModeActive;

/** Idle time on the main menu before attract starts (ms). */
#ifndef ATTRACT_IDLE_MS
#define ATTRACT_IDLE_MS 15000UL
#endif

/**
 * Run the looping attract / demo screen.
 * Display-only: disables WiFi for the duration, no API calls.
 * Any button press returns to the caller (main menu).
 */
void AttractScreenRun();

#endif

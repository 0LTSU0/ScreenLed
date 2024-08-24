#ifndef DEFS_H
#define DEFS_H

#ifdef _WIN32
#define IS_WINDOWS 1
#define SHOW_PREVIEW 1
#else
#define IS_WINDOWS 0
#define SHOW_PREVIEW 0
#endif

constexpr int NUM_SEGMENTS = 20;
constexpr int NUM_LEDS_PER_SEG = 10;

#endif // DEFS_H

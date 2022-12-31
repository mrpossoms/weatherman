#ifndef WM_PROTO
#define WM_PROTO

#include <inttypes.h>

#define PACKED __attribute__((packed))

typedef struct PACKED {
  int32_t rssi;
  uint8_t measurement_count;
} header_t;

typedef struct PACKED {
  char sensor[4];
  char unit[4];
  float value;
} measurement_t;

#endif

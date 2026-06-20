#ifndef PLUG_H_
#define PLUG_H_
#include <complex.h>
#include <raylib.h>
#define CAPACITY 512


float  left_global_samples[CAPACITY];
float  right_global_samples[CAPACITY];
float MAX_SAMPLE=0.0f;
typedef struct {
    unsigned int global_channels;
    float complex fft_left_global_samples[CAPACITY];
    float complex fft_right_global_samples[CAPACITY];
    Music music;
} Plug;

typedef void (*plug_hello_t)(void);
typedef void (*plug_init_t)(Plug *plug, const char* file_path);
typedef void (*plug_update_t)(Plug* plug);
#endif //PLUG_H_

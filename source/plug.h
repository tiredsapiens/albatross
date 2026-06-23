#ifndef PLUG_H_
#define PLUG_H_
#include <complex.h>
#include <raylib.h>

#define CAPACITY (1<<15)


typedef struct {
    unsigned int global_channels;
    float complex fft_global_samples[CAPACITY];
    float smoothed[CAPACITY];
    float snapshot[CAPACITY];
    Music music;
} Plug;

typedef void (plug_hello_t)(void);
typedef void (plug_init_t)(Plug *plug, const char* file_path);
typedef void (plug_update_t)(Plug* plug);
typedef void (plug_pre_reload_t)(Plug* plug);
typedef void (plug_post_reload_t)(Plug* plug);

#define LIST_OF_PLUGS\
    PLUG(plug_init)\
    PLUG(plug_update)\
    PLUG(plug_pre_reload)\
    PLUG(plug_post_reload)\

#endif //PLUG_H_

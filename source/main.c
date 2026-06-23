#include <assert.h>
#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "plug.h"
#include <dlfcn.h>
#include <stdbool.h>

typedef struct {
    float left;
    float right;
} Frame;

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])

#ifdef HOTRELOAD
#define PLUG(name) name##_t *name=NULL;
#else
#define PLUG(name) name##_t name;
#endif
LIST_OF_PLUGS
#undef PLUG

Plug plug ={0};
const char * libplug_file_name="./build/libplug.so";
void * libplug=NULL;

#ifdef HOTRELOAD
bool reload_libplug(void){
   if (libplug!=NULL) dlclose(libplug);
   libplug=dlopen (libplug_file_name,RTLD_NOW);
    if (libplug==NULL){
        fprintf(stderr,"ERROR: couldnt not load %s: %s\n",libplug_file_name,dlerror());
        return false;
    }
#define PLUG(name)\
        name=dlsym(libplug,#name);\
        if (name==NULL){\
            fprintf(stderr,"ERROR: couldnt not load symbol %s from %s: %s\n",#name,libplug,dlerror());\
            return false;\
        }\
    LIST_OF_PLUGS
#undef PLUG
    return true;
}
#else
#define reload_libplug() true
#endif

int main(int argc,char* argv[]) {
    if (argc>2){
        printf("More than 1 parameter was used\n");
        printf("usage : albatros <audiofile>\n");
    }
    char* filename=argv[1];
    if (!reload_libplug()) return 1; 
    InitWindow(800, 600, "Albatross");
    SetTargetFPS(60);
    InitAudioDevice();
    plug_init(&plug,filename);
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)){
            plug_pre_reload(&plug);
            if (!reload_libplug()){
                return 1;
            }
            plug_post_reload(&plug);
        }
        plug_update(&plug);
    }
    return 0;
}

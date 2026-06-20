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

plug_hello_t plug_hello=NULL;
plug_init_t plug_init=NULL;
plug_update_t plug_update=NULL;
Plug plug ={0};
const char * libplug_file_name="libplug.so";
void * libplug=NULL;

bool reload_libplug(void){

   if (libplug!=NULL) dlclose(libplug);
   libplug=dlopen (libplug_file_name,RTLD_NOW);
    if (libplug==NULL){
        fprintf(stderr,"ERROR: couldnt not load %s: %s\n",libplug_file_name,dlerror());
        return false;
    }
    plug_init=dlsym(libplug,"plug_init");
    if (plug_init==NULL){
        fprintf(stderr,"ERROR: couldnt not load symbol plug_init: %s\n",dlerror());
        return false;
    }
    plug_update=dlsym(libplug,"plug_update");
    if (plug_update==NULL){
        fprintf(stderr,"ERROR: couldnt not load symbol plug_update: %s\n",dlerror());
        return false;
    }
    return true;
}


int main() {
    if (!reload_libplug()) return 1; 
    InitWindow(2000, 600, "Albatross");
    SetTargetFPS(60);
    InitAudioDevice();
    plug_init(&plug,"./audio/celestial_symphony.mp3");
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)){
            if (!reload_libplug()){
                return 1;
            }
        }
        plug_update(&plug);
    }
    return 0;
}

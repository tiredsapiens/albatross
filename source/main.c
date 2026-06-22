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
plug_pre_reload_t plug_pre_reload=NULL;
plug_post_reload_t plug_post_reload=NULL;
Plug plug ={0};
const char * libplug_file_name="./build/libplug.so";
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
    plug_pre_reload=dlsym(libplug,"plug_pre_reload");
    if (plug_update==NULL){
        fprintf(stderr,"ERROR: couldnt not load symbol plug_pre_reload: %s\n",dlerror());
        return false;
    }

    plug_post_reload=dlsym(libplug,"plug_post_reload");
    if (plug_update==NULL){
        fprintf(stderr,"ERROR: couldnt not load symbol plug_post_reload: %s\n",dlerror());
        return false;
    }
    return true;
}


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

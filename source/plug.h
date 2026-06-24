#ifndef PLUG_H_
#define PLUG_H_
#include <complex.h>
#include <raylib.h>

#define CAPACITY (1<<15)



/* typedef void (plug_hello_t)(void); */
/* typedef void (plug_init_t)(Plug *plug, const char* file_path); */
/* typedef void (plug_update_t)(Plug* plug); */
/* typedef void (plug_pre_reload_t)(Plug* plug); */
/* typedef void (plug_post_reload_t)(Plug* plug); */

#define LIST_OF_PLUGS\
    PLUG(plug_init,void,const char* file_path)\
    PLUG(plug_update,void,void)\
    PLUG(plug_pre_reload,void*,void)\
    PLUG(plug_post_reload,void,void*)\

#define PLUG(name,ret, ...)\
    typedef ret (name##_t)(__VA_ARGS__);
LIST_OF_PLUGS
#undef PLUG

#endif //PLUG_H_

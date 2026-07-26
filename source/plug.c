#include "plug.h"
#include <assert.h>
#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VISUALISATION_NORMAL 1
#define VISUALISATION_LOG 0
typedef struct {
    float global_samples[CAPACITY];
    size_t CURRENT_SIZE;
    float MAX_SAMPLE;
    unsigned int global_channels;
    float complex fft_global_samples[CAPACITY];
    float fft_input[CAPACITY];
    float smoothed[CAPACITY];
    int visulalisation_type;
    Music music;
    Shader circle;
    float out_smear[CAPACITY];
    float ALPHA;
} Plug;

Plug *plug;

bool draw_in_terminal = false;

void fft(float in[], size_t stride, float complex out[], size_t n) {
    assert(n > 0);
    if (n == 1) {
	out[0] = in[0];
	return;
    }
    fft(in, stride * 2, out, n / 2);
    fft(in + stride, stride * 2, out + n / 2, n / 2);

    for (size_t k = 0; k < n / 2; ++k) {
	float t = (float)k / n;
	float complex v = cexp(-2 * I * PI * t) * out[k + n / 2];
	float complex e = out[k];
	out[k] = e + v;
	out[k + n / 2] = e - v;
    }
}

void draw_bar_in_terminal(unsigned int pos, float height) {
    if (height == 0.0f) {
	printf("\033[2C");
	return;
    }
    unsigned int max_height = 80;
    size_t h = ceilf(height * max_height);
    size_t to_move_down = max_height - h;

    printf("\033[%zuB", to_move_down);
    for (size_t i = 0; i < h; i++) {
	printf("/");
	printf("\033[D");
	printf("\033[B");
    }
    printf("\033[%zuA", h + to_move_down);
    printf("\033[2C");
}

void callback(void *bufferData, unsigned int frames) {
    if (frames > CAPACITY) {
	printf("callback: Received too big of a frame\n");
	return;
    }
    float(*fs)[2] = bufferData; // pointer to ararys of size 2 that have floats in them(confusing syntax , as always
    if (CAPACITY - plug->CURRENT_SIZE < frames) {
	memmove(plug->global_samples, plug->global_samples + frames, sizeof(float) * (CAPACITY - frames));
	plug->CURRENT_SIZE -= frames;
    }
    for (size_t i = 0; i < frames; i++) {
	plug->global_samples[plug->CURRENT_SIZE + i] = (fs[i][0] + fs[i][1]) / 2;
    }
    plug->CURRENT_SIZE += frames;
}
Plug *plug_pre_reload() {
    if (IsMusicValid(plug->music)) {
	DetachAudioStreamProcessor(plug->music.stream, callback);
    }
    return plug;
}

void plug_post_reload(void *state) {
    plug = (Plug *)state;
    if (IsMusicValid(plug->music)) {
	AttachAudioStreamProcessor(plug->music.stream, callback);
    }
    UnloadShader(plug->circle);
    plug->circle = LoadShader(NULL, "./shaders/circle.fs");
}

void plug_init(const char *filepath) {
    plug = malloc(sizeof(Plug));
    memset(plug, 0, sizeof(*plug));
    plug->circle = LoadShader(NULL, "./shaders/circle.fs");
    if (filepath != NULL) {

	plug->music = LoadMusicStream(filepath);
	printf("music.frameCount=%d\n", plug->music.frameCount);
	printf("music.stream.sampleRate=%u\n", plug->music.stream.sampleRate);
	printf("music.stream.sampleSize=%u\n", plug->music.stream.sampleSize);
	printf("music.stream.channels= %u \n", plug->music.stream.channels);
	PlayMusicStream(plug->music);
	plug->global_channels = plug->music.stream.channels;
	AttachAudioStreamProcessor(plug->music.stream, callback);
    }
    /*memset(plug->fft_plug->global_samples, 0, sizeof(plug->fft_plug->global_samples));*/

    /*memset(plug->smoothed, 0, sizeof(plug->smoothed));*/
}
float get_amp(float complex z, int vis_type) {
    //applies log on demand to make the visualisation logarithmic 'on demand'
    if (vis_type == VISUALISATION_NORMAL) {
	return cabsf(z);
    } else {

	float a = cabsf(z);
	return logf(a * a);
    }
}

void check_for_keys_and_files(){

    if (IsKeyPressed(KEY_SPACE)) {
	if (IsMusicStreamPlaying(plug->music)) {
	    PauseMusicStream(plug->music);
	} else {
	    ResumeMusicStream(plug->music);
	}
    } else if (IsKeyPressed(KEY_T)) {
	if (plug->visulalisation_type == VISUALISATION_NORMAL) {
	    plug->visulalisation_type = VISUALISATION_LOG;
	} else {
	    plug->visulalisation_type = VISUALISATION_NORMAL;
	}
    } else if (IsKeyPressed(KEY_A)) {
	if (draw_in_terminal) {
	    draw_in_terminal = false;
	} else {

	    draw_in_terminal = true;
	}
    }
    if (IsFileDropped()) {
	FilePathList droppedFiles = LoadDroppedFiles();
	if (droppedFiles.count > 0) {
	    const char *filepath = droppedFiles.paths[0];
	    StopMusicStream(plug->music);
	    UnloadMusicStream(plug->music);
	    plug->music = LoadMusicStream(filepath);
	    printf("music.frameCount=%d\n", plug->music.frameCount);
	    printf("music.stream.sampleRate=%u\n", plug->music.stream.sampleRate);
	    printf("music.stream.sampleSize=%u\n", plug->music.stream.sampleSize);
	    printf("music.stream.channels= %u \n", plug->music.stream.channels);
	    PlayMusicStream(plug->music);
	    plug->global_channels = plug->music.stream.channels;
	    AttachAudioStreamProcessor(plug->music.stream, callback);

	} else {
	    printf("Something went wrong when dropping files, received 0 files\n");
	    exit(-1);
	}
	UnloadDroppedFiles(droppedFiles);
    }
}
void hann_windowing(){

    for (size_t i = 0; i < CAPACITY; i++) { // applying hann windowing to get rid of phantom frequencies
	float t = (float)i / CAPACITY;
	float hann = 0.5f - 0.5f * cosf(2 * PI * t);
	plug->fft_input[i] = plug->fft_input[i] * hann;
    }
}
void plug_update() {
    plug->ALPHA = 0.3f; 
    plug->MAX_SAMPLE = 0.0f;
    int w = GetRenderWidth();
    int h = GetRenderHeight();
    float step = 1.06;
    size_t m = 0;
    UpdateMusicStream(plug->music);
    check_for_keys_and_files();

    memcpy(plug->fft_input, plug->global_samples,
           sizeof(plug->fft_input)); // this is done so the values of plug->global_samples do not get overwritten by the
                                     // callback(which runs in a different thread) before the fft is finished
    hann_windowing();
    fft(plug->fft_input, 1, plug->fft_global_samples, CAPACITY);
    //////////////////////////////////////////////////
    for (size_t i = 0; i < CAPACITY; i++) {
	float t = get_amp(plug->fft_global_samples[i], plug->visulalisation_type);
	if (t > plug->MAX_SAMPLE)
	    plug->MAX_SAMPLE = t; // Finding max_sample so i can normalise values later
    }
    //////////////////////////////////////////////////
    BeginDrawing();
    if (plug->MAX_SAMPLE == 0.0f) {
        //Something went wrong since max sample was 0, skipping drawing frame.
	ClearBackground(CLITERAL(Color){0x18, 0x18, 0x18, 0xFF});
	EndDrawing();
	return;
    }
    ClearBackground(CLITERAL(Color){0x18, 0x18, 0x18, 0xFF});
    /////////////////////////////////////////////////
    for (float f = 1.0f; (size_t)f < CAPACITY / 2; f = ceilf( f * step)) { 
        // here im applying ceiling since 1*1,06 is 1.06 and we are using it as index(int) so it gets
        // truncated to 1 , making it so im drawing the same freq over and over till it gets to 2
        // Finding how many values actually are there since im going for the 'f*=step' stride. also im only iterating
        // CAPACITY/2 because output of fft mirrors after n/2 samples 
	m += 1;
    } 
    
    float cell_width = (float)w / (m); 
                                       
    int s = m;
    m = 0;
    Texture2D texture = {rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8}; //preparing blank 'custom' texture to be drawn laater
    for (float f = 1.0f; (size_t)f < CAPACITY / 2; f = ceilf(f * step)) {                       // going until CAPACITY/2 because second half is a mirror of first half.
	float f1 = ceilf(f * step);
	float max = 0.0f;
	/////////////////////////////////////////////////////////////////////
	for (size_t q = (size_t)f; q < (CAPACITY / 2) && q < (size_t)f1; ++q) {

	    if (get_amp(plug->fft_global_samples[q], plug->visulalisation_type) > max)
		max = get_amp(plug->fft_global_samples[q],
		            plug->visulalisation_type); // getting max of the skiped values (values between f and f1) so they are not lost completely
	    

	} // This is done to turn it into a logarithmic visualisation(?), not sure why this works or how it works
	////////////////////////////////////////////////////////////////////
	float norm_value = max / plug->MAX_SAMPLE;

	/* if (t/plug->smoothed[m]>1.6f || t/plug->smoothed[m]<0.4f ) plug->alpha=0; */

	//printf("%f %f\n", plug->smoothed[m], plug->out_smear[m]);
	plug->smoothed[m] =
	    plug->smoothed[m] * (1-plug->ALPHA) + norm_value *  plug->ALPHA; // this is to smooth it out , since fft is being calculated like 60 times a second, the
                                                                             // output values go up and down around the center value and visually this renders
                                                                              // as flickering which is visually unappealing. to go around this we only use 30% of the new 
                                                                              // value and 70% of the old value, so that the value we visualise is closer to old value.    
                                                                              // this is called Exponential Moving Average(EMA)                                            

	//plug->out_smear[m] += (plug->smoothed[m] - plug->out_smear[m]) * 0.05f;  //constant here changes how fast the smear 'catcches up to' to the circle
          plug->out_smear[m] +=   plug->smoothed[m]*0.05f - plug->out_smear[m]*0.05f;
        // 8 , 8-0=8, 7,7-8=-1+8=10                                                                               
        float barH = plug->smoothed[m] * h / 2;  //getting bar height   
	if (barH > h / 2) { 
	    barH = h / 2;
	}

	float hue = (float)m / s; //m-th valu divided by the total number of values there are, to get the angle of the 'knob' in radian. this needs to be multiplied by 360*  to turn to degrees
	float saturation = 0.75f;
	float value = 1.0f;
	Color color = ColorFromHSV(hue * 360, saturation, value);
	float radius = 8 * cell_width * sqrtf(plug->smoothed[m]);  // doing sqrt to ease it , see resources. but basically sqr(x)
	float thickness =                                          // grows faster than x for 0<x<1;                             
	    (cell_width / 2) * sqrtf(plug->smoothed[m]); 
	                                                 
        //Drawing the line/colon
	Vector2 startPos = {.x = m * cell_width, .y = h / 2 - (int)barH};
	Vector2 endPos = {.x = m * cell_width, .y = h / 2};
	DrawLineEx(startPos, endPos, thickness, color);

	//Rectangle rec = {.x = startPos.x - radius, .y = startPos.y - radius, .width = 2 * radius, .height = 2 * radius };

        //position for the texture
	Vector2 texture_position = {
	    .x = startPos.x - radius,
	    .y = startPos.y - radius,
	};

        //drawing texture in shader mode
	BeginShaderMode(plug->circle);
	DrawTextureEx(texture, texture_position, 0, 2 * radius, color);
	EndShaderMode();

        
	float smear_offset = (plug->smoothed[m] - plug->out_smear[m]) * h / 2;
	Vector2 smear_position = {.x = startPos.x , .y = startPos.y + smear_offset};
        float smear_height=startPos.y-smear_position.y;
        Rectangle rect;
        //handling when smear height is negative , which means that the rect needs to be drawn on the other side of the y axis of the circle
        if (smear_height<0){
            smear_height*=-1;
            rect.x = startPos.x - cell_width/2;
            rect.y= startPos.y;//startPos.y,
            rect.width=cell_width;
            rect.height=smear_height;

        }else{
            rect.x = startPos.x - cell_width/2;
            rect.y= smear_position.y;//startPos.y,
            rect.width=cell_width;
            rect.height=smear_height;

        }
        char p[4]={0};
        snprintf(p,sizeof(p)," %zu ",m);
        if (m<17){
            printf("m:%zu circle pos: %f %f rect end pos: rect %f %f\n",m,smear_position.x,smear_position.y,rect.x+rect.height, rect.y+rect.height);

        } 
        DrawRectangleRec(rect, color);
        DrawText(p,  endPos.x, endPos.y, 1, WHITE);
	DrawCircleV(smear_position, cell_width, RED);
	if (draw_in_terminal)
	    draw_bar_in_terminal(m, plug->smoothed[m]);
	m++;
    }
    if (draw_in_terminal) {
	printf("\n");
	printf("\033[2J");
    }
    EndDrawing();
}

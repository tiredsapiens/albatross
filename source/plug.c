#include "plug.h" 
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <raylib.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    float  global_samples[CAPACITY];
    size_t CURRENT_SIZE;
    float MAX_SAMPLE;
    unsigned int global_channels;
    float complex fft_global_samples[CAPACITY];
    float temp[CAPACITY];
    Music music;
} Plug;

Plug* plug;
float  ALPHA;

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

void callback(void *bufferData, unsigned int frames) {
    if (frames>CAPACITY){
        printf("callback: Received too big of a frame\n");
        return;
    }
    float (*fs)[2] = bufferData;       //pointer to ararys of size 2 that have floats in them(confusing syntax , as always 
    if (CAPACITY-plug->CURRENT_SIZE<frames){
        memmove(plug->global_samples,plug->global_samples+frames,sizeof(float)*(CAPACITY-frames));
        plug->CURRENT_SIZE-=frames;
    }
    for (size_t i = 0; i <frames ; i++) {
        plug->global_samples[plug->CURRENT_SIZE+i]=(fs[i][0]+fs[i][1])/2;
    }
    plug->CURRENT_SIZE+=frames;

}
Plug* plug_pre_reload(){
    DetachAudioStreamProcessor(plug->music.stream,callback);
    return plug;
}

void plug_post_reload(void* state){
    plug=(Plug*)plug;
    AttachAudioStreamProcessor(plug->music.stream, callback);

}

void plug_init(const char* filepath){
    plug=malloc(sizeof(Plug));
    memset(plug,0,sizeof(*plug));
    if (filepath!=NULL){

    printf("inside\n");
    plug->music = LoadMusicStream(filepath);
    printf("music.frameCount=%d\n", plug->music.frameCount);
    printf("music.stream.sampleRate=%u\n", plug->music.stream.sampleRate);
    printf("music.stream.sampleSize=%u\n", plug->music.stream.sampleSize);
    printf("music.stream.channels= %u \n", plug->music.stream.channels);
    PlayMusicStream(plug->music);
    plug->global_channels = plug->music.stream.channels;
    AttachAudioStreamProcessor(plug->music.stream, callback);
    }
    printf("outside\n");
    /*memset(plug->fft_plug->global_samples, 0, sizeof(plug->fft_plug->global_samples));*/

    /*memset(plug->smoothed, 0, sizeof(plug->smoothed));*/
}

void plug_update(){
        ALPHA=0.5f;
        plug->MAX_SAMPLE = 0.0f;
	int w = GetRenderWidth();
	int h = GetRenderHeight();
        float step=1.09;
        size_t m=0;
	UpdateMusicStream(plug->music);
	if (IsKeyPressed(KEY_SPACE)) {
	    if (IsMusicStreamPlaying(plug->music)) {
		PauseMusicStream(plug->music);
	    } else {
		ResumeMusicStream(plug->music);
	    }
	}
        if (IsFileDropped()){
            FilePathList droppedFiles=LoadDroppedFiles();
            if (droppedFiles.count>0){
                const char* filepath=droppedFiles.paths[0];
                StopMusicStream(plug->music);
                UnloadMusicStream(plug->music);
                plug->music=LoadMusicStream(filepath);
                printf("music.frameCount=%d\n", plug->music.frameCount);
                printf("music.stream.sampleRate=%u\n", plug->music.stream.sampleRate);
                printf("music.stream.sampleSize=%u\n", plug->music.stream.sampleSize);
                printf("music.stream.channels= %u \n", plug->music.stream.channels);
                PlayMusicStream(plug->music);
                plug->global_channels = plug->music.stream.channels;
                AttachAudioStreamProcessor(plug->music.stream, callback);


            }else{
                printf("Something went wrong when dropping files, received 0 files\n");
                exit(-1);
            }
            UnloadDroppedFiles(droppedFiles);

        }
        memcpy(plug->temp, plug->global_samples, sizeof(plug->temp));             //this is done so the values of plug->global_samples do not get overwritten by the callback(which runs in a different thread) before the fft is finished
        fft(plug->temp, 1, plug->fft_global_samples,  CAPACITY);
        //////////////////////////////////////////////////
        for (size_t i = 0; i < CAPACITY; i++) {
            float t = cabsf(plug->fft_global_samples[i]);
            if (t > plug->MAX_SAMPLE) plug->MAX_SAMPLE = t;                                     // Finding max_sample so i can normalise values later
        }
        //////////////////////////////////////////////////
	if (plug->MAX_SAMPLE == 0.0f) {
            printf("MAX_SAMPLE was %f skipping drawing\n",plug->MAX_SAMPLE);
            EndDrawing(); 
            return; 
        }
	BeginDrawing();
	ClearBackground(CLITERAL(Color){0x18, 0x18, 0x18, 0xFF});
        /////////////////////////////////////////////////
        for (float f = 20.0f; (size_t)f <CAPACITY/2 ; f*=step) {
            m+=1;
        }                                                                           // Finding how many values actually are there since im going for the 'f*=step' stride. also im only iterating CAPACITY/2 because output of fft mirrors after n/2 samples
	float cell_width = (float)w / (m);                                          // The second half holds no meaningful/new information.
	int width = (int)cell_width;
        /////////////////////////////////////////////////
        m=0;
        memset(plug->temp,0,sizeof(plug->temp));
        for (float f = 20.0f; (size_t)f <CAPACITY/2 ; f*=step) {

            float f1=f*step;
            float a=0.0f;
            /////////////////////////////////////////////////////////////////////
            for (size_t q= (size_t) f; q <(CAPACITY/2) && q<(size_t)f1 ; ++q) {
                    a+= cabs(plug->fft_global_samples[q]);                          //Averaging the skiped values (values between f and f1) so they are not lost completely
            }                                                                       //This is done to turn it into a logarithmic visualisation(?), not sure why this works or how it works
            a=a/((size_t)f1 - (size_t)f +1);
            ////////////////////////////////////////////////////////////////////
	    float t = a / plug->MAX_SAMPLE;

            /* if (t/plug->smoothed[m]>1.6f || t/plug->smoothed[m]<0.4f ) ALPHA=0; */
            
            plug->temp[m] = plug->temp[m] * ALPHA + t* (1-ALPHA);           // this is to smooth it out , since fft is being calculated like 60 times a second, the output values go up and down around the center value and visually this renders
            float barH = plug->temp[m] * h / 2;                                 // as flickering which is visually unappealing. to go around this we only use 30% of the new value and 70% of the old value, so that the value we visualise is closer to old value.
	    if (barH > h / 2) {                                                     // this is called Exponential Moving Average(EMA)
		barH = h / 2;
	    }
	    DrawRectangle(m * cell_width, h / 2 - (int)barH, width, barH, BLUE);
            m++;
	}
	EndDrawing();
}


#include "plug.h" 
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <raylib.h>
#include <assert.h>
#include <string.h>

typedef struct {
    float left;
    float right;
}Frame;
size_t CURRENT_SIZE=0;
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
    Frame *frame_data = bufferData;
    if (CAPACITY-CURRENT_SIZE<frames){
        memmove(global_samples,global_samples+frames,sizeof(float)*(CAPACITY-frames));
        CURRENT_SIZE-=frames;
    }
    for (size_t i = 0; i <frames ; i++) {
        global_samples[CURRENT_SIZE+i]=(frame_data[i].left+frame_data[i].right)/2;
    }
    CURRENT_SIZE+=frames;

}
void plug_pre_reload(Plug* plug){
    DetachAudioStreamProcessor(plug->music.stream,callback);
}

void plug_post_reload(Plug* plug){
    AttachAudioStreamProcessor(plug->music.stream, callback);
}

void plug_init(Plug *plug,const char* filepath){
    plug->music = LoadMusicStream(filepath);
    printf("music.frameCount=%d\n", plug->music.frameCount);
    printf("music.stream.sampleRate=%u\n", plug->music.stream.sampleRate);
    printf("music.stream.sampleSize=%u\n", plug->music.stream.sampleSize);
    printf("music.stream.channels= %u \n", plug->music.stream.channels);
    PlayMusicStream(plug->music);
    memset(plug->fft_global_samples, 0, sizeof(plug->fft_global_samples));
    plug->global_channels = plug->music.stream.channels;
    AttachAudioStreamProcessor(plug->music.stream, callback);
}

void plug_update(Plug* plug){
	UpdateMusicStream(plug->music);
	if (IsKeyPressed(KEY_SPACE)) {
	    if (IsMusicStreamPlaying(plug->music)) {
		PauseMusicStream(plug->music);
	    } else {
		ResumeMusicStream(plug->music);
	    }
	}
        fft(global_samples, 1, plug->fft_global_samples,  CAPACITY);
        MAX_SAMPLE = 0.0f;
        for (size_t i = 0; i < CAPACITY; i++) {
            float t = cabsf(plug->fft_global_samples[i]);
            if (t > MAX_SAMPLE) MAX_SAMPLE = t;
        }
	BeginDrawing();
	ClearBackground(CLITERAL(Color){0x18, 0x18, 0x18, 0xFF});
	int w = GetRenderWidth();
	int h = GetRenderHeight();
        float step=1.09;
        size_t m=0;
        for (float f = 20.0f; (size_t)f <CAPACITY/2 ; f*=step) {
            m+=1;
        }
	float cell_width = (float)w / (m);
	int width = (int)cell_width;
        
	if (MAX_SAMPLE == 0.0f) {EndDrawing(); return; }
        m=0;
        for (float f = 20.0f; (size_t)f <CAPACITY/2 ; f*=step) {

	    float sample =  cabs(plug-> fft_global_samples[(size_t)f]);
	    float t = sample / MAX_SAMPLE;
	    float barH = t * h / 2;
            float f1=f*step;
            float a=0.0f;
            for (size_t q= (size_t) f; q <(CAPACITY/2) && q<(size_t)f1 ; ++q) {
                    a+= cabs(plug->fft_global_samples[q]);
            }

            a=a/((size_t)f1 - (size_t)f +1);
	    t = a / MAX_SAMPLE;
	    barH = t * h / 2;
	    if (barH > h / 2) {
		barH = h / 2;
	    }
	    DrawRectangle((int)(m * cell_width + 1), h / 2 - (int)barH, width, barH, RED);
            m++;
	}
	EndDrawing();
}


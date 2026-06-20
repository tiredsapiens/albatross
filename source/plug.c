#include "plug.h" 
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <raylib.h>
#include <assert.h>


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
    if (frames<CAPACITY){
        fprintf(stderr,"Received too small of a frame %d , skipping ...",frames);
        return;
    }
    float *float_data = (float *)bufferData;
    for (size_t i = 0; i < CAPACITY; i++) {
	left_global_samples[i] = float_data[2 * i];
	right_global_samples[i] = float_data[2 * i + 1];
    }
}

void plug_init(Plug *plug,const char* filepath){
    plug->music = LoadMusicStream(filepath);
    PlayMusicStream(plug->music);
    printf("music.frameCount=%d\n", plug->music.frameCount);
    printf("music.stream.sampleRate=%u\n", plug->music.stream.sampleRate);
    printf("music.stream.sampleSize=%u\n", plug->music.stream.sampleSize);
    printf("music.stream.channels= %u \n", plug->music.stream.channels);
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
        fft(left_global_samples, 1, plug-> fft_left_global_samples,  CAPACITY);
        fft(right_global_samples, 1, plug->fft_right_global_samples, CAPACITY);
        MAX_SAMPLE = 0;
        for (size_t i = 0; i < CAPACITY; i++) {
	float t = cabsf(plug->fft_left_global_samples[i]);
	if (t > MAX_SAMPLE)
	    MAX_SAMPLE = t;
	left_global_samples[i] = t;
	t = cabsf(plug->fft_right_global_samples[i]);
	if (t > MAX_SAMPLE)
	    MAX_SAMPLE = t;
	right_global_samples[i] = t;
    }
	BeginDrawing();
	ClearBackground(CLITERAL(Color){0x18, 0x18, 0x18, 0xFF});
	int w = GetRenderWidth();
	int h = GetRenderHeight();
	float cell_width = (float)w / (CAPACITY / 2);
	int width = (int)cell_width;
	if (width < 1)
	    width = 1;
	for (size_t i = 0; i < CAPACITY / 2; ++i) {
	    float left_sample =  left_global_samples[i];
	    float right_sample = right_global_samples[i];
	    float t = left_sample / MAX_SAMPLE;
	    float barH = t * h / 2;
	    if (barH > h / 2) {
		barH = h / 2;
	    }
	    DrawRectangle((int)(i * cell_width), h - (int)barH, width, barH, RED);
	    t = right_sample / MAX_SAMPLE;
	    barH = t * h / 2;
	    if (barH > h / 2) {
		barH = h / 2;
	    }
	    DrawRectangle((int)(i * cell_width + 1), h / 2 - (int)barH, width, barH, BLUE);
	}
	EndDrawing();
}


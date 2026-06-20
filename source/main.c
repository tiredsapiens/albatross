#include <assert.h>
#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

float pi;

typedef struct {
  float left;
  float right;
} Frame;

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])
#define CAPACITY 512
float MAX_SAMPLE=0;
float complex left_global_samples[CAPACITY] = {};
float complex right_global_samples[CAPACITY] = {};
static unsigned int global_channels = 0;

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
    float complex v = cexp(-2 * I * pi * t) * out[k + n / 2];
    float complex e = out[k];
    out[k] = e + v;
    out[k + n / 2] = e - v;
  }
}

void callback(void *bufferData, unsigned int frames) {
    
  float *float_data = (float *)bufferData;
  float left_channel[frames] = {};
  float right_channel[frames] = {};
  for (size_t i = 0; i < frames; i++) {
    left_channel[i] = float_data[2 * i];
    right_channel[i] = float_data[2 * i + 1];
  }
  fft(left_channel, 1, left_global_samples, frames);
  fft(right_channel,1,right_global_samples,frames);

  MAX_SAMPLE=0;
  for (size_t i = 0; i <frames ; i++) {
        float t =cabsf(left_global_samples[i]);
        if (t>MAX_SAMPLE) MAX_SAMPLE=t;
        left_global_samples[i]=t;
        t =cabsf(right_global_samples[i]);
        if (t>MAX_SAMPLE) MAX_SAMPLE=t;
        right_global_samples[i]=t;
  }
}

int main() {
  pi = atan2f(1, 1) * 4;
  InitWindow(800, 600, "Albatross");
  SetTargetFPS(60);
  InitAudioDevice();
  Music music = LoadMusicStream("audio/tower_of_dreams.mp3");
  PlayMusicStream(music);
  printf("music.frameCount=%d\n", music.frameCount);
  printf("music.stream.sampleRate=%u\n", music.stream.sampleRate);
  printf("music.stream.sampleSize=%u\n", music.stream.sampleSize);
  printf("music.stream.channels= %u \n", music.stream.channels);
  global_channels = music.stream.channels;
  AttachAudioStreamProcessor(music.stream, callback);
  while (!WindowShouldClose()) {
    UpdateMusicStream(music);
    if (IsKeyPressed(KEY_SPACE)) {
      if (IsMusicStreamPlaying(music)) {
	PauseMusicStream(music);
      } else {
	ResumeMusicStream(music);
      }
    }
    BeginDrawing();
    ClearBackground(CLITERAL(Color){0x18, 0x18, 0x18, 0xFF});
    int w = GetRenderWidth();
    int h = GetRenderHeight();
    float cell_width = (float)w / (CAPACITY/2);
    int width = (int)cell_width;
    if (width < 1)
      width = 1;
    //printf("MAX SAMPLE for this batch is %4.4f\n",MAX_SAMPLE);
    for (size_t i = 0; i < CAPACITY/2; ++i) {
      int y = h / 2;
      float left_sample = left_global_samples[i];
      float right_sample=right_global_samples[i];
      if (left_sample>MAX_SAMPLE){

          printf("left_sample is %f while max_sample is",left_sample,MAX_SAMPLE);
      }
      float t=left_sample/MAX_SAMPLE;
      float barH = t* h / 2;

      if (barH < 0) {
	 y += barH;
	 barH = -barH;
       }else if(barH>h/2){
        barH=h/2;
       }
      DrawRectangle((int)(i * cell_width), h-(int)barH, width, barH, RED);

      if (right_sample>MAX_SAMPLE){

          printf("left_sample is %f while max_sample is",right_sample,MAX_SAMPLE);
      }
      t=right_sample/MAX_SAMPLE;  
      barH = t * h / 2;

      if (barH < 0) {
	 y += barH;
	 barH = -barH;
       }else if(barH>h/2){
        barH=h/2;
       }

       DrawRectangle((int)(i * cell_width+1), h/2-(int)barH, width, barH, BLUE);
    }

    EndDrawing();
  }
  return 0;
}

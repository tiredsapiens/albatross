#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])
#define CAPACITY 4080

float global_frames[CAPACITY] = {0};
size_t global_frames_count = 0;
static unsigned int global_channels = 0;

void callback(void *bufferData, unsigned int frames) {
  /* frames =512 means there are 512 frames for every channel
   * to get total frames/samples you need to do frames*channels*/
  unsigned int samples = frames * global_channels;
  if (CAPACITY - global_frames_count >= samples) {
    memcpy(global_frames + global_frames_count, bufferData,
           samples * sizeof(float));
    global_frames_count += samples;
  } else if (samples <= CAPACITY) {
    memmove(global_frames, global_frames + samples,
            sizeof(float) * (CAPACITY - samples));
    memcpy(global_frames + (CAPACITY - samples), bufferData,
           sizeof(float) * samples);
  } else {
    memcpy(global_frames, bufferData, sizeof(float) * CAPACITY);
    global_frames_count = CAPACITY;
  }
  /* ////////////////////////////////////////////////////////////////// */
  /*  if (frames > ARRAY_LEN(global_frames) / global_channels) { */
  /*    frames = ARRAY_LEN(global_frames) / global_channels; */
  /*  } */
  /*  global_frames_count = frames *  global_channels; */
  /*  memcpy(global_frames, bufferData, global_frames_count*sizeof(float)); */
}

int main() {
  InitWindow(2000, 600, "Albatross");
  SetTargetFPS(60);
  InitAudioDevice();
  Music music = LoadMusicStream("celestial_symphony.mp3");
  PlayMusicStream(music);
  /* SetMusicVolume(music, 0.5f); */
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
    float cell_width = (float)w / global_frames_count;
    int width = (int)cell_width;
    if (width < 1)
      width = 1;
    for (size_t i = 0; i < global_frames_count; ++i) {
      int y = h / 2;
      /* int y = h / 2; */
      float sample = global_frames[i];
      int barH = (int)(sample * h / 2);
      if (barH < 0) {
        y += barH;
        barH = -barH;
      }
      DrawRectangle((int)(i * cell_width), y, width, barH, RED);
    }
    EndDrawing();
  }
  return 0;
}

#include "SDL.h"
#include "fftw3.h"
#include "ConstantQ.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

const int SAMPLES_PER_FRAME = 2048;
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 720;
const int numx = 120;
const int numy = 1;
double speed = 5.0;

const int MAX_RECORDING_SECONDS = 1000;
const int RECORDING_BUFFER_SECONDS = MAX_RECORDING_SECONDS + 1;

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;

double RGBValues[numx][numy][3];
SDL_Rect PixelArray[numx][numy];

Sint16* gRecordingBuffer = NULL;
Uint32 gBufferBytePosition = 0;

int width, height;

float note_frequencies[120];

void init_frequencies()
{
    for (int i = 0; i < 120; i++)
    {
        note_frequencies[i] = 32.70319566f * pow(2, i/12.0f);
    }
}
float result[120];
bool test = false;
void note_transform(Sint16* samples, float* result)
{
    float sin_sum;
    float cos_sum;
    for (int i = 0; i < 120; i++)
    {
        result[i] = 0;
        sin_sum = 0;
        cos_sum = 0;
        for (int j = 0; j < SAMPLES_PER_FRAME; j++)
        {
            sin_sum += samples[j] * sin(2.0f*M_PI*note_frequencies[i]*j/44100.0f);
            cos_sum += samples[j] * cos(2.0f*M_PI*note_frequencies[i]*j/44100.0f);
        }
        result[i] = sqrt(pow(sin_sum, 2) + pow(cos_sum, 2));
    }
}

void fftw_callback( void* userdata, Uint8* stream, int bytes )
{
    Sint16* real_stream = (Sint16*)stream;
    int length = bytes/2;

    double in_stream[length];

    fftw_complex* f_transform = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (length/2 + 1));
    fftw_plan p = fftw_plan_dft_r2c_1d(length, in_stream, f_transform, FFTW_ESTIMATE);

    std::copy(real_stream, real_stream+length, in_stream);
    fftw_execute(p);

    for (int i = 0; i < numx; ++i)
        {
            for (int j = 0; j < numy; ++j)
            {
                for (int k = 0; k < 3; ++k)
                {
                    RGBValues[i][j][0] = std::min((int)(pow(pow(f_transform[i][0], 2) + pow(f_transform[i][1], 2), 0.5)/5000), 255);
                }
                SDL_FillRect( gScreenSurface, &PixelArray[i][j], SDL_MapRGB( gScreenSurface->format, RGBValues[i][j][0], RGBValues[i][j][1], RGBValues[i][j][2] ) );
            }
        }
    SDL_UpdateWindowSurface( gWindow );
}

void audioRecordingCallback( void* userdata, Uint8* stream, int bytes )
{
    Sint16* real_stream = (Sint16*)stream;
    int length = bytes/2;

    float* transformed_output = (float*) malloc(sizeof(float) * 120);

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    note_transform(real_stream, transformed_output);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    std::cout << time_span.count() << ", ";
    for (int i = 0; i < numx; ++i)
    {
        for (int j = 0; j < numy; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                RGBValues[i][j][0] = std::min(255.0f, transformed_output[i]/20000);
            }
            SDL_FillRect( gScreenSurface, &PixelArray[i][j], SDL_MapRGB( gScreenSurface->format, RGBValues[i][j][0], RGBValues[i][j][1], RGBValues[i][j][2] ) );
        }
    }
    SDL_UpdateWindowSurface( gWindow );
}

void init()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

    //Initialise note frequencies
    init_frequencies();

    //Initialise colour arrays
    for (int i = 0; i < numx; ++i)
    {
        for (int j = 0; j < numy; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                RGBValues[i][j][k] = 0;
            }
            PixelArray[i][j].x = SCREEN_WIDTH * i/numx;
            PixelArray[i][j].y = SCREEN_HEIGHT * j/numy;
            PixelArray[i][j].h = SCREEN_HEIGHT / numy;
            PixelArray[i][j].w = SCREEN_WIDTH / numx;
        }
    }

    //Initialise window
    gWindow = SDL_CreateWindow( "Bolg!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    gScreenSurface = SDL_GetWindowSurface( gWindow );

    //Initialise audio

    SDL_AudioSpec desiredRecordingSpec;
    SDL_zero(desiredRecordingSpec);
    SDL_AudioSpec gReceivedRecordingSpec;
    desiredRecordingSpec.freq = 44100;
    desiredRecordingSpec.format = AUDIO_S16;
    desiredRecordingSpec.channels = 1;
    desiredRecordingSpec.samples = SAMPLES_PER_FRAME;
    desiredRecordingSpec.callback = fftw_callback;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), SDL_TRUE, &desiredRecordingSpec, &gReceivedRecordingSpec, 0);
    std::cout << gReceivedRecordingSpec.freq << std::endl;

    int bytesPerSample = gReceivedRecordingSpec.channels * ( SDL_AUDIO_BITSIZE( gReceivedRecordingSpec.format ) / 8 );
    int bytesPerSecond = gReceivedRecordingSpec.freq * bytesPerSample;
    int gBufferByteSize = RECORDING_BUFFER_SECONDS * bytesPerSecond;

    gRecordingBuffer = new Sint16[ gBufferByteSize ];
    memset( gRecordingBuffer, 0, gBufferByteSize );

    gBufferBytePosition = 0;
    SDL_PauseAudioDevice(dev, 0);
}

void close()
{
    SDL_CloseAudioDevice(2);
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    SDL_Quit();
}

int main( int argc, char* args[] )
{
    init();

    //Main event loop
    bool quit = false;
    SDL_Event e;

    while( !quit )
    {
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
        }
    }
    close();
    return 0;
}
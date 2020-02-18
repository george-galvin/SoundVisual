#include "SDL.h"
#include <windows.h>
#include <iostream>

const int SAMPLES_PER_FRAME = 2048;
double speed = 5.0;

int data_count = 0;
const int RECORDING_BUFFER_SECONDS = 100000;

Sint16* gRecordingBuffer = NULL;
Uint32 gBufferBytePosition = 0;

DCB serialParams;
DWORD bytessent;
byte data[1];
HANDLE arduino;

void one_byte_callback( void* userdata, Uint8* stream, int bytes )
{
    data_count++;
    Sint16* real_stream = (Sint16*)stream;
    int length = bytes/2;
    int mod_sum = 0;

    for (int i = 0; i < length; i++)
    {
        mod_sum += abs(real_stream[i]);
    }
    data[0] = ((mod_sum / (8*length)) + data_count) % 255;
    WriteFile(arduino, &data, 1, &bytessent, 0);
}

void init()
{
    SDL_Init(SDL_INIT_AUDIO);

    //Initialise arduino
    arduino = CreateFile("/COM5", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    serialParams.BaudRate = CBR_19200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;

    SetCommState(arduino, &serialParams);

    //Initialise audio

    SDL_AudioSpec desiredRecordingSpec;
    SDL_zero(desiredRecordingSpec);
    SDL_AudioSpec gReceivedRecordingSpec;
    desiredRecordingSpec.freq = 44100;
    desiredRecordingSpec.format = AUDIO_S16;
    desiredRecordingSpec.channels = 1;
    desiredRecordingSpec.samples = SAMPLES_PER_FRAME;
    desiredRecordingSpec.callback = one_byte_callback;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, 1), SDL_TRUE, &desiredRecordingSpec, &gReceivedRecordingSpec, 0);

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
            if( e.type == SDL_KEYDOWN )
            {
                quit = true;
            }
        }
    }
    close();
    return 0;
}

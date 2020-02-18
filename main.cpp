/* Simple LED Music Visualiser - PC side.
 * Takes input from Stereo Mix (using SDL), a feature on most old Windows computers that lets you work with
 * the output audio going to the speakers in real-time. Turns it into data to be sent over the 
 * serial port to an Arduino, to control the colours of an LED array. 
 */

#include "SDL.h" //to get input audio
#include <windows.h> //to output data to serial port

const int SAMPLES_PER_FRAME = 2048; 
const int SAMPLE_FREQ = 44100; //i.e. about 22 transmissions per second

int data_count = 0; //total number of bytes sent
const int RECORDING_BUFFER_SECONDS = 100000; //Necessary argument to the SDL audio function

//Necessary variables for the SDL function3
Sint16* gRecordingBuffer = NULL;
Uint32 gBufferBytePosition = 0;

//Necessary variables for the serial port function
DCB serialParams;
DWORD bytessent;
byte data[1];
HANDLE arduino;

void one_byte_callback( void* userdata, Uint8* stream, int bytes )
{
//The callback function, executed each time a frame's worth of new samples 
// comes in from Stereo Mix.

    data_count++;
    Sint16* real_stream = (Sint16*)stream; //turn data back to 16-bit form
    int length = bytes/2;
    int mod_sum = 0;

    for (int i = 0; i < length; i++) //get the average amplitude of the sound wave
    {
        mod_sum += abs(real_stream[i]);
    }

    data[0] = ((mod_sum / (8*length)) + data_count) % 255; 
    //Turn this into a byte, representing a saturated colour on the HSV wheel.
    //The data_count variable is added to continually change the colour scale of the array
    //to make it interesting. The (8*length) part can be scaled to make the colour changes more
    //or less subtle.

    WriteFile(arduino, &data, 1, &bytessent, 0); //Send this into 
}

void init()
{
    //Initialise SDL
    SDL_Init(SDL_INIT_AUDIO); 

    //Initialise arduino (the virtual port name "/COM5" is different on different machines)
    arduino = CreateFile("/COM5", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    serialParams.BaudRate = CBR_19200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;

    SetCommState(arduino, &serialParams);

    //Initialise connection to Stereo Mix
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
            if( e.type == SDL_KEYDOWN ) //Wait for key down on console
            {
                quit = true;
            }
        }
    }
    close();
    return 0;
}
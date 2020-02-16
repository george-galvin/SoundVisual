/* AUDIO VISUALISER

This program uses a feature of most older Windows computers called Stereo Mix, which records all the audio
output from the computer and allows it to be read in by a program in real time. My program turns this into
a visual display of which frequencies are being played - like a reverse piano. The eventual goal is to 
send the output to an Arduino board connected to a LED array, to create a physical audio visualiser.
*/

#include "SDL.h" //for the graphical output 
#include "fftw3.h" //for time-to-frequency conversion
#include <iostream> //for logging output (during testing)
#include <cmath> //for pi
#include <chrono> //for measuring execution time (during testing)

using namespace std::chrono;

const int SAMPLES_PER_FRAME = 2048;
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 720;
const int numx = 120;
double speed = 5.0;

const int MAX_RECORDING_SECONDS = 1000;
const int RECORDING_BUFFER_SECONDS = MAX_RECORDING_SECONDS + 1;

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;

double RGBValues[numx][3];
SDL_Rect PixelArray[numx];

Sint16* gRecordingBuffer = NULL;
Uint32 gBufferBytePosition = 0;


void fftw_callback( void* userdata, Uint8* stream, int bytes )
{
    //This is the 'callback' function, which executes every time the stream has SAMPLES_PER_FRAME
	//new audio samples to parse. Data is converted into the right 16-bit format, passed through a
	//fast Fourier transform to convert to the frequency domain, and then plotted on the screen.


    Sint16* real_stream = (Sint16*)stream; //Convert into correct format
    int length = bytes/2;

    double in_stream[length]; //The fast fourier transform
    fftw_complex* f_transform = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (length/2 + 1));
    fftw_plan p = fftw_plan_dft_r2c_1d(length, in_stream, f_transform, FFTW_ESTIMATE);
    std::copy(real_stream, real_stream+length, in_stream);
    fftw_execute(p);

    for (int i = 0; i < numx; ++i) //Update the screen data with the magnitude of the transform at each frequency bin.
    {
	//Right now the screen just goes red at the high amplitude frequencies - this is the base for more complex algorithms in future.
        RGBValues[i][0] = std::min((int)(pow(pow(f_transform[i][0], 2) + pow(f_transform[i][1], 2), 0.5)/5000), 255);
        SDL_FillRect( gScreenSurface, &PixelArray[i], SDL_MapRGB( gScreenSurface->format, RGBValues[i][0], RGBValues[i][1], RGBValues[i][2] ) );
    }
    SDL_UpdateWindowSurface( gWindow ); //Refresh the screen
}

void init()
{
    //Initialise SDL2
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

    //Initialise note frequencies
    init_frequencies();

    //Initialise colour arrays
    for (int i = 0; i < numx; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            RGBValues[i][k] = 0;
        }
        PixelArray[i].x = SCREEN_WIDTH * i/numx;
        PixelArray[i].y = 0;
        PixelArray[i].h = SCREEN_HEIGHT;
        PixelArray[i].w = SCREEN_WIDTH / numx;
    }

    //Initialise window
    gWindow = SDL_CreateWindow( "Sound Visualiser", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    gScreenSurface = SDL_GetWindowSurface( gWindow );

    //Initialise connection to Stereo Mix, and sets up my 'fftw_callback' function to receive data from it.

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
    SDL_PauseAudioDevice(dev, 0); //Using this function with argument 0 means 'play'.
}

void close()
{
    //Closes audio & window properly.
    SDL_CloseAudioDevice(2);
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    SDL_Quit();
}

int main( int argc, char* args[] )
{
    //	Main event loop thread - initialises program, waits for the user to quit, and exits. 

    init();
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
/* Simple LED Music Visualiser - PC side.
 * Takes input from a microphone / recording device using SDL2, and turns it into data to either control a display on screen,
 * or the colours of an LED array. The idea is to run it with Stereo Mix, a device on most old Windows computers 
 * that lets you work with the output audio going to the speakers - so you can listen to music with visual accompaniment. 
 */

#define NOMINMAX //stops windows.h from defining its own conflicting max and min functions

#include "SDL.h"
#include "fftw3.h"
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

enum class Mode { NOT_SELECTED, PC_MODE, LED_MODE };
Mode current_mode = Mode::NOT_SELECTED;

//Variables for both modes

const int SAMPLES_PER_FRAME = 2048;
const int SAMPLE_FREQ = 44100; //i.e. about 22 transmissions per second
SDL_AudioDeviceID dev;

//Variables for PC mode

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 720;
const int num_keys = 120;
fftw_complex* f_transform;
SDL_Window* window = NULL;
SDL_Surface* screen_surface = NULL;
SDL_Rect pixel_array[num_keys];

//Variables for LED mode

int data_count = 0; //total number of bytes sent
DCB serial_params;
DWORD bytes_sent;
byte output_data[1];
HANDLE arduino;

//A 'callback' function executes every time SDL has the required length of new audio data
// to pass on.
void pc_callback(void* userdata, Uint8* stream, int bytes)
{
    Sint16* real_stream = (Sint16*)stream; //turn data back to 16-bit form
    int length = bytes / 2;
    double amplitude;
    double in_stream[SAMPLES_PER_FRAME];

    //Do a fast fourier transform on the input data, storing results in f_transform
    copy(real_stream, real_stream + length, in_stream);
    fftw_plan p = fftw_plan_dft_r2c_1d(length, in_stream, f_transform, FFTW_ESTIMATE);
    fftw_execute(p); //Do a fast fourier transform 

    //Pass the result to the array of rectangles displayed on the screen.
    for (int i = 0; i < num_keys; ++i)
    {
        amplitude = min((int)(pow(pow(f_transform[i][0], 2) + pow(f_transform[i][1], 2), 0.5) / 5000), 255);
        SDL_FillRect(screen_surface, &pixel_array[i], SDL_MapRGB(screen_surface->format, 0, 0, amplitude));
    }
    SDL_UpdateWindowSurface(window);
}

void led_callback(void* userdata, Uint8* stream, int bytes)
{
    data_count++;
    Sint16* real_stream = (Sint16*)stream; //turn data back to 16-bit form
    int length = bytes / 2;
    int mod_sum = 0;

    for (int i = 0; i < length; i++) //get the average amplitude of the sound wave
    {
        mod_sum += abs(real_stream[i]);
    }

    output_data[0] = ((mod_sum / (8 * length)) + data_count) % 255;
    //Turn this into a byte, representing a saturated colour on the HSV wheel.
    //The data_count variable is added to continually change the colour scale of the array
    //to make it interesting. The (8*length) part can be scaled to make the colour changes more
    //or less subtle.

    WriteFile(arduino, &output_data, 1, &bytes_sent, 0); //Send the data to the Arduino
}

void init_pc()
{
    //Initialise colour arrays
    for (int i = 0; i < num_keys; ++i)
    {
        pixel_array[i].x = SCREEN_WIDTH * i / num_keys;
        pixel_array[i].y = 0;
        pixel_array[i].h = SCREEN_HEIGHT / 1;
        pixel_array[i].w = SCREEN_WIDTH / num_keys;
    }

    //Initialise window
    window = SDL_CreateWindow("Audio Visualiser", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    screen_surface = SDL_GetWindowSurface(window);

    f_transform = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (SAMPLES_PER_FRAME / 2 + 1));
}

void init_leds()
{
    //Initialise arduin. The virtual port name "COM3" is different on different machines and must be changed accordingly.
    arduino = CreateFileA("COM3", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    serial_params.BaudRate = CBR_19200;
    serial_params.ByteSize = 8;
    serial_params.StopBits = ONESTOPBIT;
    serial_params.Parity = NOPARITY;

    SetCommState(arduino, &serial_params);
}

void init()
{
    //Start SDL
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
   
    //Get user input on correct input device
    int num_devices = (int)SDL_GetNumAudioDevices(1);

    cout << "Select an audio device: " << endl << endl;;
    for (int i = 0; i < (int)num_devices; i++)
    {
        cout << i + 1 << ": " << SDL_GetAudioDeviceName(i, 1) << endl;
    }
    cout << endl;

    string user_input_string;
    int user_input_int = 0;
    while ((user_input_int < 1) | (user_input_int > num_devices)) //Loop until a valid device number is entered
    {
        cin >> user_input_string;
        user_input_int = atoi(user_input_string.c_str()); //This function conveniently returns 0 when input is not an integer
    }
    int selected_input = user_input_int - 1;

    //Get user input on whether to run in PC/LED mode
    cout << endl << "Select an output device: " << endl;
    cout << "1: PC Screen" << endl;
    cout << "2: LED Lights" << endl << endl;

    user_input_string = "";
    while (!(user_input_string == "1" | user_input_string == "2"))
    {
        cin >> user_input_string;
    }
    cout << endl;

    //Start specified output mode
    if (user_input_string == "1")
    {
        current_mode = Mode::PC_MODE;
        init_pc();
    }
    else
    {
        current_mode = Mode::LED_MODE;
        init_leds();
    }

    //Initialise connection to recording device
    SDL_AudioSpec desiredRecordingSpec;
    SDL_AudioSpec receivedRecordingSpec;

    SDL_zero(desiredRecordingSpec);
    desiredRecordingSpec.freq = 44100;
    desiredRecordingSpec.format = AUDIO_S16;
    desiredRecordingSpec.channels = 1;
    desiredRecordingSpec.samples = SAMPLES_PER_FRAME;
    if (current_mode == Mode::PC_MODE)
    {
        desiredRecordingSpec.callback = pc_callback;
    }
    else if (current_mode == Mode::LED_MODE)
    {
        desiredRecordingSpec.callback = led_callback;
    }

    dev = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(selected_input, 1), SDL_TRUE, &desiredRecordingSpec, &receivedRecordingSpec, 0);
    SDL_PauseAudioDevice(dev, 0); //'0' means play mode
}

void close()
{
    SDL_CloseAudioDevice(dev);
    if (current_mode == Mode::PC_MODE)
    {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}

int main(int argc, char* args[])
{
    init();

    bool quit = false;

    if (current_mode == Mode::PC_MODE) //execution loop for PC mode
    {
        SDL_Event e;
        while (!quit)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
            }
        }
    }
    else if (current_mode == Mode::LED_MODE) //execution loop for LED mode
    {
        cout << "Press ESC to finish." << endl;
        while (!quit)
        {
            quit = GetAsyncKeyState(VK_ESCAPE);
        }
    }

    close();
    return 0;
}

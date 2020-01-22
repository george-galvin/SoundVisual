void fftw_callback( void* userdata, Uint8* stream, int bytes )
{
    Sint16* real_stream = (Sint16*)stream;
    int length = bytes/2;

    int sum = 0;
    for(int a = 0; a < length; a++){
        sum += abs(real_stream[a]);
    }
    float avg = sum / length;

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
                    RGBValues[i][j][2] = std::min((int)(pow(pow(f_transform[i][0], 2) + pow(f_transform[i][1], 2), 0.5)/5000), 255);
                }
                SDL_FillRect( gScreenSurface, &PixelArray[i][j], SDL_MapRGB( gScreenSurface->format, RGBValues[i][j][0], RGBValues[i][j][1], RGBValues[i][j][2] ) );
            }
        }
    SDL_UpdateWindowSurface( gWindow );
}

void simple_grey_callback( void* userdata, Uint8* stream, int bytes )
{
    Sint16* real_stream = (Sint16*)stream;
    int length = bytes/2;

    int sum = 0;
    for(int a = 0; a < length; a++){
        sum += abs(real_stream[a]);
    }
    float avg = sum / length;

    for (int i = 0; i < numx; ++i)
        {
            for (int j = 0; j < numy; ++j)
            {
                for (int k = 0; k < 3; ++k)
                {
                    RGBValues[i][j][k] = avg/2;
                }
                SDL_FillRect( gScreenSurface, &PixelArray[i][j], SDL_MapRGB( gScreenSurface->format, RGBValues[i][j][0], RGBValues[i][j][1], RGBValues[i][j][2] ) );
            }
        }
    SDL_UpdateWindowSurface( gWindow );
}

void simple_color_callback( void* userdata, Uint8* stream, int bytes )
{
    Sint16* real_stream = (Sint16*)stream;
    int length = bytes/2;


    for (int i = 0; i < numx; ++i)
    {
        for (int j = 0; j < numy; ++j)
        {
            RGBValues[i][j][0] = (abs(real_stream[j*numx + i])/2);
            RGBValues[i][j][1] = (abs(real_stream[j*numx + i + (numx*numy)])/2);
            RGBValues[i][j][2] = (abs(real_stream[j*numx + i + (2*numx*numy)])/2);
            SDL_FillRect( gScreenSurface, &PixelArray[i][j], SDL_MapRGB( gScreenSurface->format, RGBValues[i][j][0], RGBValues[i][j][1], RGBValues[i][j][2] ) );
        }
    }
    SDL_UpdateWindowSurface( gWindow );
}

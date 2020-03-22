#include <windows.h>
#include <iostream>

DCB serialParams;
byte data[50];
DWORD bytessent;

int main(int argc, char* argv[])
{
    HANDLE arduino = CreateFile("/COM5", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    //GetCommState(arduino, &serialParams);
    serialParams.BaudRate = CBR_38400;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;

    SetCommState(arduino, &serialParams);

    for (int t = 0; t < 20; t++)
    {
        for (int i = 0; i < 50; i++)
        {
            data[i] = 128 * (t % 2);
        }
        WriteFile(arduino, &data, 50, &bytessent, 0);
        Sleep(125);
    }

	return 0;
}

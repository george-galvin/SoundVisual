#include <fstream>
#include <windows.h>

DCB serialParams;
byte data[1];
DWORD bytessent;

int main(int argc, char* argv[])
{
    data[0] = 1;

    HANDLE arduino = CreateFile("/COM5", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    serialParams.BaudRate = CBR_19200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    SetCommState(arduino, &serialParams);

	WriteFile(arduino, &data, 1, &bytessent, 0);
	return 0;
}

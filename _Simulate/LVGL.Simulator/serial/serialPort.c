#include <windows.h>
#include <stdio.h>
#include "stdint.h"
#include "serialPort.h"

#include <windows.h>
#include <stdio.h>

extern void OutputDebugPrintf(const char* strOutputString, ...);
static   HANDLE hSerial = 0;
static void (*Handle_cb)(uint8_t* dr);

void SerialPort_SendBuf(uint8_t* buf, uint16_t len)
{
    DWORD bytesWritten;
    if (hSerial == 0)
        return;
    OutputDebugPrintf("\r\n [BL]<- %s \r\n",buf);
  /*  for (int i = 0;i < len;i++)
        OutputDebugPrintf(".%02X", buf[i]);*/
    //OutputDebugPrintf("\r\n");

    if (!WriteFile(hSerial, buf, len, &bytesWritten, NULL)) {
        CloseHandle(hSerial);
        return 1;
    }
}

void Serial_Thread(LPCWSTR comname)
{
    STARTUPINFO si = { sizeof(si) };
    if (hSerial != 0)
        return;

    LPCWSTR portName = comname;
    hSerial = CreateFile(portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        0,
        0);
    if (hSerial == INVALID_HANDLE_VALUE) {
        OutputDebugPrintf("Error opening serial port\n");
        return;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        OutputDebugPrintf("Error getting state\n");
        CloseHandle(hSerial);
        return;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        OutputDebugPrintf("Error setting serial port state\n");
        CloseHandle(hSerial);
        return;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        OutputDebugPrintf("Error setting timeouts\n");
        CloseHandle(hSerial);
        return;
    }

    char dataToSend[] = "Hello, Serial Port!";
    DWORD bytesWritten;
    uint8_t dataReceived[20];
    DWORD bytesRead;
    int count = 0;
    while (1)
    {
        if (!ReadFile(hSerial, dataReceived, sizeof(dataReceived), &bytesRead, NULL)) {
            OutputDebugPrintf("Error reading from serial port\n");
            CloseHandle(hSerial);
            return;
        }
        if (bytesRead)
        {
            count = 0;
            while (bytesRead--)
            {
                uint32_t hex = dataReceived[count];
                //OutputDebugPrintf("_%02X", hex);
                if (Handle_cb)
                    Handle_cb(dataReceived[count++]);
                Sleep(1);
            }
            //OutputDebugPrintf("\r\n [BL]--> %s\r\n",dataReceived);
            //OutputDebugPrintf("\r\n");
        }
    }

    CloseHandle(hSerial);
    return;
}
void SerialPort_Init(void (*cb)(uint8_t* dr), LPCWSTR comname)
{
    HANDLE hThread;
    DWORD dwThreadId;
    hThread = CreateThread(NULL,		 // 默认安全属性
        0,			 // 默认堆栈大小
        Serial_Thread,	 // 线程函数
        comname,			 // 传递给线程函数的参数
        0,			 // 默认创建标志
        &dwThreadId); // 线程ID)
    Handle_cb = cb;
}

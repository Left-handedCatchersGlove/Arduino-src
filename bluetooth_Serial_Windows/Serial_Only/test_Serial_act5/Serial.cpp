#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include "Serial.h"

HANDLE hCom = INVALID_HANDLE_VALUE;
HANDLE hThread = NULL;
DWORD ThreadId;

void Serial::Open()
{
  hCom = CreateFile( "COM3", GENERIC_READ | GENERIC_WRITE,
    0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  if( hCom == INVALID_HANDLE_VALUE ) {
    printf("Open Error!\n");
    exit(1);
  }
  DCB dcb;
  memset(&dcb, 0, sizeof(DCB));
  dcb.DCBlength = sizeof(DCB) ;
  //dcb.BaudRate = CBR_19200;
  dcb.BaudRate = 9600;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  SetCommState(hCom, &dcb);
  hThread = CreateThread(NULL, 0, ReadDataWithOverLapped, NULL, 0, &ThreadId);
}

DWORD WINAPI ReadDataWithOverLapped(LPVOID lpParam)
{
  OVERLAPPED ovlp;
  memset(&ovlp, 0, sizeof(OVERLAPPED));
  ovlp.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  while(1)
  {
    char buff[1024];
    DWORD nRead = 0;
    if (!ReadFile(hCom, buff, 10, &nRead, &ovlp)) 
    {
      if (GetLastError() == ERROR_IO_PENDING) 
      {
        GetOverlappedResult(hCom, &ovlp, &nRead, TRUE);
      }
    }
    if (nRead)
    {
      // éÛêMèàóù
    }
    ResetEvent(ovlp.hEvent);
  }
  CloseHandle(ovlp.hEvent);
  return 0;
}
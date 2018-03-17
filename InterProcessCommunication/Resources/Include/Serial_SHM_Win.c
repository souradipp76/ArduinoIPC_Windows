#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "SerialMI_Win.h"
//#pragma comment(lib, "user32.lib")


/*struct DataExchange {
    char sendVal[10][32];
    double getVal[10];
};*/
//struct DataExchange *PID;

#define BUF_SIZE 256

static HANDLE hMapFile;

TCHAR szName[]=TEXT("Global\\MyFileMappingObject");
TCHAR szMsg[]=TEXT("Message from first process.");

__declspec(dllexport)  __stdcall void shmWriteSerial(int num1,double tagValue)
{

   //LPCTSTR pBuf;
   char* pBuf;

   hMapFile = CreateFileMappingA(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 0,                       // maximum object size (high-order DWORD)
                 BUF_SIZE,                // maximum object size (low-order DWORD)
                 szName);                 // name of mapping object

   if (GetLastError() == ERROR_ALREADY_EXISTS) {
        hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, szName);
   }

   if (hMapFile == NULL)
   {
      _tprintf(TEXT("Could not create file mapping object (%d).\n"),
             GetLastError());
      exit(1);
   }
   /*pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        BUF_SIZE);
    */
   pBuf = (char*) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        BUF_SIZE);

   if (pBuf == NULL)
   {
      _tprintf(TEXT("Could not map view of file (%d).\n"),
             GetLastError());

       CloseHandle(hMapFile);
	     exit(1);
   }

   char outData[256]="";
   sprintf(outData,"%d,%g\n", num1, tagValue);
   //strcpy(PID-> sendVal[num1], outData);

   memcpy(pBuf,outData,BUF_SIZE);
   //CopyMemory((PVOID)pBuf, PID, sizeof(struct DataExchange));
   //_getch();

   UnmapViewOfFile(pBuf);

   CloseHandle(hMapFile);

}



__declspec(dllexport)  __stdcall char* shmReadSerial(int num2)
{
   //LPCTSTR pBuf;
   char* pBuf;
   static char returnVal[BUF_SIZE]="";
   //char temp[256]="";

   hMapFile = OpenFileMappingA(
                   FILE_MAP_ALL_ACCESS,   // read/write access
                   FALSE,                 // do not inherit the name
                   szName);               // name of mapping object

   if (hMapFile == NULL)
   {
      _tprintf(TEXT("Could not open file mapping object (%d).\n"),
             GetLastError());
      //exit(1);
      return returnVal;
   }

   /*pBuf = (LPTSTR) MapViewOfFile(hMapFile, // handle to map object
               FILE_MAP_ALL_ACCESS,  // read/write permission
               0,
               0,
               BUF_SIZE);
    */
   pBuf = (char*) MapViewOfFile(hMapFile, // handle to map object
               FILE_MAP_ALL_ACCESS,  // read/write permission
               0,
               0,
               BUF_SIZE);

   if (pBuf == NULL)
   {
      _tprintf(TEXT("Could not map view of file (%d).\n"),
             GetLastError());

      CloseHandle(hMapFile);
	    //exit(1);
      return returnVal;
   }
   //PID = (struct DataExchange*)pBuf;
   //MessageBox(NULL, pBuf, TEXT("Process2"), MB_OK);
   
   memcpy(returnVal,pBuf,BUF_SIZE);
   //memcpy(pBuf,temp,BUF_SIZE);
   //printf("InData:%s",returnVal);
   int adr; 
   float Val;
   if(sscanf(returnVal,"%d,%g\n",&adr,&Val)==0)
   {
      _tprintf(TEXT("Error reading shared memory\n"));
      UnmapViewOfFile(pBuf);
      CloseHandle(hMapFile);
      return "";
   }
   if(adr!=num2)
   {
      _tprintf(TEXT("Error accessing shared memory\n"));
      UnmapViewOfFile(pBuf);
      CloseHandle(hMapFile);
      return "";
   }  

   UnmapViewOfFile(pBuf);

   CloseHandle(hMapFile);

   return returnVal;
}

int _tmain()
{
	//char S_Port[32]="";
    int S_Port;
	  int S_Baud;
    printf("Serial Port (e.g. 5 for COM5) : ");
    scanf("%d", &S_Port);
    printf("Baud Rate (e.g. 115200) : ");
    scanf("%d", &S_Baud);
    serialBegin(&S_Port, &S_Baud);
	  serialFlush();

	 while(1)
    {
    	  char* outData;
        char someData[32]="";
        const char* inData;
        char val[10]="";
        char addr[10]="";
        int i,j;

        outData = shmReadSerial(1);
        //Sleep(1000);
        if(strcmp(outData, "") == 0);
        else{
        	printf("input:%s", outData);
        	serialWrite(outData);
        	Sleep(1000);
          strcpy(outData,"");
        	//strcpy(PID->sendVal[i], "");
        }

        //printf("Available:%d\n",serialAvailable());  
        if(serialAvailable()>0)
        {
                inData = serialRead();
                strcpy(someData, inData);
                printf("ardOutput:%s\n", someData);
                for(i=0; i<strlen(someData); i++)
                {
                    if(someData[i]==',')
                    {
                        addr[i]='\0';
                        i++;
                        break;
                    }
                    addr[i] = someData[i];
                }

                for(j=i; j<strlen(someData); j++)
                {
                    val[j-i] = someData[j];
                }
                shmWriteSerial(1, atof(val));
        }
    }
  printf("came out of loop..!");
  serialEnd();
}

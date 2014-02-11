//---------------------------------------------------------------------------//
/* sio_util - provides Windows-based comm port communication capabilities*/


/* Change History                                                            
                                                                          
	JRK  12/16/05 - Initial Version                                           
	RBM  6/13/11 - Modified to remove any Borland-specific calls and removed the
	library export functionality. Also updated to be compliant with 16 bit characters
	for Unicode compliance (b/c Qt requires this)
//---------------------------------------------------------------------------*/
#include <windows.h>

#include <stdio.h>
#include "sio_util.h"
#include <QTextStream>

//---------------------------------------------------------------------------//
// Global variables                                                          //
//---------------------------------------------------------------------------//




//---------------------------------------------------------------------------//
//  Function Name:  SioOpen (Internal Library Function)                      //
//  Return Value:   Returns a handle to a COM port stream                    //
//  Parameters:     portname:  name of COM port ("COMn:", where n=1-8)       //
//                  baudrate: 9600,19200,38400,57600,115200,230400           //
//  Description:    Opens a COM port at the specified baud rate.  Set up     //
//                  read and write timeouts.                                 //
//---------------------------------------------------------------------------//
HANDLE SioOpen(QString portLocation, unsigned int baudrate,QString* error_string )
{
bool RetStat;
COMMCONFIG cc;
COMMTIMEOUTS ct;
HANDLE ComHandle;
DWORD winrate;
#ifdef UNICODE
    WCHAR portname[10];
    wcscpy(portname,(LPCTSTR)portLocation.utf16());
#else
    CHAR portname[10];
    strcpy(portname,(LPCTSTR)portLocation.data());
#endif

//Open COM port as a file

ComHandle = CreateFile(portname, GENERIC_READ | GENERIC_WRITE ,0, NULL,
			           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);

QTextStream ss(error_string,QIODevice::WriteOnly);
while (TRUE)
	{
	if (ComHandle == INVALID_HANDLE_VALUE)
		{
                ss<<"\n port named "<< portLocation <<" failed to open\n";
        break;
    	}

    switch (baudrate) {
		case 9600: 		winrate = CBR_9600; break;
		case 19200: 	winrate = CBR_19200; break;
		case 38400: 	winrate = CBR_38400; break;
		case 57600: 	winrate = CBR_57600; break;
		case 115200: 	winrate = CBR_115200; break;
		case 230400: 	winrate = 230400; break;
        default:		ss<<"Baud rate not supported using defualr of 19200";
    					winrate = CBR_19200;
    	}

    //Fill in COM port config. structure & set config.
    cc.dwSize = sizeof(DCB) + sizeof(WCHAR) + 20;
    cc.wVersion = 1;

    cc.dcb.DCBlength = sizeof(DCB);
    cc.dcb.BaudRate = winrate;
    cc.dcb.fBinary = 1;
    cc.dcb.fParity = 0;
    cc.dcb.fOutxCtsFlow = 0;
    cc.dcb.fOutxDsrFlow = 0;
    cc.dcb.fDtrControl = DTR_CONTROL_DISABLE;
    cc.dcb.fDsrSensitivity = 0;
    cc.dcb.fTXContinueOnXoff = 0;
    cc.dcb.fOutX = 0;
    cc.dcb.fInX = 0;
    cc.dcb.fErrorChar = 0;
    cc.dcb.fNull = 0;
    cc.dcb.fRtsControl = RTS_CONTROL_DISABLE;
    cc.dcb.fAbortOnError = 0;
    cc.dcb.XonLim = 100;
    cc.dcb.XoffLim = 100;
    cc.dcb.ByteSize = 8;
    cc.dcb.Parity = NOPARITY;
    cc.dcb.StopBits = ONESTOPBIT;
    cc.dcb.XonChar = 'x';
    cc.dcb.XoffChar = 'y';
    cc.dcb.ErrorChar = 0;
    cc.dcb.EofChar = 0;
    cc.dcb.EvtChar = 0;

    cc.dwProviderSubType = PST_RS232;
    cc.dwProviderOffset = 0;
    cc.dwProviderSize = 0;

    RetStat = SetCommConfig(ComHandle, &cc, sizeof(cc));
	if (RetStat == 0)
    	{
        ss<<"Failed to set COMM configuration";
        break;
        }

    //Set read/write timeout values for the file
    ct.ReadIntervalTimeout = 0;  		//ignore interval timing
    ct.ReadTotalTimeoutMultiplier = 2; 	//2 msec per char
    ct.ReadTotalTimeoutConstant = 50;  		//plus add'l 50 msec
    ct.WriteTotalTimeoutMultiplier = 2;	//Set max time per char written
    ct.WriteTotalTimeoutConstant = 50;	//plus additional time

	RetStat = SetCommTimeouts(ComHandle, &ct);
    if (RetStat == 0)
    	{
        ss<<"Failed to set Comm timeouts";
        break;
        }

    break;
	}

return(ComHandle);
}


//---------------------------------------------------------------------------//
//  Function Name:  SioChangeBaud (Internal Library Function)                //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     ComPort: COM port handle                                 //
//                  baudrate: 9600,19200,38400,57600,115200,230400           //
//  Description:    Change the baud rate to the specified values.            //
//---------------------------------------------------------------------------//
bool SioChangeBaud(HANDLE ComPort, unsigned int baudrate, QString* error_string)
{
bool RetStat;
DWORD winrate;
DCB cs;

RetStat = GetCommState(ComPort, &cs);
if (RetStat == false) return RetStat;
switch (baudrate) {
	case 9600: 		winrate = CBR_9600; break;
	case 19200: 	winrate = CBR_19200; break;
	case 38400: 	winrate = CBR_38400; break;
        case 57600: 	winrate = CBR_57600; break;
	case 115200: 	winrate = CBR_115200; break;
	case 230400: 	winrate = 230400; break;
        default:{
                QTextStream ss(error_string,QIODevice::WriteOnly);
                ss<<"Baud rate not supported";
                return false;}
    }
cs.BaudRate = winrate;
RetStat = SetCommState(ComPort, &cs);
if (RetStat == false) return RetStat;
return true;
}


//---------------------------------------------------------------------------//
//  Function Name:  SioPutChars (Internal Library Function)                  //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     ComPort: COM port handle                                 //
//                  stuff: array containing character data to send           //
//                  n: number of characters to send                          //
//  Description:    Write out n chars to the ComPort, returns only after     //
//                  chars have been sent.                                    //
//---------------------------------------------------------------------------//
bool SioPutChars(HANDLE ComPort, char *stuff, int n, QString* error_string)
{
bool RetStat;
DWORD nums;

RetStat = (bool)WriteFile(ComPort, stuff,n, &nums, NULL);
if (RetStat == 0) {
    QTextStream ss(error_string,QIODevice::WriteOnly);
    ss<<"SioPutChars failed";
}
return RetStat;
}


//---------------------------------------------------------------------------//
//  Function Name:  SioGetChars (Internal Library Function)                  //
//  Return Value:   Returns the number of characters actually read           //
//  Parameters:     ComPort: COM port handle                                 //
//                  stuff: array to store characters read                    //
//                  n: number of characters to read                          //
//  Description:    Read n chars into the array stuff.                       //
//---------------------------------------------------------------------------//
DWORD SioGetChars(HANDLE ComPort, char *stuff, int n, QString* error_string)
{
    bool RetStat;
    DWORD numread;

    RetStat = (bool)ReadFile(ComPort, stuff, n, &numread, NULL);
    if (RetStat == 0) {
        QTextStream ss(error_string,QIODevice::WriteOnly);
        ss<<"SioReadChars failed";
    }

    return numread;
}


//---------------------------------------------------------------------------//
//  Function Name:  SioTest (Internal Library Function)                      //
//  Return Value:   Returns the number of characters in ComPort's input buf  //
//  Parameters:     ComPort: COM port handle                                 //
//  Description:    Returns the number of chars in a port's input buffer.    //
//---------------------------------------------------------------------------//
DWORD SioTest(HANDLE ComPort, QString* error_string)
{
    COMSTAT cs;
    DWORD Errors;
    bool RetStat;

    RetStat = (bool)ClearCommError(ComPort, &Errors, &cs);
    if (RetStat == 0){
        QTextStream ss(error_string,QIODevice::WriteOnly);
        ss<<"SioTest failed";
    }
    return cs.cbInQue;
}


//---------------------------------------------------------------------------//
//  Function Name:  SioClrInBuf (Internal Library Function)                  //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     ComPort: COM port handle                                 //
//  Description:    Purge all chars from a port's input buffer.              //
//---------------------------------------------------------------------------//
bool SioClrInbuf(HANDLE ComPort,QString* error_string)
{
    bool RetStat;

    RetStat = (bool)PurgeComm(ComPort, PURGE_RXCLEAR);
    if (RetStat == 0){
        QTextStream ss(error_string,QIODevice::WriteOnly);
        ss<<"SioClrInbuf failed";
    }

    return RetStat;
}


//---------------------------------------------------------------------------//
//  Function Name:  SioClose (Internal Library Function)                     //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     ComPort: COM port handle                                 //
//  Description:    Close a previously opened COM port.                      //
//---------------------------------------------------------------------------//
bool SioClose(HANDLE ComPort)
{
return((bool)CloseHandle(ComPort));
}
//---------------------------------------------------------------------------


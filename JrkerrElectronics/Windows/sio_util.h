//---------------------------------------------------------------------------//
/* Change History                                                            
                                                                          
	JRK  12/16/05 - Initial Version                                           
	RBM  6/13/11 - Modified to remove any Borland-specific calls and removed the
	library export functionality. Also updated to be compliant with 16 bit characters
	for Unicode compliance (b/c Qt requires this)
        JIL 6/23/11  - Modified the system to use QStrings and QTextStreams for error handeling
//---------------------------------------------------------------------------*/
#ifndef sio_utilH
#define sio_utilH



#include <QString>

typedef unsigned long DWORD;
typedef void* HANDLE;

//Serial IO function prototypes:

HANDLE SioOpen(QString name, unsigned int baudrate, QString* error_string );
bool SioPutChars(HANDLE ComPort, char *stuff, int n,QString* error_string );
DWORD SioGetChars(HANDLE ComPort, char *stuff, int n,QString* error_string );
DWORD SioTest(HANDLE ComPort,QString* error_string );
bool SioClrInbuf(HANDLE ComPort,QString* error_string );
bool SioChangeBaud(HANDLE ComPort, unsigned int baudrate,QString* error_string );
bool SioClose(HANDLE ComPort);






#endif

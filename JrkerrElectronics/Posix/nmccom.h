//---------------------------------------------------------------------------//
// Change History                                                            //
//                                                                           //
// JRK  12/16/05 - Initial Version                                           //
// JIL  6/24/11 - QString error handling                                     //
//---------------------------------------------------------------------------//
#ifndef nmccomH
#define nmccomH

#include <QString>
#include "../Posix/qextserial/qextserialport.h" //// Diff


//The following must be created for each new module type:
//		data structure XXXMOD
//		Initializer function NewXXXMod
//		Status reading function GetXXXStat
//		NMCInit and SendNmcCmd must be modified to include calls
//			to the two functions above

#define INVALID_HANDLE_VALUE (HANDLE)(-1)

#define MAXSIOERROR     2
#define CKSUM_ERROR	0x02	//Checksum error bit in status byte
#ifdef FatHmBrd
    #define MAXNUMMOD  12
#else
    #define MAXNUMMOD	33
#endif

//Define PIC baud rate divisors
#define	PB19200		63//DIFF
#define	PB57600		20//DIFF
#define	PB115200	10
#define	PB230400	5

//Module type definitions:
#define	SERVOMODTYPE	0
#define	ADCMODTYPE	1
#define	IOMODTYPE	2
#define	STEPMODTYPE	3

typedef unsigned char byte;
typedef unsigned int DWORD;

typedef struct _NMCMOD {
        byte	modtype;		//module type
    byte	modver;			//module version number
        byte	statusitems;            //definition of items to be returned
        byte	stat;  			//status byte
    byte	groupaddr;		//current group address
    bool	groupleader;            //true if group leader
    void *	p;			//pointer to specific module's data structure NOTE: this pointer is NOT initialized! Really ought to make this a class that has a constructor that initializes it to a benign value
    } NMCMOD;









//Function prototypes:

//Initialization and shutdown
int NmcInit(QString portLocation, unsigned int baudrate, QString* error_string);
void InitVars(void);
bool NmcSendCmd(byte addr, byte cmd, char *datastr, byte n, byte stataddr,QString*error_string);
void FixSioError(QString* error_string);
void NmcShutdown(QString* error_string);

//Module type independant commands (supported by all module types)
bool NmcSetGroupAddr(byte addr, byte groupaddr, bool leader, QString* error_string);
bool NmcDefineStatus(byte addr, byte statusitems, QString* error_string);
bool NmcReadStatus(byte addr, byte statusitems, QString* error_string);
bool NmcSynchOutput(byte groupaddr, byte leaderaddr, QString* error_string);
bool NmcChangeBaud(byte groupaddr, unsigned int baudrate, QString* error_string);
bool NmcSynchInput(byte groupaddr, byte leaderaddr, QString* error_string);
bool NmcNoOp(byte addr, QString* error_string);
bool NmcHardReset(byte addr, QString* error_string);

//Retrieve module type independant data from a module's data structure
byte NmcGetStat(byte addr);
byte NmcGetStatItems(byte addr);
byte NmcGetModType(byte addr);
byte NmcGetModVer(byte addr);
byte NmcGetGroupAddr(byte addr);
bool NmcGroupLeader(byte addr);

void nap(unsigned long msec);

QextSerialPort* getComPort();
#endif


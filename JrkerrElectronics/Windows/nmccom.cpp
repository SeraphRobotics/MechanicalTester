//---------------------------------------------------------------------------//
/* Change History                                                            
                                                                          
	JRK  12/16/05 - Initial Version                                           
	RBM  6/13/11 - Modified to remove any Borland-specific calls and removed the
	library export functionality. Also updated to be compliant with 16 bit characters
	for Unicode compliance (b/c Qt requires this)
//---------------------------------------------------------------------------*/

#include <stdio.h>
#include "nmccom.h"
#include "picio.h"
#include "picservo.h"
#include "picstep.h"
#include "sio_util.h"
//#include "Global_defines.h"
#include <QTextStream>
#include <QWaitCondition>
#include <QMutex>



//---------------------------------------------------------------------------//
//Global data                                                                //
//---------------------------------------------------------------------------//
NMCMOD mod[MAXNUMMOD]; 	//Array of modules
int nummod = 0;				//start off with no modules
char cmdstr[20];			//use global command string
void * ComPort = INVALID_HANDLE_VALUE;			//Pointer to the comport 
int SioError = 0;
int IOBusy = false;
unsigned int BaudRate;


//---------------------------------------------------------------------------//
//  Function Name:  NmcHardReset                                             //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     addr: module address (1-32) or group address (0x80-0xFF) //
//  Description:    Resets all controllers with group address 'addr'         //
//                  (should include all modules).                            //
//---------------------------------------------------------------------------//
bool NmcHardReset(byte addr, QString* error_string)
{
int i;
char cstr[5];

//Send string of 0's to flush input buffers
cstr[0] = 0;
for (i=0; i<20; i++) SioPutChars(ComPort,cstr, 1,error_string);

//Send out reset command string
cstr[0] = 0xAA;			//Header
cstr[1] = addr;
cstr[2] = HARD_RESET;
cstr[3] = (byte)(cstr[1] + cstr[2]);  //checksum
SioPutChars(ComPort, cstr, 4,error_string);//send reset

nap(100);		//wait for reset to execute

for (i=0; i<nummod; i++) delete mod[i].p;  //delete and prev. declared modules
nummod = 0;
SioError = 0;	//reset the number of errors to 0

#ifdef FatHmBrd
        SioChangeBaud(ComPort, 230400,error_string);    //reset with default rate of 230400 for FatHmBrd...
#else
        SioChangeBaud(ComPort, 19200,error_string);    //reset with default rate of 19200 for JRK boards
#endif

SioClrInbuf(ComPort,error_string);   //clear out any random crap left in buffer

return TRUE;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcChangeBaud                                            //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     groupaddr: group address (0x80 - 0xFF)                   //
//                  baudrate: 19200, 57600, 115200, 230400 (PIC-SERVO only)  //
//  Description:    Change the baud rate of all controllers with group       //
//                  address 'groupaddr' (should include all modules) and     //
//                  also changes host's baud rate.  There should be no group //
//                  leader for 'groupaddr'.                                  //
//---------------------------------------------------------------------------//
bool NmcChangeBaud(byte groupaddr, unsigned int baudrate, QString* error_string)
{
char cstr[6];

cstr[0] = 0xAA;			//Header
cstr[1] = groupaddr;
cstr[2] = 0x10 | SET_BAUD;
switch (baudrate) {
	case 19200:		cstr[3] = PB19200; break;
	case 57600:		cstr[3] = PB57600; break;
	case 115200:	cstr[3] = PB115200; break;
	case 230400:	cstr[3] = PB230400; break;
        default:{
            QTextStream ss(error_string,QIODevice::WriteOnly);
            ss<<"Baud rate not supported - using default of 19200";
            cstr[3] = PB19200;
            baudrate = 19200;}
    }
cstr[4] = (byte)(cstr[1] + cstr[2] + cstr[3]);  //checksum
SioPutChars(ComPort, cstr, 5,error_string);//send command

nap(100);		//Wait for command to execute

SioChangeBaud(ComPort, baudrate,error_string);	//Reset the baud rate to the default

SioClrInbuf(ComPort,error_string);   //clear out any random crap left in buffer
nap(100);

BaudRate = baudrate;
return true;
}


//---------------------------------------------------------------------------//
//  Function Name:  InitVars (Internal Library Function)                     //
//  Return Value:   None                                                     //
//  Parameters:     None                                                     //
//  Description:    Initialize misc network variables.                       //
//---------------------------------------------------------------------------//
void InitVars(void)
{
int i;

mod[0].modtype = 0;   //Default to a known module type for module 0
mod[0].modver = 0;
mod[0].stat = 0;
mod[0].statusitems = 0;
mod[0].groupaddr = 0xFF;
mod[0].groupleader = false;

for (i=1; i<MAXNUMMOD; i++)
	{
	mod[i].modtype = 0xFF;
	mod[i].modver = 0;
    mod[i].stat = 0;
    mod[i].statusitems = 0;
    mod[i].groupaddr = 0xFF;
    mod[i].groupleader = false;
    }
}


//---------------------------------------------------------------------------//
/*  Function Name:  NmcInit                                                  //
//  Return Value:   Returns the number of controller found on network        //
//  Parameters:     portLocation: name of COM port ("COMn:", where n=1-8)        //
//                  baudrate: 19200, 57600, 115200, 230400 (PIC-SERVO only)  //
//  Description:    Initialize the network of controllers with sequential    //
//                  addresses starting at 1. Note that no other part of the 
					system may address a channel outside of those returned by 
					this fn! If they do, undefined pointers will be referenced!!
					This should really be fixed.
//---------------------------------------------------------------------------*/
int NmcInit(QString portLocation, unsigned int baudrate, QString* error_string)
{


DWORD numread;
byte addr;
char cstr[20];
SERVOMOD *p;


QTextStream ss(error_string,QIODevice::WriteOnly);

BaudRate = baudrate;

InitVars();

if (ComPort != INVALID_HANDLE_VALUE) SioClose(ComPort);	//if the handle is not as it was when created, assume it's been opened, so close it

#ifdef FatHmBrd
        ComPort = SioOpen(portLocation, baudrate, error_string);    //Open with default rate of 230400 for FatHmBrd...
#else
        ComPort = SioOpen(portLocation, 19200, error_string);    //... or Open with default rate of 19200 for JRK boards
#endif

if (ComPort == INVALID_HANDLE_VALUE){ 
        ss<<"Port open failed";
	return 0;
	}

NmcHardReset(0xFF,error_string);    //reset all the attached channels/devices to the default settings

for (addr = 1; addr < MAXNUMMOD; addr++)
{
    //First set the address to a unique value:
    cstr[0] = 0xAA;		//Header
    cstr[1] = 0;  		//Send to default address of 0
    cstr[2] = 0x20 | SET_ADDR;
    cstr[3] = addr;  	//Set new address sequentially
    cstr[4] = 0xFF;		//Set group address to 0xFF
    cstr[5] = (byte)(cstr[1] + cstr[2] + cstr[3] + cstr[4]);  //checksum
    SioPutChars(ComPort, cstr, 6,error_string);  //Send command

    numread = SioGetChars(ComPort, cstr, 2,error_string);  //get back status, cksum
    if (numread!=2) break;	//if no response, punt out of loop
    if ((cstr[0] != cstr[1]))
        {
                ss<<"Chksum error during address set in NMCInit"<<addr;
                ss<<"\n cstr[0] = "<< cstr[0]<<"cstr[1]:"<<cstr[1];
        SioClose(ComPort);
        return(0);
        }

    //Read the device type
    cstr[0] = 0xAA;		//Header
    cstr[1] = addr;  		//Send to addr
    cstr[2] = 0x10 | READ_STAT;
    cstr[3] = SEND_ID;  //send ID only
    cstr[4] = (byte)(cstr[1] + cstr[2] + cstr[3]);  //checksum
    SioPutChars(ComPort, cstr, 5,error_string);  //Send command

    numread = SioGetChars(ComPort, cstr, 4,error_string);  //get back stat, type, ver, cksum
    if (numread!=4)
    	{
        //ErrorMsgBox(L"Could not read device type");
                ss<<"Could not read device at address  "<<addr;
        //SioClose(ComPort);
        //return(0);
		break;
        }
    mod[addr].stat = cstr[0];
    mod[addr].modtype = cstr[1];
    mod[addr].modver = cstr[2];
    mod[addr].statusitems = 0;
    mod[addr].groupaddr = 0xFF;
    mod[addr].groupleader = false;
    switch (mod[addr].modtype)
    	{
        case SERVOMODTYPE: 	mod[addr].p = ServoNewMod();
        					//fix for change in I/O CTRL command
        					if ((mod[addr].modver<10) || (mod[addr].modver>20))
                              {
        					  p = (SERVOMOD *)(mod[addr].p);
                              p->ioctrl = IO1_IN | IO2_IN;
                              }
        					break;
        case IOMODTYPE: 	mod[addr].p = IoNewMod(); break;
        case STEPMODTYPE: 	mod[addr].p = StepNewMod(); break;
        }

}

nummod = addr-1;

if (nummod > 0){
        NmcChangeBaud(0xFF, baudrate,error_string);
	//if (nummod>1) sprintf(mess,"%d Modules found", nummod);
        //else sprintf(mess,"%d Module found", nummod);
	//ErrorMsgBox(mess);
        //ss<<nummod<<" Modules found";
    }
else
	{
    //ErrorMsgBox("No modules found on the network.\nPlease check power and connections.");
        ss<<"No modules found on the network. Please check power and connections.";
    SioClose(ComPort);
    }

return(nummod);	//Return number of modules
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcSendCmd (Internal Library Function)                   //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     addr: module address (1-32) or group address (0x80-0xFF) //
//                  cmd: command type from PIC-SERVO.H, PIC_STEP.H, PIC_IO.H //
//                  datastr: string containing additional command data       //
//                  n: number of additional command data bytes               //
//                  stataddr: status data address                            //
//  Description:    Sends a command to addr and shoves the returned status   //
//                  data in the structure for module address stataddr.  If   //
//                  stataddr = 0, the command is taken as a group command    //
//                  with no group leader (no status returned).  n is the     //
//                  number of auxilliary data bytes.                         //
//---------------------------------------------------------------------------//
bool NmcSendCmd(byte addr, byte cmd, char *datastr, byte n, byte stataddr, QString* error_string)
{
byte cksum;
byte outstr[20];
int i;
bool iostat = false;

//If too many SIO errors, punt
//if (SioError > MAXSIOERROR) return(false); RBM commented out. we'll try handling this below

IOBusy = true;

//Check if data is to be returned from to a known module type
if ( !( ( mod[stataddr].modtype==SERVOMODTYPE) ||
        ( mod[stataddr].modtype==ADCMODTYPE) ||
        ( mod[stataddr].modtype==IOMODTYPE)  ||
        ( mod[stataddr].modtype==STEPMODTYPE) ) )
	{
    QTextStream ss(error_string,QIODevice::WriteOnly);
    ss<<"Module type "<<mod[stataddr].modtype<<" not supported";
    return (false);
    }

//Calculate the adjust command byte, calculate checksum and send the command
cksum = 0;
outstr[0] = 0xAA;	//start with header byte
outstr[1] = addr;	//add on address byte
cksum += outstr[1];
outstr[2] = (byte)(((n<<4) & 0xF0) | cmd);
cksum += outstr[2];
for (i=0; i<n; i++)
	{
    outstr[i+3] = datastr[i];
	cksum += outstr[i+3];
    }
outstr[n+3] = cksum;

SioClrInbuf(ComPort,error_string);  //Get rid of any old input chars
SioPutChars(ComPort, (char *)outstr, n+4,error_string); //Send the command string

if (stataddr == 0)		//If a group command w/ no leader, add delay then exit
	{
    nap(60);
    IOBusy = false;
	return true;
    }

//check for response and retry if response fails
while (!iostat){
    switch (mod[stataddr].modtype) {
        case SERVOMODTYPE:	iostat = ServoGetStat(stataddr,error_string); break;
        case IOMODTYPE:		iostat = IoGetStat(stataddr,error_string); break;
        case STEPMODTYPE:	iostat = StepGetStat(stataddr,error_string); break;
    }

    if (iostat == false)
    {
        SioError++; //error counter
        FixSioError(error_string);  //flush buffers, send some garbage chars to flush remote device buffers
        SioPutChars(ComPort, (char *)outstr, n+4,error_string); //resend the command string
    }
    else SioError = 0;  //reset the error counter when comms succeed

    if (SioError > MAXSIOERROR){ //stop trying if the number of consecutive errors is too large
        break;
    }

}

IOBusy = false;
return iostat;
}


//---------------------------------------------------------------------------//
//  Function Name:  FixSioError (Internal Library Function)                  //
//  Return Value:   None                                                     //
//  Parameters:     None                                                     //
//  Description:    Attempt to re-synch communications.                      //
//---------------------------------------------------------------------------//
void FixSioError(QString* error_string)
{
int i;
char teststr[2];

//If too many errors - prompt to reset
if (SioError >= MAXSIOERROR)
	{
    QTextStream ss(error_string,QIODevice::WriteOnly);
    ss<<"Multiple communication errors - please reset the Network";
    return;
    }

//Otherwise, send out a null string to re-synch
teststr[0] = 0;
for (i=0; i<30; i++) SioPutChars(ComPort, teststr, 1,error_string);
nap(100);
SioClrInbuf(ComPort,error_string);
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcSetGroupAddr                                          //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     addr: module address (1-32)                              //
//                  groupaddr: module's group address (0x80-0xFF)            //
//                  leader: 1 = group leader, 0 = group member               //
//  Description:    Sets module's group address.                             //
//---------------------------------------------------------------------------//
bool NmcSetGroupAddr(byte addr, byte groupaddr, bool leader, QString* error_string)
{
char cmdstr[2];

if (!(groupaddr & 0x80)) return false;  //punt if not valid group address

cmdstr[0] = addr;
cmdstr[1] = groupaddr;
if (leader) cmdstr[1] &= 0x7F;  //clear upper bit if a group leader
mod[addr].groupaddr = groupaddr;
mod[addr].groupleader = leader;
return NmcSendCmd(addr, SET_ADDR, cmdstr, 2, addr,error_string);
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcSynchOutput                                           //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     groupaddr: module addr (1-32) or group addr (0x80-0xFF)  //
//                  leaderaddr: if individual groupaddr: leaderaddr = addr   //
//                              if group groupaddr: leaderaddr = group leader//
//                              (if no group leader: leaderaddr = 0          //
//  Description:    Synchronous output command issued to groupaddr.  Status  //
//                  data goes to leaderaddr.  If no leader, use leaderaddr=0.//                        //
//---------------------------------------------------------------------------//
bool NmcSynchOutput(byte groupaddr, byte leaderaddr,QString* error_string)
{
return NmcSendCmd(groupaddr, SYNCH_OUT, NULL, 0, leaderaddr,error_string);
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcSynchInput                                            //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     groupaddr: module addr (1-32) or group addr (0x80-0xFF)  //
//                  leaderaddr: if individual groupaddr: leaderaddr = addr   //
//                              if group groupaddr: leaderaddr = group leader//
//                              (if no group leader: leaderaddr = 0          //
//  Description:    Synchronous input command issued to groupaddr.  Status   //
//                  data goes to leaderaddr.  If no leader, use leaderaddr=0.//
//---------------------------------------------------------------------------//
bool NmcSynchInput(byte groupaddr, byte leaderaddr,QString*error_string)
{
return NmcSendCmd(groupaddr, SYNCH_INPUT, NULL, 0, leaderaddr,error_string);
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcNoOp                                                  //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     addr: module addr (1-32)                                 //
//  Description:    No operation - module returns it's current status.       //
//---------------------------------------------------------------------------//
bool NmcNoOp(byte addr,QString* error_string)
{
return NmcSendCmd(addr, NOP, NULL, 0, addr,error_string);
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcReadStatus                                            //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     addr: module addr (1-32)                                 //
//                  statusitems: logical or of status items to be returned   //
//                               (from PIC-SERVO.H, PIC-STEP.H, or PIC-IO.H) //
//  Description:    Returns the specified status items once.                 //
//---------------------------------------------------------------------------//
bool NmcReadStatus(byte addr, byte statusitems,QString* error_string)
{
char cmdstr[2];
byte oldstat;
bool retval;

cmdstr[0] = statusitems;
oldstat = mod[addr].statusitems;
mod[addr].statusitems = statusitems;
retval = NmcSendCmd(addr, READ_STAT, cmdstr, 1, addr,error_string);
mod[addr].statusitems = oldstat;

return retval;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcDefineStatus                                          //
//  Return Value:   0=Fail, 1=Success                                        //
//  Parameters:     addr: module address (1-32)                              //
//                  statusitems: logical or of status items to be returned   //
//                               (from PIC-SERVO.H, PIC-STEP.H, or PIC-IO.H) //
//  Description:    Defines what status data is returned after each command  //
//                  packet sent.                                             //
//---------------------------------------------------------------------------//
bool NmcDefineStatus(byte addr, byte statusitems, QString* error_string)
{
char cmdstr[2];

cmdstr[0] = statusitems;
mod[addr].statusitems = statusitems;
return NmcSendCmd(addr, DEF_STAT, cmdstr, 1, addr,error_string);
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcGetStat                                               //
//  Return Value:   Current status byte (stored locally) of a controller     //
//  Parameters:     addr: module address (1-32)                              //
//  Description:    Returns the module's current status byte.                //
//---------------------------------------------------------------------------//
byte NmcGetStat(byte addr)
{
return mod[addr].stat;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcGetStatItems                                          //
//  Return Value:   Byte specifying default status items                     //
//  Parameters:     addr: module address (1-32)                              //
//  Description:    Returns the byte specifying which default status items   //
//                  are returned in a module's status data packet.           //
//---------------------------------------------------------------------------//
byte NmcGetStatItems(byte addr)
{
return mod[addr].statusitems;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcGetModType                                            //
//  Return Value:   module type (0=PIC-SERVO, 2=PIC-IO, 3=PIC-STEP)          //
//  Parameters:     addr: module address (1-32)                              //
//  Description:    Returns the module type of a specified module.           //
//---------------------------------------------------------------------------//
byte NmcGetModType(byte addr)
{
return mod[addr].modtype;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcGetModVer                                             //
//  Return Value:   module version (stored as integer is single byte)        //
//  Parameters:     addr: module type (1-32)                                 //
//  Description:    Returns the firmware version of the specified module.    //
//---------------------------------------------------------------------------//
byte NmcGetModVer(byte addr)
{
return mod[addr].modver;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcGetGroupAddr                                          //
//  Return Value:   group address (0x80 - 0xFF)                              //
//  Parameters:     addr: module address (1 - 32)                            //
//  Description:    Returns the group address of the specified module.       //
//---------------------------------------------------------------------------//
byte NmcGetGroupAddr(byte addr)
{
return mod[addr].groupaddr;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcGroupLeader                                           //
//  Return Value:   0=module is not group leader, 1=module is group leader   //
//  Parameters:     addr: module address (1-32)                              //
//  Description:    Returns true if the specified module is a group leader,  //
//                  otherwise false.                                         //
//---------------------------------------------------------------------------//
bool NmcGroupLeader(byte addr)
{
return mod[addr].groupleader;
}


//---------------------------------------------------------------------------//
//  Function Name:  NmcShutdown                                              //
//  Return Value:   None                                                     //
//  Parameters:     None                                                     //
//  Description:    Resets controllers assuming a default group address of   //
//                  0xFF, then closes COM port in use.                       //
//---------------------------------------------------------------------------//
void NmcShutdown(QString* error_string)
{
//int retval;

//retval = MessageBox(NULL, "Do you wish to reset the NMC network?",
//                                 "",MB_YESNO | MB_DEFBUTTON1 | MB_TASKMODAL);
//
//if (retval == IDYES)
//	{
//	if (ComPort!=INVALID_HANDLE_VALUE && ComPort!=NULL) NmcHardReset(0xFF);
//    }
//else
//	{
//    if (BaudRate != 19200)
//      	{
//      	ErrorMsgBox("Returning NMC Baud rate to 19200 (default)");
//      	NmcChangeBaud(0xFF, 19200);
//      	}
//    }
if (ComPort!=INVALID_HANDLE_VALUE && ComPort!=NULL) NmcHardReset(0xFF,error_string);

nummod = 0;    
SioClose(ComPort);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


void nap(unsigned long msec) {
     QMutex dummy;
     dummy.lock();
     QWaitCondition W;
     W.wait(&dummy,msec);
     return;
}

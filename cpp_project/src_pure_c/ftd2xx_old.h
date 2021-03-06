

#include <stdarg.h>
#include "windef.h"
#include "winbase.h"

#ifndef FTD2XX_H
#define FTD2XX_H

//#define min(x, y) (x > y ? x : y)

typedef PVOID	FT_HANDLE;
typedef ULONG	FT_STATUS;

//
// Device status
//
enum {
    FT_OK,
    FT_INVALID_HANDLE,
    FT_DEVICE_NOT_FOUND,
    FT_DEVICE_NOT_OPENED,
    FT_IO_ERROR,
    FT_INSUFFICIENT_RESOURCES,
    FT_INVALID_PARAMETER,
    FT_INVALID_BAUD_RATE,

    FT_DEVICE_NOT_OPENED_FOR_ERASE,
    FT_DEVICE_NOT_OPENED_FOR_WRITE,
    FT_FAILED_TO_WRITE_DEVICE,
    FT_EEPROM_READ_FAILED,
    FT_EEPROM_WRITE_FAILED,
    FT_EEPROM_ERASE_FAILED,
	FT_EEPROM_NOT_PRESENT,
	FT_EEPROM_NOT_PROGRAMMED,
	FT_INVALID_ARGS,
	FT_NOT_SUPPORTED,
	FT_OTHER_ERROR,
	FT_DEVICE_LIST_NOT_READY,
};


#define FT_SUCCESS(status) ((status) == FT_OK)

//
// FT_OpenEx Flags
//

#define FT_OPEN_BY_SERIAL_NUMBER    1
#define FT_OPEN_BY_DESCRIPTION      2
#define FT_OPEN_BY_LOCATION			4

//
// FT_ListDevices Flags (used in conjunction with FT_OpenEx Flags
//

#define FT_LIST_NUMBER_ONLY			0x80000000
#define FT_LIST_BY_INDEX			0x40000000
#define FT_LIST_ALL					0x20000000

#define FT_LIST_MASK (FT_LIST_NUMBER_ONLY|FT_LIST_BY_INDEX|FT_LIST_ALL)

//
// Baud Rates
//

#define FT_BAUD_300			300
#define FT_BAUD_600			600
#define FT_BAUD_1200		1200
#define FT_BAUD_2400		2400
#define FT_BAUD_4800		4800
#define FT_BAUD_9600		9600
#define FT_BAUD_14400		14400
#define FT_BAUD_19200		19200
#define FT_BAUD_38400		38400
#define FT_BAUD_57600		57600
#define FT_BAUD_115200		115200
#define FT_BAUD_230400		230400
#define FT_BAUD_460800		460800
#define FT_BAUD_921600		921600

//
// Word Lengths
//

#define FT_BITS_8			(UCHAR) 8
#define FT_BITS_7			(UCHAR) 7
#define FT_BITS_6			(UCHAR) 6
#define FT_BITS_5			(UCHAR) 5

//
// Stop Bits
//

#define FT_STOP_BITS_1		(UCHAR) 0
#define FT_STOP_BITS_1_5	(UCHAR) 1
#define FT_STOP_BITS_2		(UCHAR) 2

//
// Parity
//

#define FT_PARITY_NONE		(UCHAR) 0
#define FT_PARITY_ODD		(UCHAR) 1
#define FT_PARITY_EVEN		(UCHAR) 2
#define FT_PARITY_MARK		(UCHAR) 3
#define FT_PARITY_SPACE		(UCHAR) 4

//
// Flow Control
//

#define FT_FLOW_NONE        0x0000
#define FT_FLOW_RTS_CTS     0x0100
#define FT_FLOW_DTR_DSR     0x0200
#define FT_FLOW_XON_XOFF    0x0400

//
// Purge rx and tx buffers
//
#define FT_PURGE_RX         1
#define FT_PURGE_TX         2

//
// Events
//

typedef void (*PFT_EVENT_HANDLER)(DWORD,DWORD);

#define FT_EVENT_RXCHAR		    1
#define FT_EVENT_MODEM_STATUS   2

//
// Timeouts
//

#define FT_DEFAULT_RX_TIMEOUT   300
#define FT_DEFAULT_TX_TIMEOUT   300

//
// Device types
//

typedef ULONG	FT_DEVICE;

enum {
    FT_DEVICE_BM,
    FT_DEVICE_AM,
    FT_DEVICE_100AX,
    FT_DEVICE_UNKNOWN,
	FT_DEVICE_2232C,
    FT_DEVICE_232R,
	FT_DEVICE_2232H,
	FT_DEVICE_4232H,
	FT_DEVICE_232H
};


#ifdef __cplusplus
extern "C" {
#endif

FT_STATUS WINAPI FT_Open(
	int deviceNumber,
	FT_HANDLE *pHandle
	);

FT_STATUS WINAPI FT_OpenEx(
    PVOID pArg1,
    DWORD Flags,
    FT_HANDLE *pHandle
    );

FT_STATUS WINAPI FT_ListDevices(
	PVOID pArg1,
	PVOID pArg2,
	DWORD Flags
	);

FT_STATUS WINAPI FT_Close(
    FT_HANDLE ftHandle
    );

FT_STATUS WINAPI FT_Read(
    FT_HANDLE ftHandle,
    LPVOID lpBuffer,
    DWORD dwBytesToRead,
    LPDWORD lpBytesReturned
    );

FT_STATUS WINAPI FT_Write(
    FT_HANDLE ftHandle,
    LPVOID lpBuffer,
    DWORD dwBytesToWrite,
    LPDWORD lpBytesWritten
    );

FT_STATUS WINAPI FT_IoCtl(
    FT_HANDLE ftHandle,
    DWORD dwIoControlCode,
    LPVOID lpInBuf,
    DWORD nInBufSize,
    LPVOID lpOutBuf,
    DWORD nOutBufSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped
    );


FT_STATUS WINAPI FT_SetBaudRate(
    FT_HANDLE ftHandle,
	ULONG BaudRate
	);


FT_STATUS WINAPI FT_SetDivisor(
    FT_HANDLE ftHandle,
	USHORT Divisor
	);


FT_STATUS WINAPI FT_SetDataCharacteristics(
    FT_HANDLE ftHandle,
	UCHAR WordLength,
	UCHAR StopBits,
	UCHAR Parity
	);


FT_STATUS WINAPI FT_SetFlowControl(
    FT_HANDLE ftHandle,
    USHORT FlowControl,
    UCHAR XonChar,
    UCHAR XoffChar
	);


FT_STATUS WINAPI FT_ResetDevice(
    FT_HANDLE ftHandle
	);


FT_STATUS WINAPI FT_SetDtr(
    FT_HANDLE ftHandle
	);


FT_STATUS WINAPI FT_ClrDtr(
    FT_HANDLE ftHandle
	);


FT_STATUS WINAPI FT_SetRts(
    FT_HANDLE ftHandle
	);


FT_STATUS WINAPI FT_ClrRts(
    FT_HANDLE ftHandle
	);


FT_STATUS WINAPI FT_GetModemStatus(
    FT_HANDLE ftHandle,
	ULONG *pModemStatus
	);


FT_STATUS WINAPI FT_SetChars(
    FT_HANDLE ftHandle,
	UCHAR EventChar,
	UCHAR EventCharEnabled,
	UCHAR ErrorChar,
	UCHAR ErrorCharEnabled
    );


FT_STATUS WINAPI FT_Purge(
    FT_HANDLE ftHandle,
	ULONG Mask
	);


FT_STATUS WINAPI FT_SetTimeouts(
    FT_HANDLE ftHandle,
	ULONG ReadTimeout,
	ULONG WriteTimeout
	);


FT_STATUS WINAPI FT_GetQueueStatus(
    FT_HANDLE ftHandle,
	DWORD *dwRxBytes
	);


FT_STATUS WINAPI FT_SetEventNotification(
    FT_HANDLE ftHandle,
	DWORD Mask,
	PVOID Param
	);


FT_STATUS WINAPI FT_GetStatus(
    FT_HANDLE ftHandle,
    DWORD *dwRxBytes,
    DWORD *dwTxBytes,
    DWORD *dwEventDWord
	);


FT_STATUS WINAPI FT_SetBreakOn(
    FT_HANDLE ftHandle
    );


FT_STATUS WINAPI FT_SetBreakOff(
    FT_HANDLE ftHandle
    );


FT_STATUS WINAPI FT_SetWaitMask(
    FT_HANDLE ftHandle,
    DWORD Mask
    );


FT_STATUS WINAPI FT_WaitOnMask(
    FT_HANDLE ftHandle,
    DWORD *Mask
    );


FT_STATUS WINAPI FT_GetEventStatus(
    FT_HANDLE ftHandle,
    DWORD *dwEventDWord
    );


FT_STATUS WINAPI FT_ReadEE(
    FT_HANDLE ftHandle,
	DWORD dwWordOffset,
    LPWORD lpwValue
	);


FT_STATUS WINAPI FT_WriteEE(
    FT_HANDLE ftHandle,
	DWORD dwWordOffset,
    WORD wValue
	);


FT_STATUS WINAPI FT_EraseEE(
    FT_HANDLE ftHandle
	);

//
// structure to hold program data for FT_Program function
//
typedef struct ft_program_data {

	DWORD Signature1;			// Header - must be 0x00000000 
	DWORD Signature2;			// Header - must be 0xffffffff
	DWORD Version;				// Header - FT_PROGRAM_DATA version
								//          0 = original
	                            //          1 = FT2232C extensions

	WORD VendorId;				// 0x0403
	WORD ProductId;				// 0x6001
	char *Manufacturer;			// "FTDI"
	char *ManufacturerId;		// "FT"
	char *Description;			// "USB HS Serial Converter"
	char *SerialNumber;			// "FT000001" if fixed, or NULL
	WORD MaxPower;				// 0 < MaxPower <= 500
	WORD PnP;					// 0 = disabled, 1 = enabled
	WORD SelfPowered;			// 0 = bus powered, 1 = self powered
	WORD RemoteWakeup;			// 0 = not capable, 1 = capable
	//
	// Rev4 (FT232B) extensions
	//
	UCHAR Rev4;					// non-zero if Rev4 chip, zero otherwise
	UCHAR IsoIn;				// non-zero if in endpoint is isochronous
	UCHAR IsoOut;				// non-zero if out endpoint is isochronous
	UCHAR PullDownEnable;		// non-zero if pull down enabled
	UCHAR SerNumEnable;			// non-zero if serial number to be used
	UCHAR USBVersionEnable;		// non-zero if chip uses USBVersion
	WORD USBVersion;			// BCD (0x0200 => USB2)
	//
	// Rev 5 (FT2232) extensions
	//
	UCHAR Rev5;					// non-zero if Rev5 chip, zero otherwise
	UCHAR IsoInA;				// non-zero if in endpoint is isochronous
	UCHAR IsoInB;				// non-zero if in endpoint is isochronous
	UCHAR IsoOutA;				// non-zero if out endpoint is isochronous
	UCHAR IsoOutB;				// non-zero if out endpoint is isochronous
	UCHAR PullDownEnable5;		// non-zero if pull down enabled
	UCHAR SerNumEnable5;		// non-zero if serial number to be used
	UCHAR USBVersionEnable5;	// non-zero if chip uses USBVersion
	WORD USBVersion5;			// BCD (0x0200 => USB2)
	UCHAR AIsHighCurrent;		// non-zero if interface is high current
	UCHAR BIsHighCurrent;		// non-zero if interface is high current
	UCHAR IFAIsFifo;			// non-zero if interface is 245 FIFO
	UCHAR IFAIsFifoTar;			// non-zero if interface is 245 FIFO CPU target
	UCHAR IFAIsFastSer;			// non-zero if interface is Fast serial
	UCHAR AIsVCP;				// non-zero if interface is to use VCP drivers
	UCHAR IFBIsFifo;			// non-zero if interface is 245 FIFO
	UCHAR IFBIsFifoTar;			// non-zero if interface is 245 FIFO CPU target
	UCHAR IFBIsFastSer;			// non-zero if interface is Fast serial
	UCHAR BIsVCP;				// non-zero if interface is to use VCP drivers
    //
	// Rev 6 (FT232R) extensions
	//
	UCHAR UseExtOsc;			// Use External Oscillator
	UCHAR HighDriveIOs;			// High Drive I/Os
	UCHAR EndpointSize;			// Endpoint size
	UCHAR PullDownEnableR;		// non-zero if pull down enabled
	UCHAR SerNumEnableR;		// non-zero if serial number to be used
	UCHAR InvertTXD;			// non-zero if invert TXD
	UCHAR InvertRXD;			// non-zero if invert RXD
	UCHAR InvertRTS;			// non-zero if invert RTS
	UCHAR InvertCTS;			// non-zero if invert CTS
	UCHAR InvertDTR;			// non-zero if invert DTR
	UCHAR InvertDSR;			// non-zero if invert DSR
	UCHAR InvertDCD;			// non-zero if invert DCD
	UCHAR InvertRI;				// non-zero if invert RI
	UCHAR Cbus0;				// Cbus Mux control
	UCHAR Cbus1;				// Cbus Mux control
	UCHAR Cbus2;				// Cbus Mux control
	UCHAR Cbus3;				// Cbus Mux control
	UCHAR Cbus4;				// Cbus Mux control
	UCHAR RIsD2XX;				// non-zero if using D2XX driver
	//
	// Rev 7 (FT2232H) Extensions
	//
	UCHAR PullDownEnable7;		// non-zero if pull down enabled
	UCHAR SerNumEnable7;		// non-zero if serial number to be used
	UCHAR ALSlowSlew;			// non-zero if AL pins have slow slew
	UCHAR ALSchmittInput;		// non-zero if AL pins are Schmitt input
	UCHAR ALDriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR AHSlowSlew;			// non-zero if AH pins have slow slew
	UCHAR AHSchmittInput;		// non-zero if AH pins are Schmitt input
	UCHAR AHDriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR BLSlowSlew;			// non-zero if BL pins have slow slew
	UCHAR BLSchmittInput;		// non-zero if BL pins are Schmitt input
	UCHAR BLDriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR BHSlowSlew;			// non-zero if BH pins have slow slew
	UCHAR BHSchmittInput;		// non-zero if BH pins are Schmitt input
	UCHAR BHDriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR IFAIsFifo7;			// non-zero if interface is 245 FIFO
	UCHAR IFAIsFifoTar7;		// non-zero if interface is 245 FIFO CPU target
	UCHAR IFAIsFastSer7;		// non-zero if interface is Fast serial
	UCHAR AIsVCP7;				// non-zero if interface is to use VCP drivers
	UCHAR IFBIsFifo7;			// non-zero if interface is 245 FIFO
	UCHAR IFBIsFifoTar7;		// non-zero if interface is 245 FIFO CPU target
	UCHAR IFBIsFastSer7;		// non-zero if interface is Fast serial
	UCHAR BIsVCP7;				// non-zero if interface is to use VCP drivers
	UCHAR PowerSaveEnable;		// non-zero if using BCBUS7 to save power for self-powered designs
	//
	// Rev 8 (FT4232H) Extensions
	//
	UCHAR PullDownEnable8;		// non-zero if pull down enabled
	UCHAR SerNumEnable8;		// non-zero if serial number to be used
	UCHAR ASlowSlew;			// non-zero if AL pins have slow slew
	UCHAR ASchmittInput;		// non-zero if AL pins are Schmitt input
	UCHAR ADriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR BSlowSlew;			// non-zero if AH pins have slow slew
	UCHAR BSchmittInput;		// non-zero if AH pins are Schmitt input
	UCHAR BDriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR CSlowSlew;			// non-zero if BL pins have slow slew
	UCHAR CSchmittInput;		// non-zero if BL pins are Schmitt input
	UCHAR CDriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR DSlowSlew;			// non-zero if BH pins have slow slew
	UCHAR DSchmittInput;		// non-zero if BH pins are Schmitt input
	UCHAR DDriveCurrent;		// valid values are 4mA, 8mA, 12mA, 16mA
	UCHAR ARIIsTXDEN;			// non-zero if port A uses RI as RS485 TXDEN
	UCHAR BRIIsTXDEN;			// non-zero if port B uses RI as RS485 TXDEN
	UCHAR CRIIsTXDEN;			// non-zero if port C uses RI as RS485 TXDEN
	UCHAR DRIIsTXDEN;			// non-zero if port D uses RI as RS485 TXDEN
	UCHAR AIsVCP8;				// non-zero if interface is to use VCP drivers
	UCHAR BIsVCP8;				// non-zero if interface is to use VCP drivers
	UCHAR CIsVCP8;				// non-zero if interface is to use VCP drivers
	UCHAR DIsVCP8;				// non-zero if interface is to use VCP drivers

	
	} FT_PROGRAM_DATA, *PFT_PROGRAM_DATA;


FT_STATUS WINAPI FT_EE_Program(
    FT_HANDLE ftHandle,
	PFT_PROGRAM_DATA pData
	);


FT_STATUS WINAPI FT_EE_ProgramEx(
    FT_HANDLE ftHandle,
	PFT_PROGRAM_DATA pData,
	char *Manufacturer,
	char *ManufacturerId,
	char *Description,
	char *SerialNumber
	);


FT_STATUS WINAPI FT_EE_Read(
    FT_HANDLE ftHandle,
	PFT_PROGRAM_DATA pData
	);


FT_STATUS WINAPI FT_EE_ReadEx(
    FT_HANDLE ftHandle,
	PFT_PROGRAM_DATA pData,
	char *Manufacturer,
	char *ManufacturerId,
	char *Description,
	char *SerialNumber
	);


FT_STATUS WINAPI FT_EE_UASize(
    FT_HANDLE ftHandle,
	LPDWORD lpdwSize
	);


FT_STATUS WINAPI FT_EE_UAWrite(
    FT_HANDLE ftHandle,
	PUCHAR pucData,
	DWORD dwDataLen
	);


FT_STATUS WINAPI FT_EE_UARead(
    FT_HANDLE ftHandle,
	PUCHAR pucData,
	DWORD dwDataLen,
	LPDWORD lpdwBytesRead
	);


FT_STATUS WINAPI FT_SetLatencyTimer(
    FT_HANDLE ftHandle,
    UCHAR ucLatency
    );


FT_STATUS WINAPI FT_GetLatencyTimer(
    FT_HANDLE ftHandle,
    PUCHAR pucLatency
    );


FT_STATUS WINAPI FT_SetBitMode(
    FT_HANDLE ftHandle,
    UCHAR ucMask,
	UCHAR ucEnable
    );


FT_STATUS WINAPI FT_GetBitMode(
    FT_HANDLE ftHandle,
    PUCHAR pucMode
    );


FT_STATUS WINAPI FT_SetUSBParameters(
    FT_HANDLE ftHandle,
    ULONG ulInTransferSize,
    ULONG ulOutTransferSize
    );

FT_STATUS WINAPI FT_SetDeadmanTimeout(
    FT_HANDLE ftHandle,
	ULONG ulDeadmanTimeout
    );

FT_STATUS WINAPI FT_GetDeviceInfo(
    FT_HANDLE ftHandle,
    FT_DEVICE *lpftDevice,
	LPDWORD lpdwID,
	PCHAR SerialNumber,
	PCHAR Description,
	LPVOID Dummy
    );


FT_STATUS WINAPI FT_StopInTask(
    FT_HANDLE ftHandle
    );


FT_STATUS WINAPI FT_RestartInTask(
    FT_HANDLE ftHandle
    );


FT_STATUS WINAPI FT_SetResetPipeRetryCount(
    FT_HANDLE ftHandle,
	DWORD dwCount
    );


FT_STATUS WINAPI FT_ResetPort(
    FT_HANDLE ftHandle
    );


FT_STATUS WINAPI FT_CyclePort(
    FT_HANDLE ftHandle
    );


//
// Win32-type functions
//


FT_HANDLE WINAPI FT_W32_CreateFile(
	LPCSTR					lpszName,
	DWORD					dwAccess,
	DWORD					dwShareMode,
	LPSECURITY_ATTRIBUTES	lpSecurityAttributes,
	DWORD					dwCreate,
	DWORD					dwAttrsAndFlags,
	HANDLE					hTemplate
	);


BOOL WINAPI FT_W32_CloseHandle(
    FT_HANDLE ftHandle
	);


BOOL WINAPI FT_W32_ReadFile(
    FT_HANDLE ftHandle,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesReturned,
	LPOVERLAPPED lpOverlapped
    );


BOOL WINAPI FT_W32_WriteFile(
    FT_HANDLE ftHandle,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesWritten,
	LPOVERLAPPED lpOverlapped
    );


DWORD WINAPI FT_W32_GetLastError(
    FT_HANDLE ftHandle
    );


BOOL WINAPI FT_W32_GetOverlappedResult(
    FT_HANDLE ftHandle,
	LPOVERLAPPED lpOverlapped,
    LPDWORD lpdwBytesTransferred,
	BOOL bWait
    );


BOOL WINAPI FT_W32_CancelIo(
    FT_HANDLE ftHandle
    );


//
// Win32 COMM API type functions
//
typedef struct _FTCOMSTAT {
    DWORD fCtsHold : 1;
    DWORD fDsrHold : 1;
    DWORD fRlsdHold : 1;
    DWORD fXoffHold : 1;
    DWORD fXoffSent : 1;
    DWORD fEof : 1;
    DWORD fTxim : 1;
    DWORD fReserved : 25;
    DWORD cbInQue;
    DWORD cbOutQue;
} FTCOMSTAT, *LPFTCOMSTAT;

typedef struct _FTDCB {
    DWORD DCBlength;      /* sizeof(FTDCB)                   */
    DWORD BaudRate;       /* Baudrate at which running       */
    DWORD fBinary: 1;     /* Binary Mode (skip EOF check)    */
    DWORD fParity: 1;     /* Enable parity checking          */
    DWORD fOutxCtsFlow:1; /* CTS handshaking on output       */
    DWORD fOutxDsrFlow:1; /* DSR handshaking on output       */
    DWORD fDtrControl:2;  /* DTR Flow control                */
    DWORD fDsrSensitivity:1; /* DSR Sensitivity              */
    DWORD fTXContinueOnXoff: 1; /* Continue TX when Xoff sent */
    DWORD fOutX: 1;       /* Enable output X-ON/X-OFF        */
    DWORD fInX: 1;        /* Enable input X-ON/X-OFF         */
    DWORD fErrorChar: 1;  /* Enable Err Replacement          */
    DWORD fNull: 1;       /* Enable Null stripping           */
    DWORD fRtsControl:2;  /* Rts Flow control                */
    DWORD fAbortOnError:1; /* Abort all reads and writes on Error */
    DWORD fDummy2:17;     /* Reserved                        */
    WORD wReserved;       /* Not currently used              */
    WORD XonLim;          /* Transmit X-ON threshold         */
    WORD XoffLim;         /* Transmit X-OFF threshold        */
    BYTE ByteSize;        /* Number of bits/byte, 4-8        */
    BYTE Parity;          /* 0-4=None,Odd,Even,Mark,Space    */
    BYTE StopBits;        /* 0,1,2 = 1, 1.5, 2               */
    char XonChar;         /* Tx and Rx X-ON character        */
    char XoffChar;        /* Tx and Rx X-OFF character       */
    char ErrorChar;       /* Error replacement char          */
    char EofChar;         /* End of Input character          */
    char EvtChar;         /* Received Event character        */
    WORD wReserved1;      /* Fill for now.                   */
} FTDCB, *LPFTDCB;

typedef struct _FTTIMEOUTS {
    DWORD ReadIntervalTimeout;          /* Maximum time between read chars. */
    DWORD ReadTotalTimeoutMultiplier;   /* Multiplier of characters.        */
    DWORD ReadTotalTimeoutConstant;     /* Constant in milliseconds.        */
    DWORD WriteTotalTimeoutMultiplier;  /* Multiplier of characters.        */
    DWORD WriteTotalTimeoutConstant;    /* Constant in milliseconds.        */
} FTTIMEOUTS,*LPFTTIMEOUTS;



BOOL WINAPI FT_W32_ClearCommBreak(
    FT_HANDLE ftHandle
	);


BOOL WINAPI FT_W32_ClearCommError(
    FT_HANDLE ftHandle,
	LPDWORD lpdwErrors,
    LPFTCOMSTAT lpftComstat
	);


BOOL WINAPI FT_W32_EscapeCommFunction(
    FT_HANDLE ftHandle,
	DWORD dwFunc
	);


BOOL WINAPI FT_W32_GetCommModemStatus(
    FT_HANDLE ftHandle,
	LPDWORD lpdwModemStatus
	);


BOOL WINAPI FT_W32_GetCommState(
    FT_HANDLE ftHandle,
    LPFTDCB lpftDcb
	);


BOOL WINAPI FT_W32_GetCommTimeouts(
    FT_HANDLE ftHandle,
    FTTIMEOUTS *pTimeouts
	);


BOOL WINAPI FT_W32_PurgeComm(
    FT_HANDLE ftHandle,
	DWORD dwMask
	);


BOOL WINAPI FT_W32_SetCommBreak(
    FT_HANDLE ftHandle
	);


BOOL WINAPI FT_W32_SetCommMask(
    FT_HANDLE ftHandle,
    ULONG ulEventMask
    );


BOOL WINAPI FT_W32_GetCommMask(
    FT_HANDLE ftHandle,
    LPDWORD lpdwEventMask
    );

	
BOOL WINAPI FT_W32_SetCommState(
    FT_HANDLE ftHandle,
    LPFTDCB lpftDcb
	);


BOOL WINAPI FT_W32_SetCommTimeouts(
    FT_HANDLE ftHandle,
    FTTIMEOUTS *pTimeouts
	);


BOOL WINAPI FT_W32_SetupComm(
    FT_HANDLE ftHandle,
	DWORD dwReadBufferSize,
	DWORD dwWriteBufferSize
	);


BOOL WINAPI FT_W32_WaitCommEvent(
    FT_HANDLE ftHandle,
    PULONG pulEvent,
	LPOVERLAPPED lpOverlapped
    );


//
// Device information
//

typedef struct _ft_device_list_info_node {
	ULONG Flags;
    ULONG Type;
	ULONG ID;
	DWORD LocId;
	char SerialNumber[16];
	char Description[64];
	FT_HANDLE ftHandle;
} FT_DEVICE_LIST_INFO_NODE;

// Device information flags
enum {
	FT_FLAGS_OPENED = 1,
	FT_FLAGS_HISPEED = 2
};


FT_STATUS WINAPI FT_CreateDeviceInfoList(
	LPDWORD lpdwNumDevs
	);

FT_STATUS WINAPI FT_GetDeviceInfoList(
	FT_DEVICE_LIST_INFO_NODE *pDest,
	LPDWORD lpdwNumDevs
	);

FT_STATUS WINAPI FT_GetDeviceInfoDetail(
	DWORD dwIndex,
	LPDWORD lpdwFlags,
	LPDWORD lpdwType,
	LPDWORD lpdwID,
	LPDWORD lpdwLocId,
	LPVOID lpSerialNumber,
	LPVOID lpDescription,
	FT_HANDLE *pftHandle
	);


//
// Version information
//


FT_STATUS WINAPI FT_GetDriverVersion(
    FT_HANDLE ftHandle,
	LPDWORD lpdwVersion
	);


FT_STATUS WINAPI FT_GetLibraryVersion(
	LPDWORD lpdwVersion
	);



FT_STATUS WINAPI FT_Rescan(
	void
	);


FT_STATUS WINAPI FT_Reload(
	WORD wVid,
	WORD wPid
	);

	
FT_STATUS WINAPI FT_GetComPortNumber(
    FT_HANDLE ftHandle,
	LPLONG	lpdwComPortNumber
	);

#ifdef __cplusplus
}
#endif


#endif  /* FTD2XX_H */


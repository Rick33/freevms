/*
	****************************************************************

		Copyright (c) 1992, Carnegie Mellon University

		All Rights Reserved

	Permission  is  hereby  granted   to  use,  copy,  modify,  and
	distribute  this software  provided  that the  above  copyright
	notice appears in  all copies and that  any distribution be for
	noncommercial purposes.

	Carnegie Mellon University disclaims all warranties with regard
	to this software.  In no event shall Carnegie Mellon University
	be liable for  any special, indirect,  or consequential damages
	or any damages whatsoever  resulting from loss of use, data, or
	profits  arising  out of  or in  connection  with  the  use  or
	performance of this software.

	****************************************************************
*/
//TITLE "Input/Output Utilities"
//++
//
// Module:
//
//	IOUTIL
//
// Facility:
//
//	Input and Output utility routines
//
// Abstract:
//
//	Provides a standard library of routines for input and output of
//	addresses as decimal and hexidecimal strings.
//
// Author:
//
//	Vince Fuller, CMU-CSD, April 1986
//	Copyright (c) 1986,1987, Vince Fuller and Carnegie-Mellon University
//
// Modification history:
//
// 2.2	7-Jan-1992	John Clement
//	Added LOG$Flush for immediate output
//
// 2.1	24-Dec-1991	Henry W. Miller		USBR
//	In LOG_OUTPUT and ACT_OUTPUT, use configurable variables LOG_THRESHOLD
//	and ACT_THRESHOLD to decide when to $FLUSH.  After all, we are
//	having a drought.
//
// 2.0D	05-Aug-1991	Henry W. Miller		USBR
//	In LOG_OUTPUT and ACT_OUTPUT, only $FLUSH() when a threshold of
//	2048 bytes has been hit.  Should speed up logging considerably.
//	Please excuse disgusting imagery.
//
// 2.0C	09-Jul-1991	Henry W. Miller		USBR
//	Added LIB for VMS 5.4.
//
// 2.0B	25-Mar-1991	Henry W. Miller		USBR
//	In LOG_OPEN(), print out FAB or RAB STV if error.
//	Define INIT_DYNDESC macro locally - $INIT_DYNDESC not working for
//	some reason!!
//
// 2.0  20-Oct-1989	Bruce R. Miller		CMU NetDev
//	Added code for doing activity file logging by basically
//	duplicating the LOG file code.
//
// 1.9  19-Nov-87, Edit by VAF
//	Use new $ACPWAKE macro.
//
// 1.8  24-Mar-87, Edit by VAF
//	Call RESET_PROCNAME to reset process name in exit handler.
//
// 1.7  24-Feb-87, Edit by VAF
//	Move QL_FAO and message queue management routines in here.
//	Fix a couple of misuses of LOG_STATE.
//
// 1.6  17-Feb-87, Edit by VAF
//	Fix a bunch of problems with error logging.
//
// 1.5  16-Feb-87, Edit by VAF
//	Fix ERROR_FAO/FATAL_FAO to add time+date and EOL before writing to
//	the log file.
//
// 1.4   9-Feb-87, Edit by VAF
//	Flush Error_Processor. New error handling routines are ERROR_FAO and
//	FATAL_FAO. TCPMACROS updated to use these for references to old macros.
//
// 1.3   6-Feb-87, Edit by VAF
//	Change exiting message.
//
// 1.2  30-Sep-86, Edit by VAF
//	Have exception handler send message to opr in addition to logging it
//	to the log file.
//	Have error handler always append message to log file.
//	Have exit handler close log file if it is open.
//
// 1.1  30-May-86, Edit by VAF
//	Get rid of PRINT_MSG routine - make synonymous with OPR_FAO.
//
//--


#if 0
MODULE IOUTIL(IDENT="2.2",LANGUAGE(BLISS32),
	      ADDRESSING_MODE(EXTERNAL=LONG_RELATIVE,
			      NONEXTERNAL=LONG_RELATIVE),
	      LIST(NOREQUIRE,ASSEMBLY,OBJECT,BINARY),
	      OPTIMIZE,OPTLEVEL=3,ZIP)=
#endif

#include "SYS$LIBRARY:STARLET";
//LIBRARY "SYS$LIBRARY:LIB";			// JC
//not yet#include "CMUIP_SRC:[CENTRAL]NETXPORT";
#include <cmuip/central/include/netcommon.h>
#include "tcpmacros.h"
#include "structure.h"


MACRO APPCHR(CHR,DPTR,DCNT,OCNT)
    if ((DCNT=.DCNT-1) > 0)
	{
	OCNT = OCNT+1;
	CH$WCHAR_A(CHR,DPTR);
	} %;

MACRO
    Init_DynDesc (D)
	{
	$BBLOCK [D, dsc$w_length]	= 0;
	$BBLOCK [D, dsc$b_dtype]	= DSC$K_DTYPE_T;
	$BBLOCK [D, dsc$b_class]	= DSC$K_CLASS_D;
	$BBLOCK [D, dsc$a_pointer]	= 0;
	}%;

signed long
	ACT_THRESHOLD	 = 512,
	LOG_THRESHOLD	 = 512 ;


void APP}_DEC(DPTR,DCNT,NUM,OUTCNT)

// Append a decimal value to a string
//   DPTR - Address of pointer to destination string (updated on return)
//   DCNT - Address of count of bytes remaining in destination string (updated)
//   NUM - Value to output
//   OUTCNT - Address of count of bytes output (updated)

    {
    signed long
	DIV,DIG,REM,VAL,FLAG;
    if (NUM == 0)
	{
	APPCHR(%C"0",DPTR,DCNT,OUTCNT);
	return;
	};
    DIV = 1000000000;			// Highest pwr of 10 in 32 bits
    VAL = NUM;
    if (VAL LSS 0)
	{
	VAL = -1*.VAL;
	APPCHR(%C"-",DPTR,DCNT,OUTCNT);
	};
    FLAG = 0;
    while (DIV > 0)
	{
	DIG = VAL/.DIV;
	REM = VAL MOD DIV;
	DIV = DIV/10;
	if ((DIG != 0) || (FLAG != 0))
	    {
	    FLAG = FLAG+1;
	    APPCHR(DIG+%C"0",DPTR,DCNT,OUTCNT);
	    };
	VAL = REM;
	};
    }

VOID ASCII_DEC_BYTES(struct DESC$STR * DESC,COUNT,SOURCE,LEN)

// Write a string of decimal bytes to a string descriptor.

    {
    signed long
	CPTR,CURBYTE,DPTR,DCNT,OUTCNT;
    OUTCNT = 0;
    CPTR = CH$PTR(SOURCE);
    DCNT = DESC->dsc$w_length;
    DPTR = CH$PTR(DESC->dsc$a_pointer);
    for (I=(COUNT-1);I>=0;I--)
	{
	CURBYTE = CH$RCHAR_A(CPTR);
	APP}_DEC(DPTR,DCNT,CURBYTE,OUTCNT);
	if (I != 0)
	    APPCHR(%C".",DPTR,DCNT,OUTCNT);
	};
    if (LEN != 0)
	.LEN = MIN(OUTCNT,DESC->dsc$w_length);
    }

APP}_HEX(DPTR,DCNT,NUM,OUTCNT,SIZE)

// Append a hexidecimal value to a string

    {
    BUILTIN
	ROT;
    signed long
	DIG,VAL;
    VAL = ROT(NUM,(8-.SIZE)*4); // Position first digit
    for (I=(SIZE-1);I>=0;I--)
	{
	VAL = ROT(VAL,4);	// Rotate highest order 4 bits to lowest
	DIG = VAL<0,4>;	// Get the digit
	if (DIG <= 9)
	    DIG = %C"0"+.DIG
	else
	    DIG = %C"A"+.DIG-10;
	APPCHR(DIG,DPTR,DCNT,OUTCNT);
	}
    }

void ASCII_HEX_BYTES(struct DESC$STR * DESC,COUNT,SOURCE,LEN)

// Write a string of hexidecimal bytes to a string descriptor.

    {
    signed long
	CPTR,CURBYTE,DPTR,DCNT,OUTCNT;
    CPTR = CH$PTR(SOURCE);
    DCNT = DESC->dsc$w_length;
    DPTR = CH$PTR(DESC->dsc$a_pointer);
    OUTCNT = 0;
    for (I=(COUNT-1);I>=0;I--)
	{
	CURBYTE = CH$RCHAR_A(CPTR);
	APP}_HEX(DPTR,DCNT,CURBYTE,OUTCNT,2);
	if (I != 0)
	    APPCHR(%C"-",DPTR,DCNT,OUTCNT);
	};
    if (LEN != 0)
	.LEN = MIN(OUTCNT,DESC->dsc$w_length);
    }

FORWARD ROUTINE
    GET_DEC_NUM;

GET_IP_ADDR(CPTR,VAL)

// Convert an text internet address (a.b.c.d) into binary form.
// CPTR contains the address of a pointer to the text.
// VAL is the address of where to put the value.
// Returns -1 on failure, or terminating character (GEQ 0)
// N.B. Assumes that Internet addresses are 4 bytes long.

    {
    signed long
    	DPTR,NVAL,CHR;
    DPTR = CH$PTR(VAL);
    for (I=3;I>=0;I--)
	{
	if ((CHR = GET_DEC_NUM(CPTR,NVAL)) LSS 0)
	    return -1;
	CH$WCHAR_A(NVAL,DPTR);
	if (I != 0)
	    if (CH$RCHAR_A(CPTR) != %C".")
		return -1;
	};
    return CHR;
    }

GET_DEC_NUM(CPTR,VAL)

// Read a decimal number from a string into binary form.
// CPTR is the address of a string pointer to the numeric text.
// VAL is the address of where to put the value.
// Returns:
//   -1 on failure, first non-blank character is not numeric
//   >=0 on success, returning the terminating character.
// CPTR is updated to point at the terminating character.
// Currently only handles unsigned decimal values.

    {
    signed long
    	CHR,RVAL,LPTR;
    LPTR = ..CPTR;
    DO
	CHR = CH$RCHAR_A(LPTR)
    WHILE (CHR == %C" ");
    if ((CHR LSS %C"0') || (CHR > %C'9"))
	return -1;
    RVAL = 0;
    while ((CHR GEQ %C"0') && (CHR <= %C'9"))
	{
	RVAL = RVAL*10+(CHR-%C"0");
	.CPTR = LPTR;
	CHR = CH$RCHAR_A(LPTR);
	};
    VAL = RVAL;
    return CHR;
    }

FORWARD ROUTINE
    GET_HEX_NUM;

GET_HEX_BYTES(NUMBYT,CPTR,DEST)

// Read a hexidecimal byte string.
// Returns -1 on failure, or terminating character. CPTR updated to point
// at the terminating character.
// Octets must be separated by the character "-"

    {
    signed long
	CVAL,
	LPTR,
	TCHR,
	DPTR;
    DPTR = CH$PTR(DEST);
    for (I=(NUMBYT-1);I>=0;I--)
	{
	if ((TCHR=GET_HEX_NUM(CPTR,CVAL)) LSS 0)
	    return -1;
	CH$WCHAR_A(CVAL,DPTR);
	if (I != 0)
	    if (CH$RCHAR_A(CPTR) != %C"-")
		return -1;
	};
    return TCHR;
    }

GET_HEX_NUM(INPTR,VAL)

// Read a hexidecimal number from a string into binary form.
// CPTR is the address of a string pointer to the numeric text.
// VAL is the address of where to put the value.
// Returns:
//   -1 on failure, first non-blank character is not numeric
//   >=0 on success, returning the terminating character.
// INPTR is updated to point to the termiating character.

    {
    signed long
    	CHR,RVAL,NCHR,CPTR;
    CPTR = ..INPTR;
    DO
	CHR = CH$RCHAR_A(CPTR)
    WHILE (CHR == %C" ");
    RVAL = 0;
    NCHR = 0;
    while ((0 == 0))
	{
	signed long
	    CVAL;
	if ((CHR GEQ %C"0') && (CHR <= %C'9"))
	    CVAL = CHR-%C"0"
	else
	    if ((CHR GEQ %C"a') && (CHR <= %C'f"))
		CVAL = CHR-%C"a"+10
	    else
		if ((CHR GEQ %C"A') && (CHR <= %C'F"))
		    CVAL = CHR-%C"A"+10
		else
		    break;
	NCHR = NCHR+1;
	RVAL = RVAL^4+.CVAL;
	.INPTR = CPTR;
	CHR = CH$RCHAR_A(CPTR);
	};
    if (NCHR == 0)
	return -1;
    VAL = RVAL;
    return CHR;
    }


//SBTTL "Log file handling routines"

static signed long
    LOGFAB : $FAB(FNM = "INET$LOG:",
    		  FAC = PUT,
		  SHR = GET,
		  FOP = (SQO),
		  RFM = STMLF,			// JC
		  ORG = SEQ),
    LOGRAB : $RAB(FAB = LOGFAB);

signed long
    LOG_STATE  = 0;

MACRO
    TRUE = (0 == 0) %,
    FALSE = (0 == 1) %;

FORWARD ROUTINE
 VOID    OPR_FAO;

LOG_OPEN (void)

// Open the log/trace file for debug & trace recording.
// Output: LOGFAB setup for stream output.
// Returns: TRUE if successfully opened.

    {
    signed long
	RC;
    RC = $CREATE(FAB = LOGFAB);
    if (! RC)
	{
	OPR$FAO("Log file $CREATE failed, RC = !XL, STV = !XL",
	    RC, LOGFAB->FAB$L_STV);
	return FALSE;
	};
    RC = $CONNECT(RAB = LOGRAB);
    if (! RC)
	{
	OPR$FAO("Log file $CONNECT failed, RC = !XL, STV = !XL",
	    RC, LOGRAB->FAB$L_STV);
	return FALSE;
	};
    return TRUE;
    }

LOG_CLOSE (void)
    {
    if (LOG_STATE != 0)
	{
	$DISCONNECT(RAB = LOGRAB);
	$CLOSE(FAB = LOGFAB);
	return TRUE;
	}
    else
	return FALSE;
    }

FORWARD ROUTINE
 VOID    LOG_FAO;

void LOG_CHANGE(STATE)
    {
    if (STATE != 0)
	{			// He wants it open now
	if (LOG_STATE == 0)
	    {		// Not open - open it now
	    if (LOG_OPEN())
		{
		LOG_STATE = STATE;
		LOG$FAO("!%T Logging enabled!/",0);
		};
	    };
	LOG$FAO("!%T Log event mask set to !XL!/",0,state);
	}
    else
	{			// He wants it closed
	if (LOG_STATE != 0)
	    {		// It's open - close it now
	    LOG$FAO("!%T Logging disabled!/",0);
	    LOG_CLOSE();
	    LOG_STATE = STATE;	// Set new log state
	    };
	};
    }

void LOG_OUTPUT(OUTDESC)

// Output a string to the log file.
// OUTDESC is the address of a string descriptor.

    {
    OWN
	logcount	:	initial(0) ;
    MAP
	struct DESC$STR * OUTDESC;
    if (LOG_STATE != 0)
	{
	signed long
	    RC;

	LOGRAB->RAB$W_RSZ = OUTDESC->dsc$w_length;
	LOGRAB->RAB$L_RBF = OUTDESC->dsc$a_pointer;
	LOGCOUNT = LOGCOUNT + OUTDESC->dsc$w_length ;

	RC = $PUT(RAB = LOGRAB);
//!!HACK!!// Take out this Flush!
	IF ( 	(LOGCOUNT > LOG_THRESHOLD)
	   OR	(LOG_STATE && LOG$FLUSH) )	// JC
	THEN {
	    RC = $FLUSH(RAB = LOGRAB);
	    LOGCOUNT = 0 ;
	    } ;
	};

    }

void LOG_FAO(CSTR)

// Do output to log file using $FAO to format parameters.

    {
    BUILTIN
	AP;
    signed long
	RC,
	DESC$STR_ALLOC(OUTDESC,1000);

    RC = $FAOL(CTRSTR = CSTR,
	       OUTLEN = OUTDESC->dsc$w_length,
	       OUTBUF = OUTDESC,
	       PRMLST = AP+8);
    if (RC)
	LOG_OUTPUT(OUTDESC)
    else
	OPR$FAO("LOG_FAO failure, error code is !XL",RC);
    }

void LOG_Time_Stamp (void)

// Output to the LOG device/file the current time.  Char string ends
// in a space (no crlf).
// VMS specific code.
// Exit:	none

    {
    LOG$FAO("!%T ",0);
    }



//SBTTL "Activity file handling routines"

static signed long
    ACTFAB : $FAB(FNM = "INET$ACTIVITY:",
    		  FAC = PUT,
		  SHR = GET,
		  FOP = (SQO),
		  ORG = SEQ),
    ACTRAB : $RAB(FAB = ACTFAB);

signed long
    ACT_STATE  = 0;

ACT_OPEN (void)

// Open the activity file for event recording.
// Output: ACTFAB setup for stream output.
// Returns: TRUE if successfully opened.

    {
    signed long
	RC;
    RC = $CREATE(FAB = ACTFAB);
    if (! RC)
	{
	OPR$FAO("Activity file $CREATE failed, RC = !XL",RC);
	return FALSE;
	};
    RC = $CONNECT(RAB = ACTRAB);
    if (! RC)
	{
	OPR$FAO("Activity file $CONNECT failed, RC = !XL",RC);
	return FALSE;
	};
    return TRUE;
    }

ACT_CLOSE (void)
    {
    if (ACT_STATE != 0)
	{
	$DISCONNECT(RAB = ACTRAB);
	$CLOSE(FAB = ACTFAB);
	return TRUE;
	}
    else
	return FALSE;
    }

FORWARD ROUTINE
 VOID    ACT_FAO;

void ACT_CHANGE(STATE)
    {
    if (STATE != 0)
	{			// e wants it open now
	if (ACT_STATE == 0)
	    {		// Not open - open it now
	    if (ACT_OPEN())
		{
		ACT_STATE = STATE;
		ACT$FAO("!%T Logging enabled!/",0);
		};
	    };
	ACT$FAO("!%T Log event mask set to !XL!/",0,state);
	}
    else
	{			// e wants it closed
	if (ACT_STATE != 0)
	    {		// It's open - close it now
	    ACT$FAO("!%T Logging disabled!/",0);
	    ACT_CLOSE();
	    ACT_STATE = STATE;	// Set new log state
	    };
	};
    }

void ACT_OUTPUT(OUTDESC)

// Output a string to the activity log file.
// OUTDESC is the address of a string descriptor.

    {
    OWN
	ACTCOUNT	 = 0 ;
    MAP
	struct DESC$STR * OUTDESC;
    if (ACT_STATE != 0)
	{
	signed long
	    RC;
	ACTRAB->RAB$W_RSZ = OUTDESC->dsc$w_length;
	ACTRAB->RAB$L_RBF = OUTDESC->dsc$a_pointer;
	ACTCOUNT = ACTCOUNT + OUTDESC->dsc$w_length ;

	RC = $PUT(RAB = ACTRAB);
	if ((ACTCOUNT > ACT_THRESHOLD))
	    {
	    RC = $FLUSH(RAB = ACTRAB);
	    ACTCOUNT = 0 ;
	    } ;
	};
    }

void ACT_FAO(CSTR)

// Do output to activity log file using $FAO to format parameters.

    {
    BUILTIN
	AP;
    signed long
	RC,
	DESC$STR_ALLOC(OUTDESC,1000);

    RC = $FAOL(CTRSTR = CSTR,
	       OUTLEN = OUTDESC->dsc$w_length,
	       OUTBUF = OUTDESC,
	       PRMLST = AP+8);
    if (RC)
	ACT_OUTPUT(OUTDESC)
    else
	OPR$FAO("ACT_FAO failure, error code is !XL",RC);
    }

void ACT_Time_Stamp (void)

// Output to the activity log device/file the current time.  Char string ends
// in a space (no crlf).
// VMS specific code.
// Exit:	none

    {
    ACT$FAO("!%T ",0);
    }

//sbttl "Send messages to the Central VMS operator"
/*

Function:

	Send messages to the operators console & those terminals defined
	as operators.  used by network device interface code to tell the
	world about devices going offline etc.  Message sent to operator
	is prefixed with network name as derrived from "tcp$network_name"
	logical name.

Inputs:

	Text = address of mesage descriptor(vms string descriptor).

Implicit Inputs:

	MYname = initialized string-desc for network name.
Outputs:

	lbc (low bit clear) = success
	otherwise $sndopr error return.

Side Effects:

	operator terminals will receive the xmitted messages.
	if message_length > 128-size(tcp$network_name) then message will
	be truncated.
*/

Send_2_Operator(Text)
    {
    MAP
	struct DESC$STR * text;
    EXTERNAL
	MYNAME : DESC$STR;
    OWN
	Request_ID : LONG INITIAL(0);
    LITERAL
	MAXCHR = 1024;
    signed long
	MSGLEN,
	PTR,
	MSG : $BBLOCK->DSC$K_Z_BLN,
	MSGBUF : $BBLOCK->MAXCHR;
    BIND
	MSGTEXT = MSGBUF->OPC$L_MS_TEXT : VECTOR[,BYTE],
	NAMPTR = MYNAME->dsc$a_pointer,
	NAMLEN = MYNAME->dsc$w_length;

    MSGBUF->OPC$B_MS_TYPE = OPC$_RQ_RQST;
    MSGBUF->OPC$B_MS_TARGET = OPC$M_NM_CENTRL;
    MSGBUF->OPC$L_MS_RQSTID = Request_ID;
    Request_ID = Request_ID + 1;
    MSGLEN = TEXT->dsc$w_length;
    if (MSGLEN > MAXCHR)
	.MSGLEN = MAXCHR;
    CH$MOVE(MSGLEN,TEXT->dsc$a_pointer,MSGTEXT);
    MSG->dsc$w_length = 8+.MSGLEN;
    MSG->dsc$b_class = DSC$K_CLASS_Z;
    MSG->dsc$b_dtype = DSC$K_DTYPE_Z;
    MSG->dsc$a_pointer = MSGBUF;
    return $SNDOPR(MSGBUF=MSG);
    }

void OPR_FAO(CSTR) 

// Send a message to the VMS operator, using $FAO for output formatting.

    {
    BUILTIN
	AP;
    signed long
	RC,
	DESC$STR_ALLOC(OUTDESC,1000),
	DESC$STR_ALLOC(OPRDESC,1000);

    RC = $FAOL(CTRSTR = CSTR,
	       OUTLEN = OUTDESC->dsc$w_length,
	       OUTBUF = OUTDESC,
	       PRMLST = AP+8);
    if (! RC)
	$EXIT(CODE = RC);

// Reformat for console output

    RC = $FAO(%ASCID"IPACP: !AS",OPRDESC->dsc$w_length,OPRDESC,OUTDESC);
    if (! RC)
	$EXIT(CODE = RC);
    S}_2_OPERATOR(OPRDESC);
    }

signed long BIND
    PRINT_MSG = OPR_FAO;	// Synonym for host table module to use

//SBTTL "Error processing routines - ERROR_FAO, FATAL_FAO"

void ERROR_FAO(CSTR)
//
// Send a message to the console & log the error (OPR_FAO + LOG_FAO)
//
    {
    BUILTIN
	AP;
    signed long
	RC,
	OLDSTATE,
	DESC$STR_ALLOC(OUTDESC,1000),
	DESC$STR_ALLOC(OPRDESC,1000),
	DESC$STR_ALLOC(LOGDESC,1000);

// Format the message string

    RC = $FAOL(CTRSTR = CSTR,
	       OUTLEN = OUTDESC->dsc$w_length,
	       OUTBUF = OUTDESC,
	       PRMLST = AP+8);
    if (! RC)
	{
	OPR$FAO("ERROR_FAO failure, RC = !XL",RC);
	$EXIT(CODE = RC);
	};

// Format and send message to the operator

    RC = $FAO(%ASCID"?IPACP: !AS",OPRDESC->dsc$w_length,OPRDESC,OUTDESC);
    if (! RC)
	$EXIT(CODE = RC);
    S}_2_OPERATOR(OPRDESC);

// Format the message for logging - add time+date and EOL

    RC = $FAO(%ASCID"!%T !AS!/",LOGDESC->dsc$w_length,LOGDESC,0,OUTDESC);
    if (! RC)
	$EXIT(CODE = RC);

// Make sure we are logging something & log it

    OLDSTATE = LOG_STATE;
    LOG_CHANGE(%X"80000000" || LOG_STATE);
    LOG_OUTPUT(LOGDESC);
    LOG_CHANGE(OLDSTATE);
    }


void FATAL_FAO(CSTR)
//
// Same as above, except also exit the ACP.
//
    {
    BUILTIN
	AP;
    signed long
	RC,
	OLDSTATE,
	DESC$STR_ALLOC(OUTDESC,1000),
	DESC$STR_ALLOC(OPRDESC,1000),
	DESC$STR_ALLOC(LOGDESC,1000);

// Format the output string

    RC = $FAOL(CTRSTR = CSTR,
	       OUTLEN = OUTDESC->dsc$w_length,
	       OUTBUF = OUTDESC,
	       PRMLST = AP+8);
    if (! RC)
	{
	OPR$FAO("FATAL_FAO failure, RC = !XL",RC);
	$EXIT(CODE = RC);
	};

// Format & send message to the operator

    RC = $FAO(%ASCID"?IPACP: !AS",OPRDESC->dsc$w_length,OPRDESC,OUTDESC);
    if (! RC)
	$EXIT(CODE = RC);
    S}_2_OPERATOR(OPRDESC);

// Format it for logging

    RC = $FAO(%ASCID"!%T !AS!/",LOGDESC->dsc$w_length,LOGDESC,
	      0,OUTDESC);

// Make sure we are logging something & log it

    OLDSTATE = LOG_STATE;
    LOG_CHANGE(%X"80000000" || LOG_STATE);
    LOG_OUTPUT(LOGDESC);
    LOG_CHANGE(OLDSTATE);
    $EXIT(CODE = SS$_ABORT);
    }

//SBTTL "Queued message processing"
/*
    In order to debug timing-related problems, it is often necessary to log
    information from within AST routines. However, the very act of doing this
    logging can alter timing significantly. To avoid this problem, AST routine
    log messages are queued and later written in non-AST context.
*/

struct QB$ERRMSG
{
void *     EMQ$NEXT	;	// Next item on queue
void *     EMQ$LAST	;	// Previous item on queue
long long    EMQ$MDSC	;	// Message descriptor
    };

#define    QB$ERRMSG_SIZE sizeof(struct QB$ERRMSG)
#if 0
MACRO
    QB$ERRMSG = BLOCK->QB$ERRMSG_SIZE FIELD(QB$ERRMSG_FIELDS) %;
#endif

struct QH$ERRHDR 
{
void *     EM$QHEAD	;
void *    EM$QTAIL;
    };

#define ERRHDR_SIZE sizeof(struct QH$ERRHDR)
#if 0
MACRO
    QH$ERRHDR = BLOCK->QH$ERRHDR_SIZE FIELD(QH$ERRHDR_FIELDS) %;
#endif

static signed long
    ERR_MSG_Q : QH$ERRHDR PRESET([EM$QHEAD] = ERR_MSG_Q,
				 [EM$QTAIL] = ERR_MSG_Q);

void QL_FAO(CSTR)
//
// Format and queue an error message using $FAO and the message queue.
//
    {
    EXTERNAL
	SLEEPING;
    EXTERNAL ROUTINE
	MM$QBLK_GET,
void	MM$QBlk_Free ,
	LIB$SYS_FAOL ;
    BUILTIN
	AP,
	INSQUE;
    signed long
	struct DESC$STR * MDSC,
	struct queue_blk_structure(QB$ERRMSG_FIELDS) * QB,
	RC;

// Make sure logging is enabled

    if (LOG_STATE == 0)
	return;

// Allocate a queue block for the message

    QB = MM$QBLK_GET();
    MDSC = QB->EMQ$MDSC;
//    $INIT_DYNDESC(MDSC);
    INIT_DYNDESC(MDSC);

// Format the message

    RC = LIB$SYS_FAOL(CSTR, MDSC->dsc$w_length, MDSC, AP+8);
    if (! RC)
	{
	OPR$FAO("QL_FAO failure, RC = !XL",RC);
	MM$QBlk_Free(QB);
	return;
	};

// Insert the entry onto the queue

    INSQUE(QB,ERR_MSG_Q->EM$QTAIL);

// If the ACP is sleeping, issue a wakeup so messages will be written at the
// next interval.

    $ACPWAKE;
    }

void CHECK_ERRMSG_Q (void)
//
// Write all of the messages pending on the error message queue.
// Called from main TCP processing loop after all useful work has been done.
//
    {
    EXTERNAL ROUTINE
void	MM$QBlk_Free ,
	STR$FREE1_DX ;
    BUILTIN
	REMQUE;
    signed long
	struct queue_blk_structure(QB$ERRMSG_FIELDS) * QB,
	struct DESC$STR * MDSC;

// Scan the error message queue, writing each entry to the log file.

    while (REMQUE(ERR_MSG_Q->EM$QHEAD,QB) != EMPTY_QUEUE)
	{
	MDSC = QB->EMQ$MDSC;
	LOG_OUTPUT(MDSC);
	STR$FREE1_DX(MDSC);
	MM$QBlk_Free(QB);
	};
    }

//Sbttl "VMS Exit Handler"
/*

Function:

	Double check that all user IO has been posted otherwise user process
	will hang in MWAIT waiting for the outstanding IO to complete.  Catch
	is the system dynamic memory used in the user's IO request (IRP).

Inputs:

	None.

Outputs:

	None.

Side Effects:

	All user IO is posted with the TCP error "TCP is Exiting".
*/

void Exit_Handler (void)
    {
    EXTERNAL ROUTINE
void	USER$Purge_All_IO ,
void	RESET_PROCNAME ;

    ERROR$FAO("Exit handler: Exit requested, cleaning up...");

    USER$Purge_All_IO();
    if (LOG_STATE != 0)
	LOG_CLOSE();
    if (ACT_STATE != 0)
	ACT_CLOSE();
    OPR$FAO("Network offline - ACP exiting.");

// Re-enable resource wait mode (for debugging)

    $SETRWM(WATFLG=0);

    RESET_PROCNAME();
    }

//Sbttl "VMS Exception Handler"
/*

Function:

	Catch those nasty Exceptions which might might cause TCP to crash
	& forget about user IO requests thus leaving the user jobs stuck
	in MWAIT state waiting for their IO to complete.

Inputs:

	None.

Outputs:

	SS$_Resignal, indicate we want to bomb.

Side Effects:

	All user IO is posted with the TCP error "TCP is Exiting".
*/

Exception_Handler(SIG,MECH)
    {
    MAP
	struct BLOCK * SIG[,BYTE],
	struct BLOCK * MECH[,BYTE];
extern void	USER$Purge_All_IO ();

    ERROR$FAO("Exception handler: signal name !XL",SIG->CHF$L_SIG_NAME);
    USER$Purge_All_IO();
    $FLUSH(RAB = LOGRAB);
    $FLUSH(RAB = ACTRAB);
    return(SS$_Resignal);
    }
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
//TITLE "CONFIG - Configure the network ACP"
/*

Module:

	CONFIG  - Network ACP configuration

Facility:

	Configure the the ACP according the information extracted from the
	configuration file "INET$CONFIG", generally "config.txt".

Abstract:

	Handle the aspects of physical network device management.
	Here you will find the table(dev_config) which IP accesses to send
	datagrams.  Also device configuration and initialization routines are
	located here.  The init routine here should not be confused with the
	actual device init routines found in the driver modules.  Device init
	here implies the routine which will call all device init routines for
	those devices which have been properly configured during the device
	configuration phase of start-up. Other initializations handled by this
	module include memory manager block preallocation, gateway definition,
	name server definition, and the settings of various internal control
	variables such as "IP_FORWARDING' and 'LOG_STATE".

Author:

	Original version by Stan Smith, Tim Fallon Fall/Winter, 1982.
	This version by Vince Fuller, CMU-CSD, Spring/Summer, 1986
	Copyright (c) 1986,1987, Vince Fuller and Carnegie-Mellon University

Modification History:

*** Begin USBR change log ***

6,6	24-Dec-1991	Henry W. Miller 	USBR
	Make LOG_THRESHOLD and ACT_THRESHOLD configurable variables.

6.5d	28-Jan-1990	Henry W. Miller 	USBR
	Make WINDOW_DEFAULT and ACK_THRESHOLD configurable variables.

6.5c	13-Jan-1990	Henry W. Miller 	USBR
	Make ICMPTTL, IPTTL, TCPTTL, UDPTTL configurable variables.

*** Begin CMU change log ***

6.5b	12-Nov-1990	Bruce R. Miller		CMU NetDev
	Added the RPC keyword to configure RPC services.
	Added AUTH keyword to do UIC mapping (for NFS)

6.5a	28-Aug-1990     Henry W. Miller USBR
	Added DEFTTL configuration variable.

6.5	21-Sep-90	Bruce R. Miller		CMU NetDev
	Modifications from Mark Berryman, SAIC.COM
	Added code for proxy ARPs and restored clone device code.

6.5	02-Feb-1990	Bruce R. Miller		CMU NetDev
	Removed compile-time device support.
	Added run-time device support.

6.5	20-Oct-1989	Bruce R. Miller		CMU NetDev
	Added Activity Logging stuff.  Copied from normal logging stuff.

6.3	13-MAR-1989	Dale Moore	CMU-CS
	Added Keep Alive stuff.  (I could have sworn that I added
	this stuff before, but I musta lost it.)

6.2B 7-Oct-88, Edit by Robert Smart
	Fix test for number of interfaces (John Mann fix).

6.2A 21-Jun-88, Edit by Peter Dewildt
	Add X25 driver support
	Add SLIP driver support
	Add DECNET driver support

6.2  29-Oct-87, Edit by VAF
	Flush TMPJNL and PRMJNL privs - they no longer exist in VMS 4.6.

6.1  31-Jul-87, Edit by VAF
	Change network config file to include device type field, as distinct
	from device name field. Add TELNET_SERVICE variable.

6.0  23-Mar-87, Edit by VAF
	Add variable MAX_RECV_DATASIZE.
	Replace three sizes of packet buffers with two.

5.9  24-Feb-87, Edit by VAF
	Add support for mailbox name server (MBX_SERVER keyword). Also
	reorganized the parsing of WKS stuff.

5.8  19-Feb-87, Edit by VAF
	Flush support for GREEN and DOMAIN.

5.7  17-Feb-87, Edit by VAF
	Make improvements to the error reporting.

5.6  12-Feb-87, Edit by VAF
	Add support for domain servers.

5.5   9-Feb-87, Edit by VAF
	Add support for specifying process priviliges in WKS entries.

5.4   6-Feb-87, Edit by VAF
	Change references to DEUNA/DEQNA to be "DEC Ethernet". Add support for
	VAXstation-2000 "ES" device (same as XE/XQ).

5.3   5-Feb-87, Edit by VAF
	Add support for network access control.

5.2  12-Nov-86, Edit by VAF
	Add support for WKS records in the config file.

5.1  11-Nov-86, Edit by VAF
	Add routine to set various parameter variables in the config file.
	Currently supported are IP_FORWARDING and FQ_MAX.

5.0  12-Sep-86, Edit by VAF
	Add support for "cloned" devices.

4.9   8-Aug-86, Edit by VAF
	Change name server initialization code to call GREEN_CONFIG.

4.8   4-Aug-86, Edit by VAF
	Add code to initialize name server table.

4.7  23-Jun-86, Edit by VAF
	Add code to initialize IP forwarding state.

4.6  29-Apr-86, Edit by VAF
	Put device checking timers in here.

4.5  21-Apr-86, Edit by VAF
	Phase II of flushing XPORT - Use $FAO instead of XPORT string stuff.

4.4  18-Apr-86, Edit by VAF
	Add new I/O utilities (IOUTIL.BLI module)
	Add EN (3MB ethernet) device
	Report device status after init routine called

4.3  15-Apr-86, Edit by VAF
	Support for variable-length hardware addresses.
	Read physical address as one string of Hex digits.
	Make a byte-swapped copy of the IP address for ARP's use.

4.2   8-Mar-86, Edit by VAF
	Logging flag values are in hex now.

4.1   6-Mar-86, Edit by VAF
	Rewrite to flush all traces of XPORT.

4.0  21-Feb-86, Edit by VAF
	Give each network interface its own IP address and network mask.
	Reorganize initialization code for easier modification.
	Don't preset "dev_config table", flush "NL_xxx" routines.
	Add support for gateways

*** End CMU change log ***

1.0 [1-5-82] tim fallon

	Device configuration was buried inside of IP as we only had one
	network device at the time (hyperchannel).

2.0 [4-10-82] stan smith, tim fallon.

	Moved device configuration out of IP into this module.  Idea was to
	make IP device independent; such that IP accessed all network device
	driver routines (not to be confused with vms kernel level device drivers)
	in the same fashion (same calling conventions via dev_config table).
	Devices are configured (dev_config initialized) by macro invocations
	during devconfig.bli compilation.  Actual reason behind all this change
	was that ethernet were comming to tek and we needed to be able to support
	multiple network devices (hyperchannel and ethernet) from the same
	network acp.

2.1 [4-20-83] stan smith

	Developed routine "configure_network_devices", one no longer has to
	recompile devconfig.bli to include a new ethernet or hyperchannel
	device.  Dev_config table is now initialized from the file logical
	name "tcp$network_device_file".  Each non-comment line of the file
	describes a network device (vms name, used in $assign system service;
	physical device address and device dependent bits).

3.0 [4-28-83] stan smith

	Expanded device configuration to include memory mgmt config. Routine
	"configure_network_Devices" has been renamed to configure_acp.  Format
	of "devconfig.txt" now "config.txt" file has been expanded.  First
	field is now a keyword field, processing is dispatched via this
	keyword.

3.1 [8-2-85] Noelan Olson

	Continue to run if no network devides were configured.  Favor to Stan.

3.11 [11-18-85] Rick Watson U.Texas

	Added ARP transponder code for Deuna device
*/

#if 0
MODULE CONFIG(IDENT="6.6",ZIP,OPTIMIZE,
	      ADDRESSING_MODE(EXTERNAL=LONG_RELATIVE,
			      NONEXTERNAL=LONG_RELATIVE),
	      LIST(NOEXPAND,NOREQUIRE,ASSEMBLY,	OBJECT,BINARY))
#endif

#include	<starlet.h>	// VMS system definitions
     // not yet #include "CMUIP_SRC:[CENTRAL]NETXPORT";	// String descriptor stuff
#include "netvms.h"		// Special VMS definitions
#include "STRUCTURE";	// Data structures
#include	"TCPMACROS";	// ACP-wide macros
#include "SNMP";		// MIB definitions
#include "CMUIP_SRC:[CENTRAL]NETCOMMON"; // Common Defs
#include "CMUIP_SRC:[CENTRAL]NETCONFIG"; // Device configuration defs

extern
extern  void    OPR_FAO();
extern  void    SEND_2_OPERATOR();
extern     LIB$GET_VM : ADDRESSING_MODE(GENERAL)();
extern  void    SWAPBYTES();
extern     GET_IP_ADDR();
extern     GET_DEC_NUM();
extern     GET_HEX_BYTES();
extern     GET_HEX_NUM();
extern  void    ASCII_DEC_BYTES();
extern  void    ASCII_HEX_BYTES();

MACRO
    STR[] = CH$PTR(UPLIT(%ASCIZ %STRING(%REMAINING))) %,
    STRMOVE(THESTR,DPTR) =		// Strmove - STRMOVE(STRING,BP)
	CH$MOVE(%CHARCOUNT(THESTR),STR(THESTR),DPTR) %,
    STREQL(THESTR,LITSTR) =		// Compare to literal string
	CH$EQL(%CHARCOUNT(LITSTR),STR(LITSTR),%CHARCOUNT(LITSTR),THESTR) %,
    STREQLZ(THESTR,LITSTR) =		// Compare to literal string (ASCIZ)
	CH$EQL(%CHARCOUNT(LITSTR)+1,STR(LITSTR),%CHARCOUNT(LITSTR)+1,THESTR) %;

MACRO
    Init_DynDesc (D)
	{
	$BBLOCK [D, dsc$w_length]	= 0;
	$BBLOCK [D, dsc$b_dtype]	= DSC$K_DTYPE_T;
	$BBLOCK [D, dsc$b_class]	= DSC$K_CLASS_D;
	$BBLOCK [D, dsc$a_pointer]	= 0;
	}%;


// Module-wide definitions

LITERAL
    RECLEN = 512,
    STRLEN = 128,
    STRSIZ = CH$ALLOCATION(STRLEN);

// Define file reading blocks

static signed long
    LINPTR,
    LINLEN,
    CFBUF : VECTOR[RECLEN,BYTE],
    CFFAB : $FAB(FNM = "INET$CONFIG"),
    CFRAB : $RAB(FAB = CFFAB,
		 UBF = CFBUF,
		 USZ = %ALLOCATION(CFBUF));

 void    CONFIG_ERR();
 void    Init_Device();
 void    Init_MEMGR();
 void    Init_Gateway();
 void    Init_NameServer();
 void    Init_Logging();
 void    Init_Activity_Logging();
 void    Init_Forwarding();
 void    Init_Variable();
 void    Init_MBXResolver();
 void    Init_WKS();
 void    Init_RPC();
 void    Init_Auth();
 void    Init_Local_Host();
    GETFIELD();
    PARSE_NULLFIELD();
 void    SKIPTO();
 void    SKIPWHITE();



// Here we set up the IPACP_Interface.  It provides entry points,
// literals and global pointers to any modules which may be loaded
// at run-time.  See NETCONFIG.REQ for a complete explaination of
// this structure.

    struct IPACP_Info_Structure * IPACP_Int;

// N.B. : make sure everything that needs to be defined, like the
// MAX_Physical_BufSize, *is* defined by the time this routine is called...

CNF$Define_IPACP_Interface (void)
    {
    externAL
	// pointer to IPACP AST_in_progress flag (from MAIN.BLI)
	AST_in_progress,
	// IPACP nap control (from MAIN.BLI)
	Sleeping,

	// IPACP Maximum Physical Buffer Size
	MAX_PHYSICAL_BUFSIZE,

	LOG_STATE;

	// IAPCP receive callback (from IP.BLI)
extern	IP$Receive();
	// IPACP self-address recognition (from IP.BLI)
extern 	IP$ISME();

	// Interrupt blocking routines (from WHERE???)
extern 	void MAIN$NOINT();
extern 	void MAIN$OKINT();

	// Error reporting routines (from CONFIG.BLI)
extern 	void CNF$Device_Error();

	// Memory allocation routines (from MEMGR.BLI)
extern 	void MM$Seg_Get();
extern 	void MM$Seg_Free();
extern 	void MM$QBlk_Free();

	// Formatted event logging routines (from IOUTIL.BLI)
extern 	void LOG_FAO();
extern 	void QL_FAO();
extern 	void OPR_FAO();
extern 	void ERROR_FAO();
extern 	void FATAL_FAO();

    // IAPCP receive callback.
    IPACP_Int [ ACPI$IP_Receive ]	= IP$Receive;

    // pointer to IPACP sleeping flag
    IPACP_Int [ ACPI$Sleeping ] 	= Sleeping;

    // pointer to IPACP AST_in_progress flag
    IPACP_Int [ ACPI$AST_in_progress ] 	= AST_in_progress;

    // Interrupt blocking routines
    IPACP_Int [ ACPI$NOINT ]		= MAIN$NOINT;
    IPACP_Int [ ACPI$OKINT ]		= MAIN$OKINT;

    // Error reporting routines
    IPACP_Int [ ACPI$Device_Error ]	= CNF$Device_Error;

    // IPACP self-address recognition
    IPACP_Int [ ACPI$IP_ISME ]		= IP$ISME;

    // Memory allocation routines
    IPACP_Int [ ACPI$Seg_Get ]		= MM$Seg_Get;
    IPACP_Int [ ACPI$Seg_Free ]		= MM$Seg_Free;
    IPACP_Int [ ACPI$QBlk_Free ]	= MM$QBlk_Free;

    // Provide event logging entry points
    IPACP_Int [ ACPI$LOG_STATE ]	= LOG_STATE;	// pointer
    IPACP_Int [ ACPI$LOG_FAO ]		= LOG_FAO;
    IPACP_Int [ ACPI$QL_FAO ]		= QL_FAO;
    IPACP_Int [ ACPI$OPR_FAO ]		= OPR_FAO;
    IPACP_Int [ ACPI$ERROR_FAO ]	= ERROR_FAO;
    IPACP_Int [ ACPI$FATAL_FAO ]	= FATAL_FAO;

    // IPACP max physical buffer size
    IPACP_Int [ ACPI$MPBS ] 		= MAX_PHYSICAL_BUFSIZE;

    IPACP_Int
    }



/* Here we initialize all of the variable that can be set by the config
   file.
*/

void Init_Vars (void)
    {
    externAL
	struct IP_group_MIB_struct * IP_group_MIB;

    IP_group_MIB->IPMIB$ipForwarding = 2;	// Just a host, no forwarding
    }


//Sbttl "Device Configuration table definitions"
/*

Here we configure the known network devices.  Each entry in the dev_config
table represents a known network device (eg, hyperchannel, ethernet etc).
If your system doesn't have the specified device, then leave the table entry
alone or replace the nonexistant entry with one of your own choosing.  All
the device_init macro does is to place the device initialization and send
routine addresses in the appropriate device-type entry.
See the DC_Fields portion of STRUCTURE.DEF for details.

*/

void No_Check(ndx)

// Dummy routine for non-checked devices.

    {
    return;
    }

static signed long
    DEV_Attn;

// gotta declare this PSect on the quad boundary so-as we can use
// self relative queue instructions on it...

PSECT
    GLOBAL = quads(ALIGN(3));

signed long
    DEV_Config_Tab: Device_Configuration_Table PSECT(quads) ALIGN(3),
    DEV_Count;



CNF$DEVICE_STAT ( Inx, RB_A )
!
// Dump out a Device configuration entry.
!
    {
    BIND
	RB = RB_A : D$Dev_Dump_Return_Blk;

    IF (Inx LSS 0) || (Inx > DC_Max_Num_Net_devices-1) OR
	(dev_config_tab[Inx,dc_valid_device] == 0) THEN
	return -1;

    // fill in the simple stuff...
    RB->DU$Dev_Address = dev_config_tab[Inx,dc_ip_address];
    RB->DU$Dev_netmask = dev_config_tab[Inx,dc_ip_netmask];
    RB->DU$Dev_network = dev_config_tab[Inx,dc_ip_network];
    RB->DU$Dev_Pck_Xmit = 0;
    RB->DU$Dev_Pck_Recv = 0;

    // Start a new block so we can use BIND to simplify the code.
    {
    BIND
	DevNam  = dev_config_tab[Inx,dc_devname] : $BBLOCK->DSC$K_Z_BLN,
	DevSpec = dev_config_tab[Inx,dc_devspec] : $BBLOCK->DSC$K_Z_BLN;
    signed long
	StrLen;

    // Copy device name
    StrLen = DevNam_Max_Size;
    if (Strlen > DevNam->dsc$w_length)
	StrLen = DevNam->dsc$w_length;
    RB->DU$DevNam_Len = Strlen;
    CH$MOVE ( Strlen , DevNam->dsc$a_pointer , RB->DU$DevNam_Str );

    // Copy device-specific field
    StrLen = DevSpec_Max_Size;
    if (Strlen > DevSpec->dsc$w_length)
	StrLen = DevSpec->dsc$w_length;
    RB->DU$DevSpec_Len = Strlen;
    CH$MOVE ( Strlen , DevSpec->dsc$a_pointer , RB->DU$DevSpec_Str );
    }

    D$Dev_dump_blksize
    }


CNF$DEVICE_LIST ( RB )
//
// Dump out the list of valid devices.
//
	struct D$Device_List_Return_Blk * RB;
    {
    signed long
	RBIX;

    RBIX = 1;
    for (I=0;I<=DC_Max_Num_Net_devices-1;I++)
	if (dev_config_tab[I,dc_valid_device] != 0)
	    {
	    RB[RBIX] = I;
	    RBIX = RBIX + 1;
	    };
    // First element of list is the count.
    RB[0] = RBIX - 1;

    // return total size in bytes.
    RBIX * 4
    }


//SBTTL "Configure Network ACP"

/*
Function:

	Read the IPACP configuration file, "INET$CONFIG", containing all info
	about the ACP initialization parameters, including: devices, memory
	management parameters, logging state, variables, well-known services,
	etc. Each line of the file contains a keyword followed by the values
	associated with the keyword. Lines beginning in ";" are ignored as
	comment lines.

Side Effects:

	Initializes the DEV_CONFIG table, the globals for memory management,
	and calls all of the configuration routines in other modules.
*/

void CNF$Configure_ACP (void)
    {
    LOCAL		     
	RC,
	cfield : VECTOR->STRSIZ,
	cflen,
	cptr;

// Initialize all the variables before they are set by the config script

    INIT_VARS();

// OPEN the file "config.TXT" & read/decode network device data into blockvector
// dev_config and memory management info.

    IF NOT ((RC = $OPEN(FAB = CFFAB)) AND
	    (RC = $CONNECT(RAB = CFRAB))) THEN
	{
	send_2_operator(%ascid "Unable to access "INET$CONFIG:"");
	$exit(code=.RC);
	};

// Extract information from each non-comment line of the file

    DEV_Count = 0;		// No devices

    while ($GET(RAB = CFRAB))
	{
	signed long
	    chr;
	linptr = CH$PTR(CFRAB->RAB$L_UBF);
	linlen = CFRAB->RAB$W_RSZ;

// IF 1st char of line is a "!" then is it a comment & we ignore it.

	chr = CH$RCHAR(CH$PTR(CFRAB->RAB$L_UBF));
	if ((chr != %C"!') && (chr != %C';"))
	    {
	    if (linlen > (RECLEN-1))
		linlen = RECLEN-1;
	    cptr = CH$PLUS(linptr,linlen);
	    CH$WCHAR_A(0,cptr);		// Make ASCIZ

// Read the first field (category keyword) and dispatch to the appro processing
// routine.

	    cflen = GETFIELD(cfield);
	    cptr = CH$PTR(cfield);
	    SELECTONE TRUE OF
		SET
		[STREQLZ(cptr,"DEVICE_INIT")]:
		    Init_Device();
		[STREQLZ(cptr,"MEMGR_INIT")]:
		    Init_Memgr();
		[STREQLZ(cptr,"GATEWAY")]:
		    Init_Gateway();
		[STREQLZ(cptr,"NAME_SERVER")]:
		    Init_NameServer();
		[STREQLZ(cptr,"LOGGING")]:
		    Init_Logging();
		[STREQLZ(cptr,"ACTIVITY")]:
		    Init_Activity_Logging();
		[STREQLZ(cptr,"IP_FORWARDING")]:
		    Init_Forwarding();
		[STREQLZ(cptr,"VARIABLE")]:
		    Init_Variable();
		[STREQLZ(cptr,"MBX_RESOLVER")]:
		    Init_MbxResolver();
		[STREQLZ(cptr,"WKS")]:
		    Init_WKS();
		[STREQLZ(cptr,"RPC")]:
		    Init_RPC();
		[STREQLZ(cptr,"AUTH")]:
		    Init_Auth();
		[STREQLZ(cptr,"LOCAL_HOST")]:
		    Init_Local_Host();
		[OTHERWISE]:
		    Config_Err(%ASCID"Unknown keyword");
		TES;
	    };
	};

    $DISCONNECT(RAB = CFRAB);
    $CLOSE(FAB = CFFAB);

// make sure we did some device configuration here as networks without devices
// are not really very interesting.

    if (dev_count <= 0)
	ERROR$FAO("No network devices detected in INET$CONFIG");
    }

CONFIG_ERR(EMSG) : NOVALUE (void)
!
// Handle error in configuration file. Give the error message and exit.
!
    {
    FATAL$FAO("CONFIG - !AS in line:!/!_"!AD"",
	      EMSG,CFRAB->RAB$W_RSZ,CFRAB->RAB$L_UBF);
    }

void Init_Device (void)

// Handle a DEVICE_INIT entry in the INET$CONFIG file.
// Parses the device description and adds to DEV_CONFIG table.

    {
extern 	LIB$CALLG		: ADDRESSING_MODE (GENERAL)();
extern 	LIB$FIND_IMAGE_SYMBOL	: ADDRESSING_MODE (GENERAL)();
extern 	STR$APPEND		: BLISS ADDRESSING_MODE (GENERAL)();
extern 	STR$CASE_BLIND_COMPARE	: BLISS ADDRESSING_MODE (GENERAL)();
extern 	STR$COPY_DX		: BLISS ADDRESSING_MODE (GENERAL)();
struct Device_Info_Structure * Devinfo;
struct Device_Configuration_Entry * dev_config;
    signed long
//	tmp,
	rc,
	ipaddr,
	ipmask,
	argv[1],
	Image_Init,
	devtype[STRSIZ],
	devtlen,
	devstr[STRSIZ],
	devslen,
	devspec[STRSIZ],
	dev_desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= Devstr),
	devtyp_desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= Devtype),
	dev_spec_desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= Devtype),
	devidx;		// device index into devconfig.

// skip over delimiter.

    SKIPTO(%C":");

// Dev_config table overflow?
// Table index = 0 to DC_Max_Num_Net_devices-1.

    devidx = Dev_count;
    if (Dev_Count > DC_Max_Num_Net_devices-1 )
	FATAL$FAO(%STRING("Too many network devices in INET$CONFIG, max = ",
			  %NUMBER(DC_Max_Num_Net_Devices)));
    Dev_Count = Dev_Count+1;
    dev_config = dev_config_tab[devidx,dc_begin];

// Clear all flags

    dev_config->dc_flags = 0;
    dev_config->dc_clone_dev = -1;

// Get device driver type

    devtyp_desc [dsc$w_length] = GETFIELD(devtyp_desc [dsc$a_pointer]);
//    devtlen = GETFIELD(devtype);
    SKIPTO(%C":");
   
//    cptr = CH$PTR(devtype);

//!!HACK!!// For now, we just include the ethernet code inorderto test
// it more easily (all symbols included, plus code is now in main image PSECT)


    if ((STR$CASE_BLIND_COMPARE(devtyp_desc,%ASCID"ETHER") == 0))
	{
extern	    DRV$TRANSPORT_INIT();

	Image_Init = DRV$TRANSPORT_INIT;
	}
// Get name of shared-image IP-transport
    else IF (rc = LIB$FIND_IMAGE_SYMBOL(
		devtyp_desc,
		%ASCID"DRV$TRANSPORT_INIT",
		Image_Init,
		%ASCID"CMUIP_ROOT:[SYSLIB].EXE"
		) != SS$_NORMAL) THEN
	{
	Image_Init = 0;
	Signal(rc);
	Config_Err(%ASCID"Trouble accessing device support image.")
	};

    if ((Image_Init != 0))
	{	// Set up the vector
	DevInfo = LIB$CALLG(argv,Image_Init);

	dev_config->dc_rtn_init = DevInfo->DI$Init;
	dev_config->dc_rtn_xmit = DevInfo->DI$Xmit;
	dev_config->dc_rtn_dump = DevInfo->DI$Dump;
	dev_config->dc_rtn_check = DevInfo->DI$Check
	};

// Get vms device name, used
// by the device init routine to $assign the device.
    dev_desc [dsc$w_length] = GETFIELD(dev_desc [dsc$a_pointer]);
//    $INIT_DYNDESC( dev_config [dc_devname] );
    INIT_DYNDESC( dev_config [dc_devname] );
    STR$COPY_DX (Dev_Config [dc_devname], dev_desc);
    STR$APPEND (Dev_config [dc_devname], %ASCID ":");
// skip over field terminator.
    SKIPTO(%C":");

// Get vms device specific information, used by the device go receive
// and initiate connections.
    dev_desc [dsc$w_length] = GETFIELD(dev_desc [dsc$a_pointer]);
//    $INIT_DYNDESC( dev_config [dc_devspec] );
    INIT_DYNDESC( dev_config [dc_devspec] );
    STR$COPY_DX (Dev_Config [dc_devspec], dev_desc);
// skip over field terminator.
    SKIPTO(%C":");

// Set hardware address/device dependant words
//    IF GET_HEX_BYTES(dev_config->dc_phy_size,LINPTR,
//		     CH$PTR(dev_config->dc_phy_addr)) LSS 0 THEN
//	Config_Err(%ASCID"Bad device address");
//    SKIPTO(%C":");

// Get device IP address

    if (GET_IP_ADDR(LINPTR,ipaddr) LSS 0)
	Config_Err(%ASCID"Bad device IP address");
    dev_config->dc_ip_address = ipaddr;
    SKIPTO(%C":");

// Get device IP network mask

    if (GET_IP_ADDR(LINPTR,ipmask) LSS 0)
	Config_Err(%ASCID"Bad device IP mask");
    dev_config->dc_ip_netmask = ipmask;

// Set device IP network number

    dev_config->dc_ip_network = ipmask && ipaddr;

// mark the device as valid so initialization routine will execute the device
// init rtn.

    dev_config->dc_valid_device = true;

// Initialize all of the other fields for this device

    Dev_Config->dc_send_Qhead = Dev_Config->dc_send_Qhead;
    Dev_Config->dc_send_Qtail = Dev_Config->dc_send_Qhead;
    Dev_Config->dc_online = false;

// See if this device name is a duplicate.

    if (devidx > 0)
	for (I=0;I<=(devidx-1);I++)
	    {
	    // If match, then mark this device as a clone of the it.
	    IF STR$CASE_BLIND_COMPARE(Dev_Config_Tab[I,dc_devname],
			Dev_Config_Tab[devidx,dc_devname]) == 0 THEN
		{
		dev_config->dc_is_clone = TRUE;
		dev_config->dc_clone_dev = I;
		break;
		};
	    };
    }

void Init_Gateway (void)
//!!HACK!!// Make gateway struct dynamic.
// Handle a GATEWAY entry in the INET$CONFIG file.
// Parses gateway description and adds to GWY_TABLE
// Gateway entries are of the form:
//   GATEWAY:<gwy_name>:<gwy_address>:<gwy-network>:<gwy-netmask>
// Where (these are also the names of the GWY_TABLE fields):
//   gwy_name	Name of the gateway (for debugging purposes)
//   gwy_address	IP address of gateway *must be locally-connected*
//   gwy_network	Network number served by gateway (0 means any/default)
//   gwy_netmask	AND mask for comparing network number
//   gwy_status	Gateway status. Initialized to "up" (nonzero) by this code.

    {
extern	void IP$Gwy_Config();
    signed long
	GWYname [STRSIZ],
	GWY_Name_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= GWYname),
	GWYaddr,
	GWYnet,
	GWYnetmask,
	tmp;

// Skip delimiter

    SKIPTO(%C":");

// First, get the gateway name.

    GWY_Name_Desc [dsc$w_length] = GETFIELD(GWYname);

// Next, the gateway address

    SKIPTO(%C":");
    if (GET_IP_ADDR(LINPTR,GWYaddr) LSS 0)
	Config_err(%ASCID"Bad gateway address");

// Next, the network number behind the gateway

    SKIPTO(%C":");
    if (GET_IP_ADDR(LINPTR,GWYnet) LSS 0)
	Config_err(%ASCID"Bad gateway network number");

// Next, the network mask for that network

    SKIPTO(%C":");
    if (GET_IP_ADDR(LINPTR,GWYnetmask) LSS 0)
	Config_err(%ASCID"Bad gateway mask");

// Tell IP about this gateway

    IP$Gwy_Config(GWY_Name_Desc,GWYaddr,GWYnet,GWYnetmask);
    }

void Init_NameServer (void)

// Handle a Name_Server entry in the INET$CONFIG file.
// Parses Name Server description and adds to NS_TABLE
// Name Server entries are of the form:
//   NAME_SERVER:<NS_name>:<NS_address>
// Where (these are also the names of the NS_TABLE fields):
//   NS_name	Name of the gateway (for debugging purposes)
//   NS_address	IP address of gateway *must be locally-connected*
//   NS_status	Gateway status. Initialized to "up" (nonzero) by this code.

    {
    signed long
	NSname [STRSIZ],
	NSnlen,
	NSaddr,
	tmp;

// Skip delimiter

    SKIPTO(%C":");

// First, get the name server name

    NSnlen = GETFIELD(NSname);

// Next, the name server address

    SKIPTO(%C":");
    if (GET_IP_ADDR(LINPTR,NSaddr) LSS 0)
	Config_err(%ASCID"Bad name server address");

// Add this entry to the name server database

    OPR$FAO("%IPACP: Obsolete keyword NAME_SERVER found in INET$CONFIG");
    }

void Init_MEMGR (void)
//
// Handle MEMGR-INIT entry in the INET$CONFIG file.
// Specifies memory-manager initialization parameters.
//
    {
    externAL
	QBLK_COUNT_BASE : UNSIGNED BYTE,
	UARG_COUNT_BASE : UNSIGNED BYTE,
	MIN_SEG_COUNT_BASE : UNSIGNED BYTE,
	MAX_SEG_COUNT_BASE : UNSIGNED BYTE;

// Get # of Queue blocks to preallocate

    SKIPTO(%C":");
    if (GET_DEC_NUM(LINPTR,QBLK_count_base) LSS 0)
	Config_Err(%ASCID"Bad integer value");

// Get # of UARG blocks to preallocate

    SKIPTO(%C":");
    if (GET_DEC_NUM(LINPTR,UARG_count_base) LSS 0)
	Config_Err(%ASCID"Bad integer value");

// Get # of minimum-size packet buffers to preallocate

    SKIPTO(%C":");
    if (GET_DEC_NUM(LINPTR,MIN_seg_count_base) LSS 0)
	Config_Err(%ASCID"Bad integer value");

// get # of maximum-size packet buffers to preallocate

    SKIPTO(%C":");
    if (GET_DEC_NUM(LINPTR,MAX_seg_count_base) LSS 0)
	Config_Err(%ASCID"Bad integer value");
    }



void INIT_LOGGING (void)

// Set initial logging state.

    {
extern	void LOG_CHANGE();
    signed long
	logstate;

// Skip terminator

    SKIPTO(%C":");

// Get log state value

    if (GET_HEX_NUM(LINPTR,logstate) LSS 0)
	Config_Err(%ASCID"Bad hex value for log state");

// Set log state

    LOG_CHANGE(logstate);
    }



void INIT_ACTIVITY_LOGGING (void)

// Set initial activity logging state.

    {
extern	void ACT_CHANGE();
    signed long
	logstate;

// Skip terminator

    SKIPTO(%C":");

// Get log state value

    if (GET_HEX_NUM(LINPTR,logstate) LSS 0)
	Config_Err(%ASCID"Bad hex value for activity log state");

// Set log state

    ACT_CHANGE(logstate);
    }



//SBTTL "Initialize IP forwarding state"

void INIT_FORWARDING (void)
    {
    signed long
	ipstate;
    externAL
	struct IP_group_MIB_struct * IP_group_MIB;

// Skip terminator

    SKIPTO(%C":");

// Get forwarding state value

    if (GET_HEX_NUM(LINPTR,ipstate) LSS 0)
	Config_Err(%ASCID"Bad hex value for forwarding state");

// Set state for IP module

    IP_group_MIB->IPMIB$ipForwarding = ipstate;
    }



//SBTTL "Initialize various configuration variables"

void INIT_VARIABLE (void)
    {
    externAL
	FQ_MAX,
	SYN_WAIT_COUNT,
	ACCESS_FLAGS,
	MAX_RECV_DATASIZE,
	DEFAULT_MSS,
	TELNET_SERVICE,
	RPC_SERVICE,
	SNMP_SERVICE,
	KEEP_ALIVE,
	RETRY_COUNT,
	MAX_LOCAL_PORTS,
	MAX_CONN,
	MAX_GATEWAYS,
	WINDOW_DEFAULT,
	ACK_THRESHOLD,
	ICMPTTL,
	IPTTL,
	TCPTTL,
	UDPTTL,
	ACT_THRESHOLD,
	LOG_THRESHOLD,
	struct IP_group_MIB_struct * IP_group_MIB;
    signed long
	varname [STRSIZ],
	varlen,
	varptr,
	varval;

// Skip terminator

    SKIPTO(%C":");

// Get the variable name

    varlen = GETFIELD(varname);
    varptr = CH$PTR(varname);

// Skip the terminator

    SKIPTO(%C":");

// Get the variable value

    if (GET_DEC_NUM(LINPTR,varval) LSS 0)
	Config_Err(%ASCID"Bad variable value");

// Check the variable name & set it.
//!!HACK!!// Document these!
    SELECTONE TRUE OF
	SET
	[STREQLZ(varptr,"IP_FORWARDING")]:
	    IP_group_MIB->IPMIB$ipForwarding = varval;
	[STREQLZ(varptr,"FQ_MAX")]:
	    FQ_MAX = varval;
	[STREQLZ(varptr,"SYN_WAIT_COUNT")]:
	    SYN_WAIT_COUNT = varval;
	[STREQLZ(varptr,"ACCESS_FLAGS")]:
	    ACCESS_FLAGS = varval;
	[STREQLZ(varptr,"MAX_TCP_DATASIZE")]:
	    MAX_RECV_DATASIZE = varval;
	[STREQLZ(varptr,"DEFAULT_MSS")]:
	    DEFAULT_MSS = varval;
	[STREQLZ(varptr,"TELNET_SERVICE")]:
	    TELNET_SERVICE = varval;
	[STREQLZ(varptr,"RPC_SERVICE")]:
	    RPC_SERVICE = varval;
	[STREQLZ(varptr,"SNMP_SERVICE")]:
	    SNMP_SERVICE = varval;
	[STREQLZ(varptr,"KEEP_ALIVE")]:
	    KEEP_ALIVE = varval;
	[STREQLZ(varptr,"RETRY_COUNT")]:
	    RETRY_COUNT = varval;
	[STREQLZ(varptr,"MAX_LOCAL_PORTS")]:
	    MAX_LOCAL_PORTS = varval;
	[STREQLZ(varptr,"MAX_CONN")]:
	    MAX_CONN = varval;
	[STREQLZ(varptr,"MAX_GATEWAYS")]:
	    Max_Gateways = varval;
	[STREQLZ(varptr,"ICMPTTL")]:
	    ICMPTTL = varval;
	[STREQLZ(varptr,"IPTTL")]:
	    IPTTL = varval;
	[STREQLZ(varptr,"TCPTTL")]:
	    TCPTTL = varval;
	[STREQLZ(varptr,"UDPTTL")]:
	    UDPTTL = varval;
	[STREQLZ(varptr,"WINDOW_DEFAULT")]:
	    WINDOW_DEFAULT = varval;
	[STREQLZ(varptr,"ACK_THRESHOLD")]:
	    ACK_THRESHOLD = varval;
	[STREQLZ(varptr,"ACT_THRESHOLD")]:
	    ACT_THRESHOLD = varval;
	[STREQLZ(varptr,"LOG_THRESHOLD")]:
	    LOG_THRESHOLD = varval;
	[OTHERWISE]:
	    Config_Err(%ASCID"Unknown variable name");
	TES;
    }



//SBTTL "Process definitions"
/*
    Define some keyword tables for parsing options to $CREPRC
 )%

// Macro for defining two-word privilege bit

MACRO
    PRVIX(BYT,BST,BLN,EXT) = UPLIT(BYT/4,1^BST) %;
LITERAL
    PVSIZE = 2,
    PVIX = 0,
    PVAL = 1;

BIND

// Define the STATUS keywords

    STATNAMES = PLIT(
	%ASCID"SSRWAIT",PRC$M_SSRWAIT,
	%ASCID"SSFEXCU",PRC$M_SSFEXCU,
	%ASCID"PSWAPM",PRC$M_PSWAPM,
	%ASCID"NOACNT",PRC$M_NOACNT,
	%ASCID"BATCH",PRC$M_BATCH,
	%ASCID"HIBER",PRC$M_HIBER,
	%ASCID"IMGDMP",PRC$M_IMGDMP,
	%ASCID"NOUAF",PRC$M_NOUAF,
	%ASCID"NETWRK",PRC$M_NETWRK,
	%ASCID"DISAWS",PRC$M_DISAWS,
	%ASCID"DETACH",PRC$M_DETACH,
	%ASCID"INTER",PRC$M_INTER,
	%ASCID"NOPASSWORD",PRC$M_NOPASSWORD) : VECTOR,

// Define the PRIVILEGE keywords

    PRIVNAMES = PLIT(
	%ASCID"*",-1,
	%ASCID"ALLSPOOL",PRVIX(PRV$V_ALLSPOOL),
	%ASCID"BUGCHK",PRVIX(PRV$V_BUGCHK),
	%ASCID"BYPASS",PRVIX(PRV$V_BYPASS),
	%ASCID"CMEXEC",PRVIX(PRV$V_CMEXEC),
	%ASCID"CMKRNL",PRVIX(PRV$V_CMKRNL),
	%ASCID"DETACH",PRVIX(PRV$V_DETACH),
	%ASCID"DIAGNOSE",PRVIX(PRV$V_DIAGNOSE),
	%ASCID"DOWNGRADE",PRVIX(PRV$V_DOWNGRADE),
	%ASCID"EXQUOTA",PRVIX(PRV$V_EXQUOTA),
	%ASCID"GROUP",PRVIX(PRV$V_GROUP),
	%ASCID"GRPNAM",PRVIX(PRV$V_GRPNAM),
	%ASCID"GRPPRV",PRVIX(PRV$V_GRPPRV),
	%ASCID"LOG_IO",PRVIX(PRV$V_LOG_IO),
	%ASCID"MOUNT",PRVIX(PRV$V_MOUNT),
	%ASCID"NETMBX",PRVIX(PRV$V_NETMBX),
	%ASCID"NOACNT",PRVIX(PRV$V_NOACNT),
	%ASCID"OPER",PRVIX(PRV$V_OPER),
	%ASCID"PFNMAP",PRVIX(PRV$V_PFNMAP),
	%ASCID"PHY_IO",PRVIX(PRV$V_PHY_IO),
	%ASCID"PRMCEB",PRVIX(PRV$V_PRMCEB),
	%ASCID"PRMGBL",PRVIX(PRV$V_PRMGBL),
	%ASCID"PRMMBX",PRVIX(PRV$V_PRMMBX),
	%ASCID"PSWAPM",PRVIX(PRV$V_PSWAPM),
	%ASCID"READALL",PRVIX(PRV$V_READALL),
	%ASCID"SECURITY",PRVIX(PRV$V_SECURITY),
	%ASCID"SETPRI",PRVIX(PRV$V_SETPRI),
	%ASCID"SETPRV",PRVIX(PRV$V_SETPRV),
	%ASCID"SHARE",PRVIX(PRV$V_SHARE),
	%ASCID"SHMEM",PRVIX(PRV$V_SHMEM),
	%ASCID"SYSGBL",PRVIX(PRV$V_SYSGBL),
	%ASCID"SYSLCK",PRVIX(PRV$V_SYSLCK),
	%ASCID"SYSNAM",PRVIX(PRV$V_SYSNAM),
	%ASCID"SYSPRV",PRVIX(PRV$V_SYSPRV),
	%ASCID"TMPMBX",PRVIX(PRV$V_TMPMBX),
	%ASCID"UPGRADE",PRVIX(PRV$V_UPGRADE),
	%ASCID"VOLPRO",PRVIX(PRV$V_VOLPRO),
	%ASCID"WORLD",PRVIX(PRV$V_WORLD)) : VECTOR;


// Define process quotas

$FIELD QUOTA_FIELDS (void)
    SET
    QTA$TYPE	= [$BYTE],	// Quota type
    QTA$VALUE	= [$ULONG]	// Quota value
    TES;
LITERAL
    QUOTA_SIZE = $FIELD_SET_SIZE,
    QUOTA_BLEN = 5;
MACRO
    QUOTA = BLOCK->QUOTA_SIZE FIELD(QUOTA_FIELDS) %;
LITERAL
    MAXQUOTA = 15;		// Max number of quotas to specify

// Define a keyword table for parsing the process quotas.

BIND
    QUOTANAMES = PLIT(
	%ASCID"ASTLM",	PQL$_ASTLM,
	%ASCID"BIOLM",	PQL$_BIOLM,
	%ASCID"BYTLM",	PQL$_BYTLM,
	%ASCID"CPULM",	PQL$_CPULM,
	%ASCID"DIOLM",	PQL$_DIOLM,
	%ASCID"ENQLM",	PQL$_ENQLM,
	%ASCID"FILLM",	PQL$_FILLM,
	%ASCID"JTQUOTA",PQL$_JTQUOTA,
	%ASCID"PGFLQUOTA",PQL$_PGFLQUOTA,
	%ASCID"PRCLM",	PQL$_PRCLM,
	%ASCID"TQELM",	PQL$_TQELM,
	%ASCID"WSDEFAULT",PQL$_WSDEFAULT,
	%ASCID"WSEXTENT",PQL$_WSEXTENT,
	%ASCID"WSQUOTA",PQL$_WSQUOTA);

KEY_VALUE(KEYTAB,KEYLEN,KEYSTR)
//
// Get keyword value from keyword string.
//
	long * KEYTAB;
    {
    signed long
	struct $BBLOCK * CURSTR->DSC$K_S_BLN;

    INCR I FROM 0 TO (KEYTAB[-1]-1) BY 2 DO
	{
	CURSTR = KEYTAB[I];
	IF CH$EQL(KEYLEN,KEYSTR,
		  CURSTR->dsc$w_length,CURSTR->dsc$a_pointer) THEN
	    return KEYTAB[I+1];
	};
    Config_Err(%ASCID"Bad keyword field");
    return -1;
    }

PARSE_PRCPRIVS(PRVBLK)
//
// Parse a comma-separated list of privilege keywords.
// Returns the number of keywords seen.
//
	long PRVBLK[2];
    {
    signed long
	PRIVCNT,
	struct [PRIVPTR->PVSIZE],
	PRIVBUF[STRSIZ],
	PLEN,
	CHR;

// Initialize to "no privileges"

    PRVBLK[0] = 0;
    PRVBLK[1] = 0;
    PRIVCNT = 0;

// Loop, parsing keywords.

    while (TRUE)
	{
	PLEN = GETFIELD(PRIVBUF);
	if (PLEN == 0)
	    break;
	PRIVCNT = PRIVCNT + 1;
	PRIVPTR = KEY_VALUE(PRIVNAMES,PLEN,CH$PTR(PRIVBUF));
	if (PRIVPTR == -1)
	    {
	    PRVBLK[0] = -1;
	    PRVBLK[1] = -1;
	    }
	else
	    PRVBLK[PRIVPTR->PVIX] = PRVBLK[PRIVPTR->PVIX] || PRIVPTR->PVAL;
	SKIPWHITE();

// Check for end of field separator

	CHR = CH$RCHAR_A(LINPTR);
	if (CHR == %C":")
	    break;
	};

// Return count of fields found

    return PRIVCNT;
    }

PARSE_PRCQUOTAS(QLIST,QMAX)
//
// Parse a comma-separated list of quota values.
// Returns the count of quotas seen.
//
    {
	struct QUOTA * QPTR;
    signed long
	QUOTBUF [STRSIZ],
	QUOTLEN,
	QUOTCNT,
	QUOTYPE,
	QUOTVAL,
	CHR;

// Initialize to none

    QUOTCNT = 0;
    CH$FILL(0,QMAX*QUOTA_BLEN,QLIST);
    QPTR = QLIST;

// Loop, parsing keywords.

    while (TRUE)
	{
	QUOTLEN = GETFIELD(QUOTBUF);
	if (QUOTLEN == 0)
	    break;

// Parse the keyword

	QUOTYPE = KEY_VALUE(QUOTANAMES,QUOTLEN,CH$PTR(QUOTBUF));
	SKIPWHITE();

// Parse the separator - must be an equal sign

	CHR = CH$RCHAR_A(LINPTR);
	if (CHR != %C"=")
	    CONFIG_ERR(%ASCID"Missing "=" in quota description");
	SKIPWHITE();

// Parse the quota value

	if (GET_DEC_NUM(LINPTR,QUOTVAL) LSS 0)
	    CONFIG_ERR(%ASCID"Missing or bad quota value");
	SKIPWHITE();

// Set the value in the quotalist

	QPTR->QTA$TYPE = QUOTYPE;
	QPTR->QTA$VALUE = QUOTVAL;

// Increment to next entry, checking for overflow

	QUOTCNT = QUOTCNT + 1;
	if (QUOTCNT GEQ (QMAX-1))
	    CONFIG_ERR(%ASCID"Exceeded max number of quota keywords");
	QPTR = QPTR + QUOTA_BLEN;
	SKIPWHITE();

// Check for terminator

	CHR = CH$RCHAR_A(LINPTR);
	if (CHR == %C":")
	    break;
	};

// Put the terminator entry in the list

    QPTR->QTA$TYPE = PQL$_LIST};
    QPTR->QTA$VALUE = 0;

// Return the number of entries in the list.

    return QUOTCNT+1;
    }

PARSE_PRCSTATUS (void)
//
// Parse a comma-separated list of process status flags.
// Returns the values OR'ed together.
//
    {
    signed long
	STATVAL,
	STATBUF[STRSIZ],
	STATLEN,
	CHR;

// Initialize value to all off

    STATVAL = 0;

// Loop, parsing the flags

    while (TRUE)
	{
	STATLEN = GETFIELD(STATBUF);
	if (STATLEN == 0)
	    break;
	STATVAL = STATVAL || KEY_VALUE(STATNAMES,STATLEN,CH$PTR(STATBUF));
	SKIPWHITE();
	CHR = CH$RCHAR_A(LINPTR);
	if (CHR == %C":")
	    break;
	};

// Finall, return the complete value

    return STATVAL;
    }

//SBTTL "Init_WKS - Add a WKS entry"
		     
void Init_WKS (void)

// Handle a WKS entry in the INET$CONFIG file.
// Parses WKS description and calles Seg$WKS_Config to add the info.
// WKS entries are of the form:
//   WKS:<port>:<procname>:<imagename>:<status>:<privs>:<priority>:
// (cont) <queue-limit>:<maxsrv>

    {
extern	STR$COPY_DX();
extern	void Seg$WKS_Config();
    signed long
	CHR,LPTR,
	WKSprname [STRSIZ],
	WKSprname_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= WKSprname),
	WKSimname : VECTOR->STRSIZ,
	WKSimname_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= WKSimname),
	QUOTAS : BLOCK[MAXQUOTA*QUOTA_BLEN] FIELD(QUOTA_FIELDS),
	Quota_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= Quotas),
	WKSInput : VECTOR->STRSIZ,
	WKSInput_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= WKSInput),
	WKSOutput : VECTOR->STRSIZ,
	WKSOutput_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= WKSOutput),
	WKSError : VECTOR->STRSIZ,
	WKSError_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= WKSError),
	WKSport,
	WKSstat,
	WKSpriv[2],
	WKSprior,
	WKSqlim,
	WKSmaxsrv;

// Skip delimiter
    SKIPTO(%C":");

// First, get the port number
    if (GET_DEC_NUM(LINPTR,WKSport) LSS 0)
	Config_Err(%ASCID"Bad WKS port value");
    SKIPTO(%C":");

// Next, process name
    WKSprname_Desc [dsc$w_length] = GETFIELD(WKSprname);
    SKIPTO(%C":");

// Next, image name - N.B. image name may not have ":" in it.
    WKSimname_Desc = GETFIELD(WKSimname);
    SKIPTO(%C":");

// Next, $CREPRC status flags. Comma-separated list of keywords.
    WKSstat = PARSE_PRCSTATUS();

// Next, $CREPRC priviliges. Comma-separated list of keywords.
    PARSE_PRCPRIVS(WKSPRIV);

// Parse the process quotas
    if (PARSE_NULLFIELD())
	SKIPTO(%C":")
    else
	Quota_Desc->dsc$w_length = PARSE_PRCQUOTAS(QUOTAS,MAXQUOTA)*QUOTA_BLEN;

// Parse the process INPUT file
    if (PARSE_NULLFIELD())
	{
	SKIPTO(%C":");
	STR$COPY_DX ( WKSInput_Desc , %ASCID"_NLA0:" )
	}
    else
	{
	WKSInput_Desc [dsc$w_length] = GETFIELD(WKSInput);
	SKIPTO(%C":")
	};

// Parse the process OUTPUT file
    if (PARSE_NULLFIELD())
	{
	SKIPTO(%C":");
	STR$COPY_DX ( WKSOutput_Desc , %ASCID"_NLA0:" )
	}
    else
	{
	WKSOutput_Desc [dsc$w_length] = GETFIELD(WKSOutput);
	SKIPTO(%C":")
	};

// Parse the process ERROR stream
    if (PARSE_NULLFIELD())
	{
	SKIPTO(%C":");
	STR$COPY_DX ( WKSError_Desc , %ASCID"_NLA0:" );
	}
    else
	{
	WKSError_Desc [dsc$w_length] = GETFIELD(WKSError);
	SKIPTO(%C":")
	};

// Next, priority value
    if (GET_DEC_NUM(LINPTR,WKSprior) LSS 0)
	Config_Err(%ASCID"Bad priority value");
    SKIPTO(%C":");

// Next the number of outstanding SYN's we will allow for this server.
    if (GET_DEC_NUM(LINPTR,WKSqlim) LSS 0)
	Config_Err(%ASCID"Bad queue limit value");    

// Now, find out what the maximum number of servers permitted is.  If zero
// or error, then set to unlimited (0).  (This is actually GLBOAL_MAXSRV as
// defined in TCP_SEGIN.BLI.
//
	WKSmaxsrv = 0;
	if ((CH$RCHAR_A(LINPTR) == %C":"))
	    if (GET_DEC_NUM(LINPTR,WKSmaxsrv) LSS 0)
		WKSmaxsrv = 0;

// Tell the configuration routine in SEGIN about this guy.

    Seg$WKS_Config(WKSport, WKSprname_Desc, WKSimname_Desc, WKSstat,
		WKSpriv,WKSprior,WKSqlim,WKSmaxsrv,Quota_Desc,
		WKSInput_Desc,WKSOutput_Desc,WKSError_Desc);
    }

//SBTTL "Init_RPC - Add an RPC entry"
		     
void Init_RPC (void)

// Handle an RPC entry in the INET$CONFIG file.
// Parses RPC description and calles RPC$Config to add the info.
// RPC entries are of the form:
//   RPC:<name>:<prog>:<vers>:<prot>:<port>:<imagename>

    {
extern	STR$COPY_DX	();
extern	RPC$CONFIG();
    signed long
	RC,
	CHR,LPTR,
	RPCprog,
	RPCvers,
	RPCprot,
	RPCport,
	RPCname[STRSIZ],
	RPCname_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= RPCname),
	RPCimname [STRSIZ],
	RPCimname_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= RPCimname);

// Skip delimiter
    SKIPTO(%C":");

// First, get the service name
    RPCname_Desc [dsc$w_length] = GETFIELD(RPCname);
    SKIPTO(%C":");

// Next, get the program number
    if (GET_DEC_NUM(LINPTR,RPCprog) LSS 0)
	Config_Err(%ASCID"Bad RPC program number");
    SKIPTO(%C":");

// Next, get the version number
    if (GET_DEC_NUM(LINPTR,RPCvers) LSS 0)
	Config_Err(%ASCID"Bad RPC version number");
    SKIPTO(%C":");

// Next, get the protocol number
    if (GET_DEC_NUM(LINPTR,RPCprot) LSS 0)
	Config_Err(%ASCID"Bad RPC protocol number");
    SKIPTO(%C":");

// Next, get the port number
    if (GET_DEC_NUM(LINPTR,RPCport) LSS 0)
	Config_Err(%ASCID"Bad RPC port number");
    SKIPTO(%C":");

// Next, image name - N.B. image name may not have ":" in it.
    RPCimname_Desc = GETFIELD(RPCimname);
//    SKIPTO(%C":");

// Tell the configuration routine in the RPC module about this guy.

    RC=RPC$CONFIG( RPCname_Desc,RPCprog,RPCvers,RPCprot,RPCport,RPCimname_Desc);
    if (RC LSS 0) Config_Err(%ASCID"Can not accept RPC config entry");
    }

//SBTTL "Init_Auth - Add an authorization entry"
		     
void Init_Auth (void)

// Handle an AUTH entry in the INET$CONFIG file.
// Parses AUTH description and calles RPC$Config_Auth to add the info.
// AUTH entries are of the form:
//   AUTH:<[UIC]>:<uid>:<gid>:<hostname>

    {
extern	STR$COPY_DX	();
extern	RPC$CONFIG_AUTH();
    signed long
	RC,
	CHR,LPTR,
	AUTHuic,
	AUTHuid,
	AUTHgid,
	AUTHhostname [STRSIZ],
	AUTHhostname_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= AUTHhostname);

// Skip delimiter
    SKIPTO(%C":");

// First, get the VMS uic number
    if (GET_HEX_NUM(LINPTR,AUTHuic) LSS 0)
	Config_Err(%ASCID"Bad AUTH uic number");
    SKIPTO(%C":");

// Next, get the net uid number
    if (GET_DEC_NUM(LINPTR,AUTHuid) LSS 0)
	Config_Err(%ASCID"Bad AUTH uid number");
    SKIPTO(%C":");

// Next, host name - N.B. host name may not have ":" in it.
    AUTHhostname_Desc = GETFIELD(AUTHhostname);
//    SKIPTO(%C":");

// Tell the configuration routine in the RPC module about this guy.

    RC=RPC$CONFIG_AUTH(AUTHuic, AUTHuid, AUTHgid, AUTHhostname_Desc);
    if ((RC LSS 0)) Config_Err(%ASCID"Can not accept AUTH config entry");
    }

//SBTTL "Init_MBXResolver - Define the system name resolver process"

void Init_MBXResolver (void)
//
// Define the system name resolver process.
// MBX_RESOLVER:<image>:<priority>:<flags>:<privs>:<quotas>
//
    {
extern	void NML$CONFIG();
    signed long
	IMAGENAME [STRSIZ],
	ImageName_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= imagename),
	STATFLAGS,
	PRIORITY,
	PRIVS : VECTOR[2],
	QUOTAS : BLOCK[MAXQUOTA*QUOTA_BLEN] FIELD(QUOTA_FIELDS),
	Quota_Desc	: $BBLOCK [DSC$K_Z_BLN] PRESET (
				[dsc$w_length]	= 0,
				[dsc$b_dtype]	= DSC$K_DTYPE_Z,
				[dsc$b_class]	= DSC$K_CLASS_Z,
				[dsc$a_pointer]	= Quotas);

// Skip delimiter

    SKIPTO(%C":");

// Parse the image name

    ImageName_Desc [dsc$w_length] = GETFIELD(IMAGENAME);
    SKIPTO(%C":");

// Parse the priority value

    if (GET_DEC_NUM(LINPTR,PRIORITY) LSS 0)
	Config_Err(%ASCID"Bad priority value");
    SKIPTO(%C":");

// Parse the process status values

    STATFLAGS = PARSE_PRCSTATUS();

// Parse the process privileges

    PARSE_PRCPRIVS(PRIVS);

// Parse the process quotas

    Quota_Desc [dsc$w_length] = PARSE_PRCQUOTAS(QUOTAS,MAXQUOTA) * QUOTA_BLEN;

// Call the name resolver configuration routine

    NML$CONFIG(IMAGENAME_Desc, PRIORITY, STATFLAGS, PRIVS,
	       QUOTA_Desc);
    }

//SBTTL "Add an entry to the local hosts list"

void INIT_LOCAL_HOST (void)
//
// Format is Local_Host:<ip-address>:<ip-mask>
// Reads the data and calls USER$ACCESS_CONFIG(ipaddr,ipmask)
//
    {
extern	void USER$ACCESS_CONFIG();
    signed long
	hostaddr,
	hostmask;

// First, read the host address

    SKIPTO(%C":");
    if (GET_IP_ADDR(LINPTR,hostaddr) LSS 0)
	Config_err(%ASCID"Bad host address");

// Then, the host mask

    SKIPTO(%C":");
    if (GET_IP_ADDR(LINPTR,hostmask) LSS 0)
	Config_err(%ASCID"Bad host mask");

// Got the info. Call routine to add to the list

    USER$ACCESS_CONFIG(hostaddr,hostmask);
    }

//SBTTL "Parsing utility routines"

SKIPTO(TCHR) : NOVALUE (void)

// Skip characters past a given character in line buffer.

    {
    signed long
	CHR;
    while ((CHR = CH$RCHAR_A(LINPTR)) != 0)
	if (CHR == TCHR)
	    return;
    Config_err(%ASCID"SKIPTO failure (EOL)");
    }

void SKIPWHITE (void)

// Skip over whitespace in the current record.

    {
    signed long
        CHR,LPTR;
    LPTR = LINPTR;
    while ((CHR = CH$RCHAR_A(LPTR)) != 0)
	{
	if (CHR != %C" ")
	    return;
	LINPTR = LPTR;
	};
    Config_err(%ASCID"SKIPWHITE failure (EOL)");
    }

GETFIELD(FLDADR)
    
// Read a field the current record (LINPTR). A field is defined as the a
// sequence of non-whitespace characters up to whitespace or a field
// delimiter (currently ",', '=' and ':"). Returns field size on success
// with the field text (case-folded) at FLDADR.
// Returns 0 on failure (end of record).

    {
    signed long
        PTR,CHR,CNT,DSTPTR;

    SKIPWHITE();
    PTR = LINPTR;
    DSTPTR = CH$PTR(FLDADR);
    CNT = 0;
    WHILE ((CHR = CH$RCHAR_A(PTR)) != 0) AND
	  (CHR != %C",') && (CHR != %C':') && (CHR != %C' ") AND
	  (CHR != %C"=") DO
	{
	LINPTR = PTR;
	CNT = CNT+1;
	if ((CHR GEQ %C"a') && (CHR <= %C'z"))
	    CHR = CHR-(%C"a'-%C'A");
	CH$WCHAR_A(CHR,DSTPTR);
	};
    CH$WCHAR_A(0,DSTPTR);
    if (cnt == 0)
	Config_err(%ASCID"Bad or null field found");
    return cnt;
    }

PARSE_NULLFIELD (void)
    
// Read a field the current record (LINPTR). A field is defined as the a
// sequence of non-whitespace characters up to whitespace or a field
// delimiter (currently ",', '=' and ':"). Returns field size on success
// with the field text (case-folded) at FLDADR.
// Returns 0 on failure (end of record).

    {
    signed long
        PTR,CHR;

    SKIPWHITE();
    PTR = LINPTR;

    IF ((CHR = CH$RCHAR_A(PTR)) != 0) AND
	  (CHR != %C",') && (CHR != %C':') && (CHR != %C' ") AND
	  (CHR != %C"=") THEN 0
    else 1

    }

//Sbttl "Initialize Network Devices as described in dev_config."
/*

Function:

	Initialize all known network devices from dev_config table.

Inputs:

	None.

Implicit Inputs:

	dev_config table entries.

Outputs:
]
	None.

Side Effects:

	if the dev_config table entry is marked as a valid entry then
	the device init routine is called.  This routine will assign the
	device & place the IO channel in the dev_config table entry dc_io_chan.
*/


void CNF$Net_Device_Init (void)
    {
    externAL
	RETRY_COUNT,
	MAX_PHYSICAL_BUFSIZE;
    signed long
	j,
      cdev;
	struct Device_Configuration_Entry * dev_config;

    Dev_attn = 0;		// No devices need attention
    for (j=0;j<=(Dev_count-1);j++)
	{
	if (Dev_Config_Tab[J,dc_valid_Device])
	    {
DESC$STR_ALLOC(oprmsg,80);
DESC$STR_ALLOC(ipstr,20);
		DESC$STR_ALLOC(phystr,50);
	    signed long
		struct $BBLOCK * devnam->DSC$K_S_BLN,
      stastr;

// Initialize this device

	    if (Dev_Config_Tab[j,dc_is_clone])
		cdev = Dev_Config_Tab[j,dc_clone_dev]
	    else
		{
		cdev = j;
		(Dev_config_Tab[j,dc_rtn_init])(Dev_config_Tab[j,dc_begin],
				IPACP_Int,
				.RETRY_COUNT,
				.MAX_PHYSICAL_BUFSIZE);
		};

// And tell the operator the status.

	    devnam = dev_config_tab[j,dc_devname];
//	    ASCII_Hex_Bytes(phystr,dev_config_tab[cdev,dc_phy_size],
//			    dev_config[cdev,dc_phy_addr],
//			    phystr->dsc$w_length);
	    ASCII_Dec_Bytes(ipstr,4,dev_config_tab[j,dc_ip_address],
			    ipstr->dsc$w_length);
	    if (dev_config_tab[cdev,dc_online])
		stastr = %ASCID"Online"
	    else
		stastr = %ASCID"Offline";

	    OPR$FAO("Net device !SL is !AS, IP=!AS (!AS)",
		    j,devnam,ipstr,stastr);

	    dev_config_tab[cdev,dcmib_ifIndex] = cdev;

	    };
	};
    }


//SBTTL	"Check devices needing attention"

 void    CNF$Check_Devices();

static signed long
    CHECKTIME [2] ={-20000000,-1}; // 2 seconds in the future

void CNF$Check_Sched (void)
    {
    $SETIMR(	DAYTIM = CHECKTIME,
		ASTADR = CNF$Check_Devices);
    }

void CNF$Device_Error (void)
    {
    if (dev_attn == 0)
	CNF$Check_Sched();		// Schedule a check
    dev_attn = dev_attn+1;	// And bump count of wedged devices
    }

void CNF$Check_Devices (void)
    {
    register i;
    for (i=0;i<=(Dev_count-1);i++)
	IF Dev_Config_tab[i,dc_valid_device] && 
	   (NOT Dev_Config_tab[i,dc_online]) AND
	   (NOT Dev_Config_tab[i,dc_is_clone]) THEN
	     // Try to restart
	    dev_attn = dev_attn + (Dev_config_tab[i,dc_rtn_check])(
						Dev_config_tab[i,dc_begin]);

    if (dev_attn != 0)
	CNF$Check_Sched();		// Reschedule if there is still a problem
    }



//
// A simple routine to return our local IP address.  By using this, we
// cut down on external access to the device configuration table.
//
CNF$Get_Local_IP_addr (void)
    {
    Dev_Config_Tab[0,dc_ip_address]
    }

}
ELUDOM

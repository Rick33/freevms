/*
================================================================================
  DCL/2 version 1.00
  	Digital Command Language

    Date de cr�ation    : 28 novembre 2001

    Tous droits r�serv�s � l'auteur, Jo�l BERTRAND
	All rights reserved worldwide
================================================================================
*/


/*
================================================================================
  Copyright (C) 2001 BERTRAND Jo�l

  This file is part of DCL/2.

  DCL/2 is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2, or (at your option) any
  later version.
            
  DCL/2 is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.
              
  You should have received a copy of the GNU General Public License
  along with Octave; see the file COPYING.  If not, write to the Free
  Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
================================================================================
*/

#ifndef DCL$DCL
#	define DCL$DCL

#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
#	include <stdarg.h>
#	include <signal.h>
#	include <unistd.h>

#	include <readline/readline.h>
#	include <readline/history.h>

#	include <sys/types.h>
#	include <sys/wait.h>

/*
--------------------------------------------------------------------------------
  DCL return values
--------------------------------------------------------------------------------
*/

#	define		DCL$FAILURE		-1
#	define		DCL$SUCCESS		0

/*
--------------------------------------------------------------------------------
  DCL errors
--------------------------------------------------------------------------------
*/

//	ambiguous command verb - supply more characters
#	define		DCL$WABVERB		1
//	unrecognized command verb - check validity and spelling
#	define		DCL$WIVVERB		2
//	unrecognized keyword - check validity and spelling
#	define		DCL$WIVKEYW		3
//	unrecognized qualifier - check validity, spelling, and placement
#	define		DCL$WIVQUAL		4

/*
--------------------------------------------------------------------------------
  DCL compatibilities
--------------------------------------------------------------------------------
*/

#	define		DCL$NONE			0
#	define		DCL$END_OF_LIST		-1
#	define		DCL$NB_FLAGS		50
#	define		DCL$NB_COMP			(1 + (DCL$NB_FLAGS / (8 * sizeof(char))))

#	define		DCL$Q_ACL						1
#	define		DCL$Q_ALLOCATION				2
#	define		DCL$Q_BACKUP					3
#	define		DCL$Q_BEFORE					4
#	define		DCL$Q_BRIEF						5
#	define		DCL$Q_BY_OWNER					6
#	define		DCL$Q_COLUMNS					7
#	define		DCL$Q_CONCATENATE				8
#	define		DCL$Q_CONFIRM					9
#	define		DCL$Q_CONTIGUOUS				10
#	define		DCL$Q_CREATED					11
#	define		DCL$Q_DATE						12
#	define		DCL$Q_EXACT						13
#	define		DCL$Q_EXCLUDE					14
#	define		DCL$Q_EXPIRED					15
#	define		DCL$Q_EXTENSION					16
#	define		DCL$Q_FILE_ID					17
#	define		DCL$Q_FTP						18
#	define		DCL$Q_FULL						19
#	define		DCL$Q_GRAND_TOTAL				20
#	define		DCL$Q_HEADING					21
#	define		DCL$Q_HIGHLIGHT					22
#	define		DCL$Q_LOG						23
#	define		DCL$Q_MODIFIED					24
#	define		DCL$Q_OUTPUT					25
#	define		DCL$Q_OVERLAY					26
#	define		DCL$Q_OWNER						27
#	define		DCL$Q_PAGE						28
#	define		DCL$Q_PRINTER					29
#	define		DCL$Q_PROTECTION				30
#	define		DCL$Q_RCP						31
#	define		DCL$Q_READ_CHECK				32
#	define		DCL$Q_REPLACE					33
#	define		DCL$Q_SEARCH					34
#	define		DCL$Q_SECURITY					35
#	define		DCL$Q_SELECT					36
#	define		DCL$Q_SHELVED_STATE				37
#	define		DCL$Q_SINCE						38
#	define		DCL$Q_SIZE						39
#	define		DCL$Q_STYLE						40
#	define		DCL$Q_TIME						41
#	define		DCL$Q_TOTAL						42
#	define		DCL$Q_TRAILING					43
#	define		DCL$Q_TRUNCATE					44
#	define		DCL$Q_VERSIONS					45
#	define		DCL$Q_VOLUME					46
#	define		DCL$Q_WIDTH						47
#	define		DCL$Q_WRAP						48
#	define		DCL$Q_WRITE_CHECK				49

/*
--------------------------------------------------------------------------------
  DCL command structure
--------------------------------------------------------------------------------
*/

#	define		DCL$VERB		0x01
#	define		DCL$KEYWORD		0x02
#	define		DCL$QUALIFIER	0x04

	typedef struct dcl_env
	{
		int						end_flag;
		int						historic_length;
		unsigned char			*last_error;
		unsigned char			*prompt;
	} dcl$env;

	typedef struct dcl_command
	{
		unsigned char		*name;		// static not allocated by malloc()
		unsigned char		compat[DCL$NB_COMP];
		int					(*function)(unsigned char *argument,
									struct dcl_command *self,
								   	struct dcl_command *commands,
									dcl$env *env);
		int					qualifier;
		int					type;
		int					length;
		struct dcl_command	*next;
	} dcl$command;


/*
--------------------------------------------------------------------------------
  Protoypes
--------------------------------------------------------------------------------
*/

	dcl$command *command_add_on(dcl$command *commands,
			unsigned char *name, int (*function)(), int type,
			int qualifier,...);

	int commands_init(dcl$command **commands);
	int get_compat_flag(dcl$command *command, int flag);
	int loop(dcl$command *commands, dcl$env *env);
	int parsing(unsigned char *line, dcl$command *commands, dcl$env *env,
			int required_type, dcl$command *parent);

	unsigned char *next_argument(unsigned char *ptr);
	unsigned char *read_command(dcl$env *env);

	void commands_freeing(dcl$command *commands);
	void set_compat_flag(dcl$command *command, int flag);

/*
--------------------------------------------------------------------------------
  Builtins
--------------------------------------------------------------------------------
*/

	int copy_function(unsigned char *argument, dcl$command *self,
			dcl$command *commands, dcl$env *env);

	int directory_function(unsigned char *argument, dcl$command *self,
			dcl$command *commands, dcl$env *env);

	int help_function(unsigned char *argument, dcl$command *self,
			dcl$command *commands, dcl$env *env);

	int logout_function(unsigned char *argument, dcl$command *self,
			dcl$command *commands, dcl$env *env);

	int run_function(unsigned char *argument, dcl$command *self,
			dcl$command *commands, dcl$env *env);

	int set_function(unsigned char *argument, dcl$command *self,
			dcl$command *commands, dcl$env *env);
	int show_function(unsigned char *argument, dcl$command *self,
			dcl$command *commands, dcl$env *env);
#endif

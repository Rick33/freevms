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


#include "dcl.h"

int
run_function(unsigned char *argument, dcl$command *self,
		dcl$command *commands, dcl$env *env)
/*
RUN

     Executes an image within the context of your process (see Image).

     Creates a subprocess or a detached process to run an image and
     deletes the process when the image completes execution (see
     Process).
*/
{
	dcl$arg				*arg;
	dcl$arg				*new;

	int					status;

	arg = NULL;

	while((*argument) != 0)
	{
		argument = next_argument(argument);

		if ((new = malloc(sizeof(dcl$arg))) == NULL)
		{
			return(DCL$FAILURE);
		}

		(*new).next = arg;
		(*new).argument = argument;
		arg = new;

		/* Fixer les probl�mes de parenth�ses */
		while(((*argument) != ' ') && ((*argument) != 0))
		{
			argument++;
		}

		if ((*argument) == ' ')
		{
			(*argument) = 0;
			argument++;
		}
	}

	status = call(arg, env);

	free(arg);
	return(status);
}

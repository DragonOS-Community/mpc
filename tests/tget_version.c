/* tget_version.c -- Test file for mpc_get_version

Copyright (C) 2002, 2003, 2004, 2005, 2007, 2008 Andreas Enge, Paul Zimmermann, Philippe Th\'eveny

This file is part of the MPC Library.

The MPC Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The MPC Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the MPC Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpc.h"

int
main (void)
{
  if (strcmp (mpc_get_version (), MPC_VERSION_STRING) != 0)
    {
      printf ("Error: header and library do not match\n"
              "mpc_get_version: \"%s\"\nMPC_VERSION_STRING: \"%s\"\n",
              mpc_get_version(), MPC_VERSION_STRING);
      exit (1);
    }

  return 0;
}
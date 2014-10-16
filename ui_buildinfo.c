/*
 *                   ##
 *                  ##             cafe (CaFE) : Calcurator, Function Expandable
 *                  ##             
 *   ###   ####   ######  ###       an RPN calculator for command line interface
 * ##     ##  ##    ##   ## ##
 * ##     ##  ##    ##   ###             (c) Tsukimidai Communications Syndicate
 *   ###   ### ##   ##    ####               1991 - 2010
 */

/**
***   version 2.0 release 0.5
**/

#include	"cafe2.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"ui.h"


int		g_build_number;

void title( void );


void ui_buildinfo( void )
{
	char	s[ 80 ];

#if 1
	cprintf( BOLD, CONT, "%s\n", version_string( s ) );
#else
	title();
#endif
}

char *version_string( char *str )
{
	sprintf( str, "CaFE version%s %s %s (pack %s)", VERSION_INFO, __DATE__, __TIME__, BUILDNUM );
	return ( str );
}

void ui_version_information( void )
{
	cprintf( BOLD, CONT, "\n" );
	cprintf( BOLD, CONT, "CaFE\n" );
	cprintf( BOLD, CONT, "    version : %s\n", VERSION_INFO );
	cprintf( BOLD, CONT, "    packed  : %s\n", BUILDNUM );
	cprintf( BOLD, CONT, "    build   : %s %s for %s\n", __DATE__, __TIME__, BUILDFOR );
	cprintf( BOLD, CONT, "\n" );
	cprintf( BOLD, CONT, "    int      size : %d bits\n", sizeof( long ) * 8 );
	cprintf( BOLD, CONT, "    floating size : %d bits\n", sizeof( double ) * 8 );
	cprintf( BOLD, CONT, "\n" );
	cprintf( BOLD, CONT, "    std lib       : %s\n", STDLIB_LOCATION );
	cprintf( BOLD, CONT, "\n" );
}


void ui_information( void )
{
	ui_version_information();
	
	cprintf( BOLD, CONT, "    options : \n" );
	cprintf( BOLD, CONT, "        -?            " );
	cprintf( NORM, CONT, ": to see this information\n" );
	cprintf( BOLD, CONT, "        -!            " );
	cprintf( NORM, CONT, ": setting non-interactive mode without standard input\n" );
	cprintf( BOLD, CONT, "        -f file_name  " );
	cprintf( NORM, CONT, ": set file to load\n" );
	cprintf( BOLD, CONT, "        -L            " );
	cprintf( NORM, CONT, ": (default) use all file load and save options\n" );
	cprintf( BOLD, CONT, "        -L-           " );
	cprintf( NORM, CONT, ": NOT to use all file load and save options\n" );
	cprintf( BOLD, CONT, "        -H            " );
	cprintf( NORM, CONT, ": (default) load history file at program launch and \n" );
	cprintf( NORM, CONT, "                        save history at program quit\n" );
	cprintf( BOLD, CONT, "        -H-           " );
	cprintf( NORM, CONT, ": (default) load history file at program launch and \n" );
	cprintf( NORM, CONT, "                        save history at program quit\n" );
	cprintf( BOLD, CONT, "        -S            " );
	cprintf( NORM, CONT, ": (default) save history at program quit\n" );
	cprintf( BOLD, CONT, "        -S-           " );
	cprintf( NORM, CONT, ": NOT to save history at program quit\n" );
	cprintf( NORM, CONT, "\n" );
}


void title( void )
{
#if 0
	cprintf( BOLD, NORM, "        @@@@          @@@@@  @@@@@    CaFE (Calculator, Function Expandable)  \n" );
	cprintf( BOLD, NORM, "      @@     @@@@    @@     @@        version %s\n", VERSION_INFO );
	cprintf( BOLD, NORM, "     @@    @@  @@   @@@@   @@@@       [%s @ %s]    \n", __DATE__, __TIME__ );
	cprintf( BOLD, NORM, "    @@    @@  @@   @@     @@          (c) Tsukimidai Communications Inc.      \n" );
	cprintf( BOLD, NORM, "    @@@@   @@@ @  @@     @@@@@            All rights reserved. 1991-2005      \n" );
#else
	cprintf( BOLD, NORM, " 		      ##       \n" );
	cprintf( BOLD, NORM, "		     #         cafe : calcurator function expandable  \n" );
	cprintf( BOLD, NORM, "		     #		  \n" );
	cprintf( BOLD, NORM, " ##  ###  ###  ##    (c) Tsukimidai Communications Syndicate\n" );
	cprintf( BOLD, NORM, "#   #  #   #  #  #       all rights reserved 1991-2005\n" );
	cprintf( BOLD, NORM, "#   #  #   #  # #    version %s\n", VERSION_INFO );
	cprintf( BOLD, NORM, " ##  ## # #    ###   [%s @ %s]\n", __DATE__, __TIME__ );
#endif
}



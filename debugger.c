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

#include	"debugger.h"
#include	"ui.h"
#include	"key.h"


name_chain	g_debugger_func_nc	=	{
										"",
										NULL
									};
									
name_chain	*g_debugger_func_nc_p	= &g_debugger_func_nc;


int		g_debugger_exec_level	= 0;

string_object	debugging_custom_command[ 256 ]		= { NULL };


void debugger_enable( void )
{
	int		current_exec_level;

	current_exec_level	= get_recursive_level();

	if ( current_exec_level )
		g_debugger_exec_level	= current_exec_level;
	else
		g_debugger_exec_level	= 1;
}

void debugger_disable( void )
{
	g_debugger_exec_level	= 0;
}

void debugger_set_level( int current_exec_level )
{
	if ( !g_debugger_exec_level )
		return;
		
	if ( current_exec_level < g_debugger_exec_level )
	{
		if ( current_exec_level == 0 )
			g_debugger_exec_level	= 1;
		else
			g_debugger_exec_level	= current_exec_level;
	}
		
}

void debugger( string_object next_token )
{
	static string_object	prev_command		= NULL;
	int						current_exec_level;
	int						exit				= FALSE;
	int						c;

	current_exec_level	= get_recursive_level();

	if ( !prev_command )
		prev_command	= make_string_object( "", -1 );

	if ( (g_debugger_exec_level) && (current_exec_level <= g_debugger_exec_level) )
	{		
//		cprintf( NORM, CONT, "**** debugging mode   current executing function ****\n" );
		cprintf( NORM, CONT, "<< in function \"" );
		cprintf( BOLD, CONT, "%s", g_debugger_func_nc_p->name );
		cprintf( NORM, CONT, "\"  " );
		cprintf( NORM, CONT, "[prev:\"" );
		cprintf( BOLD, CONT, "%s", prev_command );
		cprintf( NORM, CONT, "\"]  [next:\"" );
		cprintf( BOLD, CONT, "%s", next_token );
		cprintf( NORM, CONT, "\"]  [d/r:" );
		cprintf( BOLD, CONT, "%d", g_debugger_exec_level );
		cprintf( NORM, CONT, "/" );
		cprintf( BOLD, CONT, "%d", current_exec_level );
		cprintf( NORM, CONT, "] [faf=" );
		cprintf( BOLD, CONT, "%d", g_cafe_mode.function_exe_abort.value );
		cprintf( NORM, CONT, "] >>\n" );
		
		do
		{
			cprintf( NORM, CONT, "\rdebug > " );

			c	= getch();
			
			if ( debugging_custom_command[ (unsigned char)c ] )
			{
				cprintf( NORM, CONT, "executing \"" );
				cprintf( BOLD, CONT, "%s", debugging_custom_command[ (unsigned char)c ] );
				cprintf( NORM, CONT, "\"\n" );

				evaluate( debugging_custom_command[ (unsigned char)c ] );
			}

			switch ( c )
			{
				case 'n' : 
				case ' ' : 
					exit	= TRUE;
					break;
				case 'i' :
					if ( current_exec_level == g_debugger_exec_level )
						g_debugger_exec_level++;
					exit	= TRUE;
					break;
				case 'o' :
					g_debugger_exec_level	= current_exec_level - 1;
					exit	= TRUE;
					break;
				case 'g' :
					debugger_disable();
					exit	= TRUE;
					break;
				case 'q' :
					cprintf( ERROR, CONT, "error condition has been thrown to quit from the debugging mode.\n" );
					debugger_disable();
					exit	= TRUE;
					break;
				case 's' :
					cprintf( NORM, CONT, "\n" );
					show_stack( NULL );
					break;
				case '?' :
					cprintf( NORM, CONT, "\n" );
					cprintf( BOLD, CONT, "    n" );
					cprintf( NORM, CONT, "        : execute next command\n" );
					cprintf( BOLD, CONT, "    [return]" );
					cprintf( NORM, CONT,        " : execute next command\n" );
					cprintf( BOLD, CONT, "    [space]" );
					cprintf( NORM, CONT,       "  : execute next command\n" );
					cprintf( BOLD, CONT, "    i" );
					cprintf( NORM, CONT, "        : step into function\n" );
					cprintf( BOLD, CONT, "    o" );
					cprintf( NORM, CONT, "        : step out from function\n" );
					cprintf( BOLD, CONT, "    g" );
					cprintf( NORM, CONT, "        : go to next breakpoint\n" );
					cprintf( BOLD, CONT, "    s" );
					cprintf( NORM, CONT, "        : show stack\n" );
					cprintf( BOLD, CONT, "    q" );
					cprintf( NORM, CONT, "        : abort from debug mode\n" );
					cprintf( BOLD, CONT, "    e" );
					cprintf( NORM, CONT, "        : evaluate key input\n" );
					break;
				case 'e' :
					evaluate( key_input( PROMPT_STR ) );
					break;
				case 'h' :
				case 'v' :
					break;
				default :
					cprintf( NORM, CONT, "?" );
					break;
			}
		}
		while ( !exit );
		cprintf( NORM, CONT, "\n" );
	}
	dispose_string_object( prev_command );
	prev_command	= make_string_object( next_token, -1 );
}


int debugger_get_debugger_exec_level( void )
{
	return ( g_debugger_exec_level );
}
			

void debugger_set_debugger_exec_level( int v )
{
	g_debugger_exec_level	= v;
}
			

void debugger_push_name_chain( string_object s )
{
	name_chain_push( &g_debugger_func_nc_p, s );
}


void debugger_pop_name_chain( void )
{
	name_chain_pop( &g_debugger_func_nc_p );
}


string_object debugger_get_current_function_name( void )
{
	return ( g_debugger_func_nc_p->name );
}


void debugger_set_custom_command( void )
{
	string_object	command;
	string_object	index;
	
	if ( !(command	= pop_s()) )
		return;

	if ( !(index	= pop_s()) )
		return;
	
	if ( *((char *)command) )
		debugging_custom_command[ (int)*((char *)index) ]	= make_string_object( command, -1 );
	else
		debugging_custom_command[ (int)*((char *)index) ]	= NULL;
}



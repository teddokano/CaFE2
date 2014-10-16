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

#ifndef		DEBUGGER_H
#define		DEBUGGER_H

void debugger_enable( void );
void debugger_disable( void );
void debugger_set_level( int current_exec_level );
void debugger( string_object next_token );
string_object debugger_get_current_function_name( void );
void debugger_set_custom_command( void );

int		debugger_get_debugger_exec_level( void );
void debugger_set_debugger_exec_level( int v );

void debugger_push_name_chain( string_object s );
void debugger_pop_name_chain( void );


#endif	//	DEBUGGER_H



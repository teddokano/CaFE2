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

#ifndef		COMMAND_TABLE_H
#define		COMMAND_TABLE_H



#include	"functions.h"



typedef			void 				(* funcPtr)();

typedef	struct command	{
							char	*str;
							void	(*buildin_command)( char **src_p );
							char	*help;
						}
						command;


void	initialize_keywords( void );
void	add_keywd_to_key( char *key );
void	add_function_to_key( function *fnc_ptr );
void	remove_function_from_key( function *fnc_ptr );
void	remove_keywd_from_key( char *key );
int		find_n_match_candidate( char *key );
int		try_token( char *key, char **src_p );

void *find_token( char *key, char **src_p );

#endif	//	COMMAND_TABLE_H




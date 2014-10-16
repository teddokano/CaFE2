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

#ifndef		FUNCTIONS_H
#define		FUNCTIONS_H

#include	"cafe2.h"

//#define	ERROR		-1
//#define	NO_ERROR	0
#define		TEMPORAL_FUNC_DEFINED	10


typedef struct _function	function;

struct	_function	{
						char		name[ MAX_TOKEN_LENGTH ];
						char		*body;
						char		*package_name;
						function	*next;
					};


int		function_define( char **src_p );
int		function_forget( char **src_p );
int		function_rename( char **src_p );
int		function_remove( char **src_p );
int		function_isdefined( char **src_p );
int		function_replace( char **src_p );
void	function_tokenize( void );
void	function_self( void );
void	function_body( char **src_p );

void	function_list( FILE *mode );
void	show_function( char **src_p );

void	function_remove_by_package_name( char *name );

int		functions_compile( string_object *src_p );

string_object function_get_temporal_function_body( void );

#endif //	FUNCTIONS_H




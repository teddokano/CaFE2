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
#include	<ctype.h>

#include	"functions.h"
#include	"key.h"
#include	"command_table.h"
#include	"fileop.h"
#include	"stack.h"
#include	"string.h"
#include	"debugger.h"
#include	"ui.h"



//	functions are stored in a special stack. This stack is implemented as link list. 



function	g_function_list_base	= { "", NULL, NULL, NULL };

static int		rename_func( function *fnc_ptr, char *trg );
static void		get_body( char **body, char **src_p );
static function	*already_defined( char *str, int offset );
static function	*end_of_function_list( void );
static void		forget( function *fnc_ptr );
static int		remove_func( char *name );
string_object	convert_function_body( string_object s );


string_object function_get_temporal_function_body( void )
{
	if ( !(g_function_list_base.body) )
		return ( make_string_object( "", -1 ) );
	else
		return ( make_string_object( g_function_list_base.body, -1 ) );
}

int function_define( char **src_p )
{
	function	*fnc_ptr;
	char		*name;
	int			temporal_flag	= FALSE;
	
	name	= get_next_token( src_p, NORMAL );

	//cprintf( BOLD, CONT, "function_define ..name <%s>\n", name );


	if ( !name || (*name == ';') )
	{
		cprintf( ERROR, CONT, "no function name given for function_define\n" );
		dispose_string_object( name );
		return ( ERROR );
	}
	
	if ( MAX_TOKEN_LENGTH <= strlen( name ) )
	{
		cprintf( ERROR, CONT, "function name too long. max name length is %d chars.\n", MAX_TOKEN_LENGTH - 1 );
		dispose_string_object( name );
		return ( ERROR );
	}

	if ( *name == '@' )
	{
		if ( *(name + 1) == '\0' )
		{
			temporal_flag	= TRUE;
		}
		else
		{
			cprintf( ERROR, CONT, "invalid function name. function name should not start with '@'.\n", MAX_TOKEN_LENGTH - 1 );
			dispose_string_object( name );
			return ( ERROR );
		}
	}	
	
	if ( (fnc_ptr	= already_defined( name, 0 )) )
	{
		/*
		 *		if the function name given as "@", the function already_defined() returns 
		 *		reserved area for temporal function space (i.e function name "").
		 */
	
		if ( !temporal_flag )
			cprintf( WARN, CONT, "function overwriting : \"%s\"\n", name );
		
		if ( (fnc_ptr->body) )				//	the temporal function have NULL in initial state
			free( fnc_ptr->body );
			
		fnc_ptr->body	= NULL;
		
		if ( (fnc_ptr->package_name) )		//	the temporal function have NULL in initial state
			dispose_string_object( fnc_ptr->package_name );
			
		fnc_ptr->package_name	= NULL;
	}
	else  //  newly defining
	{
		fnc_ptr			= end_of_function_list();

		if ( NULL == (fnc_ptr->next	= (function *)malloc( sizeof( function ) )) )
		{
			cprintf( ERROR, CONT, "malloc @ function_define()\n" );
			return ( ERROR );
		}
			
		fnc_ptr			= fnc_ptr->next;		//	update pointer to next
		
		fnc_ptr->body			= NULL;
		fnc_ptr->next			= NULL;
		fnc_ptr->package_name	= NULL;
		
		strncpy( fnc_ptr->name, name, MAX_TOKEN_LENGTH );
		
		add_function_to_key( fnc_ptr );
	}

	dispose_string_object( name );


	fnc_ptr->package_name	= make_string_object( file_get_loading_file_name(), -1 );

//cprintf( NORM, CONT, "function defining \"%s\" in \"%s\"\n", fnc_ptr->name, fnc_ptr->package_name );

	get_body( &(fnc_ptr->body), src_p );


//cprintf( NORM, CONT, "function defining \"%s\" in \"%s\"\n", fnc_ptr->name, fnc_ptr->package_name );
//cprintf( NORM, CONT, ">>>\"%s\"\n", fnc_ptr->body );


	return ( temporal_flag ? TEMPORAL_FUNC_DEFINED : NO_ERROR );
}


static void get_body( char **body, char **src_p )
{
	char	*token;
	char	level		= 1;

	while ( (token	= get_next_token( src_p, FILE_PROCESSING )) )
	{
		if ( *token == ':' )
			level++;
		else if ( *token == ';' )
			level--;

		if ( !level )
			break;

		*body	= add_string( *body, token, WITH_SPACE );

		dispose_string_object( token );
		token	= NULL;
	}

	if ( token )
	{
		dispose_string_object( token );
		token	= NULL;
	}

	*body	= add_string( *body, "", WITHOUT_SPACE );
}


int function_forget( char **src_p )
{
	function	*fnc_ptr;
	char		*name;

	name	= get_next_token( src_p, NORMAL );

	if ( !name )
	{
		cprintf( ERROR, CONT, "no function name given for function_forget\n" );
		return ( ERROR );
	}
	
	if ( (fnc_ptr	= already_defined( name, -1 )) )
	{		
		forget( fnc_ptr->next );
		fnc_ptr->next	= NULL;
 	}
	else
	{
		cprintf( NORM, CONT, "function \"%s\" not found to forget.\n", name );
	}

	dispose_string_object( name );

	return ( NO_ERROR );
}


int function_rename( char **src_p )
{
	function	*fnc_ptr;
	char		*src;
	char		*trg;
	int			result;

	src		= get_next_token( src_p, NORMAL );
	trg		= get_next_token( src_p, NORMAL );

	if ( !src || !trg )
	{
		cprintf( ERROR, CONT, "need two arguments to remname function\n" );
		
		if ( src )
			dispose_string_object( src );

		if ( trg )
			dispose_string_object( trg );

		return ( ERROR );
	}
	
	if ( (fnc_ptr	= already_defined( src, -1 )) )
	{		
		result	= rename_func( fnc_ptr->next, trg );
 	}
	else
	{
		cprintf( WARN, CONT, "function is not found to rename\n" );
		result	= ERROR;
	}

	dispose_string_object( src );
	dispose_string_object( trg );

	return ( result );
}


int rename_func( function *fnc_ptr, char *trg )
{
	remove_function_from_key( fnc_ptr );
	
	strncpy( fnc_ptr->name, trg, MAX_TOKEN_LENGTH );
	
	add_function_to_key( fnc_ptr );
	
	return ( NO_ERROR );
}
	


int function_replace( char **src_p )
{
	function		*fnc_ptr;
	char			*name;

	name	= get_next_token( src_p, NORMAL );

	if ( !name )
		return ( NO_ERROR );
	
	if ( (fnc_ptr	= already_defined( name, 0 )) )
		push_s( fnc_ptr->body );
	else
		push_s( name );
	
	dispose_string_object( name );

	return ( NO_ERROR );
}


void function_tokenize( void )
{
	string_object	sb;
	string_object	s;
	string_object	snb;
	string_object	sn;
	int				count	= 3;	//	"3" for two '\''s and last null character
	char			c;
	
	if ( !(sb	= pop_s()) )
		return;
	
	if ( string_token_count( sb ) <= 1 )
	{
		push_s( sb );
		dispose_string_object( sb );
		return;
	}
	
	s	= sb;

	while ( (c	= *s++) )
	{
		switch ( c )
		{
			case '\\' : 
			case '\'' : 
				count++;
				break;
		}
	}
	
	s	= sb;
	
	snb	= make_string_object( "'", strlen( s ) + count );
	sn	= snb + 1;
	
	while ( *s )
	{
		switch ( *s )
		{
			case '\\' : 
			case '\'' : 
				*sn++	= '\\';
				break;
			default : 
				break;
		}
		*sn++	= *s++;
	}

	*sn++	= '\'';
	*sn		= '\0';

	push_s( snb );

	dispose_string_object( sb );
	dispose_string_object( snb );
}


void function_self( void )
{
	push_s( debugger_get_current_function_name() );
}


void function_body( char **src_p )
{
	function	*fnc_ptr;
	char		*name;
	
	if ( !(name		= get_next_token( src_p, NORMAL )) )
		return;

	if ( (fnc_ptr	= already_defined( name, 0 )) )
		push_s( fnc_ptr->body );
	
	dispose_string_object( name );
}


int function_remove( char **src_p )
{
	char	*name;
	int		result	= ERROR;

#if 0
	while ( (name	= get_next_token( src_p, NORMAL )) )
	{
		if ( already_defined( name, 0 ) )
		{
			result	= remove_func( name );
			dispose_string_object( name );
		}
	}
	
	return ( NO_ERROR );
#else

	if ( (name	= get_next_token( src_p, NORMAL )) )
	{
		if ( already_defined( name, 0 ) )
		{
			result	= remove_func( name );
			dispose_string_object( name );

			return ( result );
		}
	}
	
	return ( NO_ERROR );

#endif
}


int function_isdefined( char **src_p )
{
	char		*name;
	
	if ( !(name		= get_next_token( src_p, NORMAL )) )
		return ( ERROR );

	if ( already_defined( name, 0 ) )
		push_i( TRUE );
	else
		push_i( FALSE );
	
	dispose_string_object( name );
	
	return ( NO_ERROR );
}


void function_remove_by_package_name( char *name )
{
	function	*fnc_ptr;
	function	*next;
	
	fnc_ptr		= g_function_list_base.next;

	while ( fnc_ptr )
	{
		next		= fnc_ptr->next;

		if ( !strcmp( fnc_ptr->package_name, name ) )
			remove_func( fnc_ptr->name );
			
		fnc_ptr		= next;
	}
}




static int remove_func( char *name )
{
	function	*prev_fp;
	function	*targ_fp;
	function	*next_fp;

	if ( !name )
	{
		cprintf( ERROR, CONT, "need argument to remove function\n" );
		return ( ERROR );
	}

	if ( NULL == (prev_fp	= already_defined( name, -1 )) )
	{
		cprintf( ERROR, CONT, "function is not found to remove\n" );
		dispose_string_object( name );
		return ( ERROR );
	}
	
	targ_fp		= prev_fp->next;
	next_fp		= targ_fp->next;

	prev_fp->next	= next_fp;
	
	remove_function_from_key( targ_fp );

	targ_fp->next	= NULL;
	
	cprintf( WARN, CONT, "remove function \"%s\".\n", targ_fp->name );

	*(targ_fp->name)		= '\0';
	
	dispose_string_object( targ_fp->package_name );
	targ_fp->package_name	= NULL;

	free( targ_fp->body );
	targ_fp->body			= NULL;
	
	free( targ_fp );
	targ_fp					= NULL;

	return ( NO_ERROR );
}


static function *already_defined( char *str, int offset )
{
	function	*fnc_ptr;
	function	*prev_fnc_ptr;
	
	if ( *str == '@' )
		return ( &g_function_list_base );
//		return ( &g_temporal_function );

	fnc_ptr			= &g_function_list_base;
	prev_fnc_ptr	= fnc_ptr;
	
	while ( fnc_ptr )
	{
		if ( !strcmp( fnc_ptr->name, str ) )
		{
			if ( offset )
				return ( prev_fnc_ptr );
			else
				return ( fnc_ptr );
		}
		
		prev_fnc_ptr	= fnc_ptr;
		fnc_ptr			= fnc_ptr->next;		
	}
	
	return ( fnc_ptr );
}


static function *end_of_function_list( void )
{
	function	*fnc_ptr;

	fnc_ptr		= &g_function_list_base;

	while ( fnc_ptr->next )
		fnc_ptr		= fnc_ptr->next;

	return ( fnc_ptr );
}


static void forget( function *fnc_ptr )
{
	if ( NULL != fnc_ptr->next )
		forget( fnc_ptr->next );
	
	remove_function_from_key( fnc_ptr );

	fnc_ptr->next	= NULL;
	
	cprintf( WARN, CONT, "forget function \"%s\".\n", fnc_ptr->name );

	*(fnc_ptr->name)		= '\0';
	
	free( fnc_ptr->body );
	fnc_ptr->body		= NULL;
	
	free( fnc_ptr );
	fnc_ptr				= NULL;
}


void function_list( FILE *mode )
{
	//	the parameter "mode" is used to list-up the functions
	//
	//	if the "mode" holds a file pointer, the function may be called for saving function list into file.
	//	if the "mode" is NULL, the list will be shown to the screen
	
	function	*fnc_ptr;
//	int			i	= 0;

	fnc_ptr		= g_function_list_base.next;

	while ( fnc_ptr )
	{
		if ( mode == 0 )
		{

#if 0 //	2008-Feb-01  modified 
//			cprintf( NORM, CONT, "function[%2d] : %-16s %s; (%s)\n", i++, fnc_ptr->name, fnc_ptr->body, fnc_ptr->package_name );
			cprintf( NORM, CONT, ":" );
			cprintf( BOLD, CONT, "%-16s", fnc_ptr->name );
			cprintf( NORM, CONT, "%s; (%s)\n", fnc_ptr->body, fnc_ptr->package_name );

#else
			push_s( fnc_ptr->name );
#endif


		}
		else
		{
			if ( !strcmp( fnc_ptr->package_name, "" ) )
				fprintf( mode, "\t:%-20s\t%s;\n", fnc_ptr->name, fnc_ptr->body );
		}
		fnc_ptr		= fnc_ptr->next;
	}
}


void show_function( char **src_p )
{
	function	*fnc_ptr;
	char		*name		= NULL;
	char		*s			= NULL;
	char		*t;

	if ( (name	= get_next_token( src_p, NORMAL )) )
	{
		if ( (fnc_ptr	= already_defined( name, 0 )) )
		{
			if ( (t	= (char *)alloca( strlen( fnc_ptr->name ) + strlen( fnc_ptr->body ) + 5 )) ) 
			{
				sprintf( t, ":%s %s;", fnc_ptr->name, fnc_ptr->body );

				if ( (s		= make_string_object( t, -1 )) )
				{
					key_set_pre_given_str( s );
				}
			}
		}
		else
		{
			cprintf( ERROR, CONT, "no function \"%s\" found\n", name );
		}
	}
	
	if ( name )
		dispose_string_object( name );
}


int functions_compile( char **src_p )
{
#if 0
	function		*fnc_ptr;
	string_object	s;
	string_object	*sp;
	char			*name;

	name	= get_next_token( src_p, NORMAL );

	if ( !name )
	{
		cprintf( ERROR, CONT, "no function name given to compile\n" );
		return ( ERROR );
	}
	
	if ( !(fnc_ptr	= already_defined( name, 0 )) )
	{
		cprintf( NORM, CONT, "function \"%s\" not found to compile.\n", name );
		return ( ERROR );
	}

	cprintf( NORM, CONT, "compile function \"%s\".\n", name );
	
	if ( !(fnc_ptr->body) )
	{
		cprintf( NORM, CONT, "this function has no body.\n" );
		return ( ERROR );
	}

	cprintf( NORM, CONT, "compiling function body: \"%s\".\n", fnc_ptr->body );

	compile( fnc_ptr->body );

	dispose_string_object( name );

#endif
	return ( NO_ERROR );
}






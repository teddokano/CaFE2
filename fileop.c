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
#include	<unistd.h>

#include	"fileop.h"
#include	"functions.h"
#include	"key.h"
#include	"ui.h"
#include	"stack.h"
#include	"controls.h"
#include	"string.h"


char	g_default_string_for_loading[]		= "";
char	*g_loading_file_name_ptr			= g_default_string_for_loading;


void			set_loading_file_name( char *name );
string_object	convert_home_path( string_object s );
void			save_stack( stack *trg_p, FILE *fp );

//	functions are stored in a special stack. This stack is implemented as link list. 





#include <glob.h>


int fileop( char **src_p, int mode )
{
	char	*name;
	int		result	= ERROR;
	int		i;

	name	= pop_s();
	
	if ( !name )
	{
		if ( mode == FILEOP_CHDIR )
		{
			name	= make_string_object( "~", -1 );
		}
		else
		{
			cprintf( ERROR, CONT, "no file name given\n" );
			return ( ERROR );
		}
	}
	
	name	= convert_home_path( name );

	if ( mode == FILEOP_USE )
	{
		glob_t	g;

		glob( name, GLOB_TILDE | GLOB_MARK | GLOB_BRACE, NULL, &g );

		if ( g.gl_pathc == 0 )
		{
			cprintf( ERROR, CONT, "??? file \"%s\"\n", name );
			return ( ERROR );
		}
		else if ( (g.gl_pathc != 1) && (mode != FILEOP_USE) )
		{
			cprintf( ERROR, CONT, "can not write into multiple files \"%s\"\n", name );
			return ( ERROR );
		}
	
		for ( i  = 0; i < g.gl_pathc; i++ )
		{
			result	= fileop_use_by_name( (g.gl_pathv)[ i ] );
		}
		
		globfree( &g );
	}
	else if ( mode == FILEOP_PUT )
		result  = fileop_put_by_name( name );
	else if ( mode == FILEOP_SAVE )
		result  = fileop_save_by_name( name );
	else if ( mode == FILEOP_CHDIR )
	{
		chdir( name );
		result  = NO_ERROR;
	}

	else
		result	= ERROR;
	
	dispose_string_object( name );
	
	return ( result );
}


#include	<sys/stat.h>

int fileop_use_by_name( char *file_name )
{
	FILE			*fp;
	char			name[ MAX_TOKEN_LENGTH ];
	string_object	src;
	string_object	s;
	char			comment_state	= FALSE;
	unsigned long 	file_size;
	unsigned long 	i;
	struct stat		stat_buffer;
	
	// file open an prepare to load

	strcpy( name, file_name );
	file_handle_path( name, MAX_TOKEN_LENGTH );

	if ( NULL == (fp	= fopen( name, "r" )) )
	{
		cprintf( ERROR, CONT, "fail to use file \"%s\"\n", name );
		return ( ERROR );
	}

	set_loading_file_name( name );

	//	file loading

	stat( name, &stat_buffer );
	file_size	= (unsigned long)(stat_buffer.st_size);

	if ( NULL == (src	= make_string_object( "", file_size + 1 )) )
	{
		cprintf( ERROR, CONT, "file is too big, can't read to memory : file size = %lu (%s)\n", file_size, name );
		return ( ERROR );
	}

	fread( src, file_size, 1, fp );
	
	for ( i = 0, s = src; i < file_size; i++, s++ )
	{
		switch ( *s )
		{
			case '#' : 
				comment_state	= TRUE;
				break;
			case '\n' : 
				comment_state	= FALSE;
				*s	= LINEFEED_REPLACED;
				break;
			default : 
				break;
		}

		if ( comment_state )
			*s	= ' ';
	}

	file_line_count( LINE_COUNT_RESET );

	if ( evaluate( src ) )
	{
		cprintf( BOLD, CONT, "error found in file : \"%s\"\n", name );
		cprintf( BOLD, CONT, "at line #%d\n", file_line_count( LINE_COUNT_STAY ) );
	}
	
	dispose_string_object( src );

	set_loading_file_name( NULL );
	
	fclose( fp );
	
	return ( NO_ERROR );
}


int file_line_count( int mode )
{
	static int	line_count;

	if ( mode == LINE_COUNT_RESET )
		line_count	= 0;
	else if ( mode == LINE_COUNT_INCREMENT )
		line_count++;
	else
		;
	
	return ( line_count );
}


void file_set_package_name( void )
{
	char	*s;

	if ( !(s	= pop_s()) )
	{
		cprintf( ERROR, CONT, "no package name specified\n" );
		return;
	}
	
	set_loading_file_name( s );

	if ( s )
		dispose_string_object( s );
}

void set_loading_file_name( char *name )
{
	if ( g_loading_file_name_ptr != g_default_string_for_loading )
		dispose_string_object( g_loading_file_name_ptr );
	
	if ( !name )
		g_loading_file_name_ptr		= g_default_string_for_loading;
	else
		g_loading_file_name_ptr		= make_string_object( name, -1 );

	controls_set_package( g_loading_file_name_ptr );
}


char *file_get_loading_file_name( void )
{
	return ( g_loading_file_name_ptr );
}



#define		HISTORIES	100

int fileop_put_by_name( char *file_name )
{
	FILE	*fp;
	char	name[ MAX_TOKEN_LENGTH ];
	char	s[80];
	char	**strp;
	char	*quoted;
	int		i;
	
	strcpy( name, file_name );

	file_handle_path( name, MAX_TOKEN_LENGTH );
	
	if ( NULL == (fp	= fopen( name, "w" )) )
	{
		cprintf( ERROR, CONT, "fail to put file \"%s\"\n", name );
		return ( ERROR );
	}

	fprintf( fp, "###\n" );
	fprintf( fp, "### CaFE file made by \"put\" command.\n" );
	fprintf( fp, "###    --- %s ---\n", version_string( s ) );

	fprintf( fp, "###\n" );
	fprintf( fp, "\n" );

	time_now( s );
	fprintf( fp, "###\n### ### saved : %s\n###\n", s );

	fprintf( fp, "\n\n### functions\n\n" );
	
	if ( !(strcmp( file_name, PREFERENCE_FILE3 )) )
		fprintf( fp, "\"\" package\n\n" );

	function_list( fp );

	fprintf( fp, "\n\n### history section\n\n" );

	if ( NULL == (strp	=( char **)malloc( sizeof( char * ) * g_cafe_mode.n_history.value )) )
	{
		fclose( fp );
		cprintf( ERROR, CONT, "fail to put file (@malloc)\"%s\"\n", name );
		return ( ERROR );
	}

	history_strings( strp, g_cafe_mode.n_history.value );
	
	for ( i = (g_cafe_mode.n_history.value - 1); i >= 0 ; --i )
	{
		if ( !strp[ i ] )
			continue;
		
		if ( strstr( strp[ i ], "qq" ) )
			continue;
		
		if ( strstr( strp[ i ], "quit" ) )
			continue;
		
		quoted	= string_quote( strp[ i ], '\"' );
		fprintf( fp, "%s\thistory\n", quoted );
		dispose_string_object( quoted );

	}
	
	free( strp );

	fclose( fp );

	return ( NO_ERROR );
}


int fileop_save_by_name( char *file_name )
{
	FILE	*fp;
	char	name[ MAX_TOKEN_LENGTH ];
	char	s[80];
	
	strcpy( name, file_name );

	file_handle_path( name, MAX_TOKEN_LENGTH );
	
	if ( NULL == (fp	= fopen( name, "w" )) )
	{
		cprintf( ERROR, CONT, "fail to save file \"%s\"\n", name );
		return ( ERROR );
	}

	fprintf( fp, "###\n" );
	fprintf( fp, "### CaFE file made by \"save\" command.\n" );
	fprintf( fp, "###    --- %s ---\n", version_string( s ) );

	time_now( s );
	
	fprintf( fp, "###\n###    ### saved : %s\n###\n", s );
	fprintf( fp, "\n\n### user stack operation functions\n\n" );

	fprintf( fp, "0 >newlydefined_{\n" );
	fprintf( fp, "0 >newlydefined_k}\n\n" );
	fprintf( fp, "fisdef {    if z :@ :{ =new =target ; 1 >newlydefined_{ ; pop\n\n" );
	fprintf( fp, "fisdef k}   if z :@ :k} =parent =target ; 1 >newlydefined_k} ; pop\n\n\n" );
	
	fprintf( fp, "### content of stack\n\n" );
	
	save_stack( NULL, fp );
	
	fprintf( fp, "\n\n### content of stack end\n\n" );

	fprintf( fp, "<newlydefined_{  if t :@ forget { ;  pop\n" );
	fprintf( fp, "<newlydefined_k} if t :@ forget k} ; pop\n" );
	
	fclose( fp );

	return ( NO_ERROR );
}


void save_stack( stack *trg_p, FILE *fp )
{
	stack			*stack_p;
	stack_item		*si_p;
	string_object	s;

	if ( trg_p )
		fprintf( fp, "\n{ " );

	if ( NULL == (stack_p	= make_stack()) )
	{
		cprintf( ERROR, CONT, "unrecoverable @ show_stack (1)\n" );
		exit ( 1 );
	}


	stack_rcopy( stack_p, trg_p );
	
	si_p		= stack_p->stack_top_p;

	while ( si_p )
	{
		if ( si_p->type == STACK )
		{
			save_stack( *((stack **)si_p->item_p), fp );
		}
		else
		{
			s	= ui_stack_item_to_string( si_p, -1, STRING_WITH_QUOTE );
			fprintf( fp, "%s ", s );
			dispose_string_object( s );
		}
		si_p	= si_p->next;
	}
	dispose_stack( stack_p );

	if ( trg_p )
		fprintf( fp, "k}\n" );
}


void file_handle_path( char *path, int length )
{
	char	tmp[ MAX_TOKEN_LENGTH ]	= "";

	strcpy( tmp, path );

	if ( *path == '~' )
	{
		strcpy( path, getenv( "HOME" ) );
		
		if ( (length - 1) <= strlen( path ) + (strlen( tmp ) - 1) )
		{
			cprintf( ERROR, CONT, "converting file path @ handle_path\n" );
			return;
		}
		else
		{
			strcat( path, tmp + 1 );
		}
	}
}


string_object convert_home_path( string_object s )
{
	string_object	t;
	string_object	home;
	string_object	pos;
	int				len;
	
	if ( !strchr( s, '~' ) )
		return ( s );

	if ( !(home	= getenv( "HOME" )) )
		return ( s );

	len		= strlen( home );
	
	t	= make_string_object( s , strlen( s ) + len - 1 );

	pos	= strchr( t, '~' );
	
	memmove( pos + len - 1, pos, strlen( pos ) );
	strncpy( pos, home, len );

	dispose_string_object( s );
	return ( t );

/*

XXXX~ZZZZZZZZZZ0  X4 ~1 Z10
YYYYYYY0          Y7         

XXXX~ZZZZZZZZZZ------0
XXXX------~ZZZZZZZZZZ0

*/
}



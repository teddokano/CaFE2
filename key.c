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

#include	<ncurses.h>
#include	<stdlib.h>
#include	<string.h>

#include	"key.h"
#include	"ui.h"
#include	"fileop.h"
#include	"command_table.h"


enum	{
			HSTRY_RESET,
			HSTRY_UP,
			HSTRY_DOWN
		};

typedef struct _key_history		key_history;

struct	_key_history	{
							char		src[ MAX_SRC_LINE_LENGTH ];
							key_history	*next;
							key_history	*prev;
						};

key_history		*g_history_entry		= NULL;

char	*g_pre_given_str	= NULL;

static void		get_history_string( int direction, char *s, char *c );
static int		candidate( char *src, int *num_p );
static char		**load_file_names( char *string );
static void		dispose_file_names( char **list );

#define	TEST22

char *key_input( char *prompt )
{
	char			src[ MAX_SRC_LINE_LENGTH ];
	char			tmp_str[ MAX_SRC_LINE_LENGTH ];
	char			curr_buff[ MAX_SRC_LINE_LENGTH ];
	char			*ret;
	int				ch;
	int				length;
	int				curs;
	int				done		= FALSE;
	int				prev_len, i;
	int				by, bx;
	int				bby=0, bbx=0;
	int				bbyp=1000;
	int				scrl_ofst	= 0;
	int				y, x;
//	int				py, px;
	int				ontime_display;
	
	ontime_display	= g_cafe_mode.ontime_display.value;
	g_cafe_mode.ontime_display.value	= FALSE;
	
	if ( g_pre_given_str )
	{
		length	= strlen( g_pre_given_str );

		if ( length < MAX_SRC_LINE_LENGTH )
		{
			strncpy( src, g_pre_given_str, length );
			*(src + length)	= '\0';
		}
		
		dispose_string_object( g_pre_given_str );
		g_pre_given_str		= NULL;
	}
	else
	{
		*src		= '\0';
		length		= 0;
	}

	curs	= length;
	
	get_history_string( HSTRY_RESET, NULL, NULL );
	
	getyx( stdscr, y, x );
	
	if ( x )
		cprintf( NORM, CONT, "\n" );


	getyx( stdscr, by, bx );
//	px	= bx;
//	py	= by;
	
	prev_len	= length;


	show_prompt();
	getyx( stdscr, y, x );

	scrl_ofst	= ((x - bx) + length) / COLS;

	cprintf( NORM, CONT, "%s", src );

	while ( !done )
	{
		ch	= getch();									/* If raw() had not been called we have to preess
														 * enter before it gets to the program			*/

//	cprintf( BOLD, CONT, "[%o<>%o %o^v%o] %o\n", KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, ch );
		switch ( ch )
		{
			case KEY_DOWN :
				get_history_string( HSTRY_DOWN, src, curr_buff );
				length		= strlen( src );
				curs		= length;
				break;
			case KEY_UP :
				get_history_string( HSTRY_UP,   src, curr_buff );
				length		= strlen( src );
				curs		= length;
				break;
			case KEY_LEFT :
				curs	= curs ? (curs - 1) : curs;
				break;
			case KEY_RIGHT :
				curs	= (curs < length) ? (curs + 1) : curs;
				break;
			case KEY_BACKSPACE : 
			case 0x7F :
			case 0x14A :
				if ( curs )
				{
					strcpy( tmp_str, src + curs );
					strcpy( src + curs - 1, tmp_str );
					length--;
					curs--;
				}
				break;
			case 0x09 :
				{
					int		num;
					
					length	= candidate( src, &num );
					curs	= length;

					if ( 1 < num )
					{
						getyx( stdscr, by, bx );
						bx		= 0;
						bbyp	= by;
					}
				}
				break;

#if 0 // this is requied if raw() is used instead of cbreak()
			case 0x03 : // for '^C'
				exit( 1 );
				break;
				
#endif

			case 0x1B : // for [Esc]
				break;
				
			case 0x0A :
				if ( *src )
					key_register_history( src );
				done	= TRUE;
				break;
			case EOF :
				if ( *src )
					key_register_history( src );

				g_cafe_mode.quit.value	= TRUE;
				done					= TRUE;

				fprintf( stderr, "\r\nerror : EOF detected.\r\n" );
				fprintf( stderr, "CaFE is launched in interactive mode.\r\n" );
				fprintf( stderr, "use \"-!\" option to launch CaFE in non-interactive mode.\r\n" );
				cprintf( ERROR, ABORT, "now in interactive mode\n" );
				break;
				
#ifdef TEST_FILLING_STR
			case 'z' :

				for ( i = 0; i < MAX_SRC_LINE_LENGTH - 1; i++ )
					src[ i ]	= 'z';
				src[ MAX_SRC_LINE_LENGTH - 1 ]	= '\0';
				length		= strlen( src );
				curs		= length;
				break;
#endif

#if 0
			case 'k' :
				cprintf( NORM, CONT, "[%d]", COLS );
				cprintf( NORM, CONT, "[%d]", LINES );
				cprintf( NORM, CONT, "[%d]", scrl_ofst );
				getyx( stdscr, y, x );
				cprintf( NORM, CONT, "[%d]", y );
//	cprintf( NORM, CONT, "[%x]", getch() );
	
				break;
#endif

			default : 
				if ( length < MAX_SRC_LINE_LENGTH - 2 )
				{
					strcpy( tmp_str, src + curs );
					strcpy( src + curs + 1, tmp_str );
					*(src + curs)	= ch;
					length++;
					curs++;
				}
				break;
		}

		//	print full string at once

		getyx( stdscr, y, x );		

		bby		= by - (((LINES - 1) <= y + scrl_ofst) ? scrl_ofst : 0);
		bbx		= bx;

		bby		= (bbyp < bby) ? bbyp : bby;
		bbyp	= bby;

		move( bby, bbx );
//		show_prompt();cprintf( NORM, CONT, "!%d!",bby );
		show_prompt();



		getyx( stdscr, y, x );

		scrl_ofst	= ((x - bx) + length) / (COLS);

		cprintf( NORM, CONT, "%s", src );

		//	print " " (space) if the string is shorter than previous

		for ( i = length; i < prev_len; i++ )
			cprintf( NORM, CONT, " " );
				
		//	update "prev_len" for next time

		prev_len	= length;

		//	print string to update the cursor position

		strcpy( tmp_str, src );
		*(tmp_str + curs)	= '\0';


		getyx( stdscr, y, x );
		
		bby		= by - (((LINES - 1) <= y + scrl_ofst) ? scrl_ofst : 0);
		bbx		= bx;

		bby		= (bbyp < bby) ? bbyp : bby;
		bbyp	= bby;

		move( bby, bbx );
//		show_prompt();cprintf( NORM, CONT, "@%d@",bby );
		show_prompt();
		cprintf( NORM, CONT, "%s", tmp_str );
		refresh();		/* Print it on to the real screen */
	}

	move( bby, bbx );
	show_prompt();
	cprintf( NORM, CONT, "%s\n", src );

	g_cafe_mode.error_detected.value	= FALSE;
	g_cafe_mode.ontime_display.value	= ontime_display;

	ret		= make_string_object( src, -1 );

	return ( ret );
}


static void get_history_string( int direction, char *s, char *c )
{
	static key_history	*hp		= NULL;

	if ( direction == HSTRY_RESET )
	{
		hp		= NULL;
		return;
	}

	if ( !g_history_entry )
		return;
		
	if ( direction == HSTRY_UP )
	{
		if ( !hp )
		{
			hp	= g_history_entry;
			strcpy( c, s );
		}
		else if ( hp->prev )
			hp		= hp->prev;
	}
	else
	{
		if ( !hp )
			return;
	
		hp		= hp->next;
	}
	
	if ( hp )
		strcpy( s, hp->src );
	else
		strcpy( s, c );
}


int key_register_history( char *src )
{
	key_history		*history_ptr;
	
	if ( NULL == (history_ptr	= (key_history *)malloc( sizeof( key_history ) )) )
		return ( ERROR );
	
	strcpy( history_ptr->src, src );
	history_ptr->prev		= g_history_entry;
	history_ptr->next		= NULL;
	
	if ( g_history_entry )
		g_history_entry->next	= history_ptr;

	g_history_entry			= history_ptr;

	return ( NO_ERROR );
}


void history_strings( char **strp, int n )
{
	key_history		*history_ptr;
	int				i;

	history_ptr		= g_history_entry;


	for ( i = 0; i < n; i++ )
	{
		if ( !history_ptr )
		{
			strp[ i ]	= NULL;
		}
		else
		{
			strp[ i ]	= history_ptr->src;
			history_ptr	= history_ptr->prev;
		}
	}
}


static int candidate( char *src, int *num_p )
{
	char	key[ MAX_SRC_LINE_LENGTH ];
	char	*pos;
	char	**list			= NULL;		//	surpressing warning
	int		count;
	int		quote			= FALSE;
	int		length_src;
	int		length_key;
	
	if ( NULL == (pos	= strrchr( src, ' ' )) )
		pos		= src;
	else
		++pos;
	
	quote	= (*pos == '"') ? TRUE : FALSE;
	
	if ( quote )
	{
		file_handle_path( pos + 1, MAX_SRC_LINE_LENGTH );

		if ( quote )
			list	= load_file_names( pos );

		count	= 0;	
		while ( list[ count ] )
			add_keywd_to_key( list[ count++ ] );
	}

	strcpy( key, pos );

	*num_p	= find_n_match_candidate( key );

	length_src	= strlen( src );
	length_key	= strlen( key );

	if ( quote )
	{
		count	= 0;	
		while ( list[ count ] )
			remove_keywd_from_key( list[ count++ ] );

		dispose_file_names( list );
	}
	
	
	if ( (length_src + length_key) < (MAX_SRC_LINE_LENGTH - 1) )
	{
		*pos	= '\0';
			
		strcat( src, key );
		return ( strlen( src ) );
	}
	
	return ( length_src );
}


static char **load_file_names( char *string )
{
	char	command_str0[ MAX_SRC_LINE_LENGTH ];
	char	command_str1[ MAX_SRC_LINE_LENGTH ];
	char	tmp[ MAX_SRC_LINE_LENGTH ];
	char	tmp2[ MAX_SRC_LINE_LENGTH + 2 ];
	FILE	*fp;
	char	**list	= NULL;	//	surpressing warning
	int		items;
	int		count	= 0;
	char	*pos;

#if 0
	sprintf( command_str0, "find %s* -print -maxdepth 0 2> /dev/null | wc -l ", string + 1 );
	sprintf( command_str1, "find %s* -print -maxdepth 0 2> /dev/null", string + 1);
#else
	sprintf( command_str0, "ls -1adp %s*  2> /dev/null | wc -l ", string + 1 );
	sprintf( command_str1, "ls -1adp %s*  2> /dev/null", string + 1);
#endif

	if ( NULL == (fp	= popen( command_str0, "r" )) )
	{
		cprintf( ERROR, CONT, "popen @ load_file_names" );
		exit( 1 );
	}
	
	fgets( tmp, MAX_SRC_LINE_LENGTH, fp );
	items	= strtol( tmp, NULL, 0 );

	pclose( fp );	

	if ( NULL == (list	= (char **)malloc( sizeof( char * ) * items + 1 )) )
	{
		cprintf( ERROR, CONT, "malloc @ load_file_names" );
		exit( 1 );
	}
	
	if ( !items )
	{
		list[ 0 ]	= NULL;
		return ( list );
	}

//	cprintf( BOLD, CONT, "items = %d ... %%%%%% %s\n", items, command_str1 );
	
	if ( NULL == (fp	= popen( command_str1, "r" )) )
	{
		cprintf( ERROR, CONT, "popen @ load_file_names" );
		exit( 1 );
	}
	
	while ( 1 )
	{
		fgets( tmp, MAX_SRC_LINE_LENGTH, fp );

		if ( feof( fp ) )
			break;
		
		pos		= strchr( tmp, '\n' );
		
		if ( pos )
			*pos	= '\0';
			
		strcpy( tmp2, "\"" );
		strcat( tmp2, tmp );
		
		if ( *(tmp2 + strlen(tmp2) - 1) != '/' )
			strcat( tmp2, "\"" );

		list[ count++ ]	= make_string_object( tmp2, -1 );
	}
	
	list[ count ]	= NULL;
	
	pclose( fp );
	
	return ( list );
}


static void dispose_file_names( char **list )
{
	int		count	= 0;

	while ( list[ count ] )
	{
		free( list[ count++ ] );
	}
	
	free( list );
}


void key_set_pre_given_str( char *s )
{
	g_pre_given_str		= s;
}





















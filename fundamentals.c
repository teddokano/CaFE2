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

#define		_GNU_SOURCE

#include	"cafe2.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<limits.h>
#include	<ncurses.h>
#include	<ctype.h>
#include	<time.h>
#include	<errno.h>


#include	"command_table.h"
#include	"stack.h"
#include	"key.h"
#include	"ui.h"
#include	"fileop.h"
#include	"debugger.h"
#include	"string.h"

#define		AUTO_PREF_FIRST		0
#define		USER_PREF_FIRST		1

int				g_recursive_level		= 0;
char			**g_mode_label_list;
name_chain		*g_load_file_list_p		= NULL;
name_chain		*g_prof_list_p			= NULL;

int				token_type( string_object s );
static char		*handle_args( int argc, char **argv );

void		initialize( void );
void		intial_file_loads( void );
int			check_file_loading_order( void );
int			operation( char **src_p );
double		a_to_f( char *s );
char		**load_mode_label( void );

static void set_interrupt( void );


int main( int argc, char **argv )
{
	string_object	string;
	string_object	proc_s;
	string_object	filename;
	
	proc_s	= handle_args( argc, argv );

	initialize();

	while ( (filename	= name_chain_pop( &g_load_file_list_p )) )
	{
		fileop_use_by_name( filename );
		dispose_string_object( filename );
	}

	do
	{
		if ( g_cafe_mode.interactive_mode.value )
		{
			//	this is for "interactive mode"
		
			string	= key_input( PROMPT_STR );
		}
		else
		{
			//	these are for "non-interactive mode"

			ui_suppress_cprintf( SUPPRESS_WARN );	//	eliminating "stack empty" message 

			if ( g_cafe_mode.get_stdin.value == 1 )
			{
				string_object	stdin_s;

				stdin_s	= make_string_object( "", MAX_SRC_LINE_LENGTH );
				fgets( stdin_s, MAX_SRC_LINE_LENGTH, stdin );
				
				if ( !(*stdin_s) )	//	at a new line, no '\n' means no next line
				{
					exit ( 0 );
				}
				else if ( *stdin_s == '\n' )
				{
					cprintf( NORM, CONT, "\n" );	//	do nothing for empty line
					continue;
				}
				string	= add_string( stdin_s, proc_s, WITH_SPACE );
			}
			else
			{
				string	= make_string_object( proc_s ? proc_s : "", -1 );
				g_cafe_mode.quit.value	= TRUE;
			}
		}

		g_recursive_level	= 0;
		
		if ( string )
		{
			evaluate( string );

			dispose_string_object( string );
			string	= NULL;
		}

//		if ( g_cafe_mode.interactive_mode.value )
//			stack_save();
		
		g_cafe_mode.function_exe_abort.value	= FALSE;
	}
	while ( !g_cafe_mode.quit.value );

	if ( g_cafe_mode.interactive_mode.value && g_cafe_mode.put_history.value )
	{
		fileop_put_by_name( PREFERENCE_FILE3 );
		endwin();		/* End curses mode      */
	}
	
	return ( 0 );
}


static string_object handle_args( int argc, char **argv )
{
	string_object	str			= NULL;
	int				arglen;
	int				enable_flag	= TRUE;
	int				i;

	g_cafe_mode.get_stdin.value		= 0;

	for ( i = 1; i < argc; i++ )
	{
		if ( *argv[ i ] == '-' )
		{
			arglen	= strlen( argv[ i ] );
			
			if ( (arglen == 2) || (arglen == 3) )
			{
				if ( arglen == 3 )
				{
					if ( isupper( *(argv[ i ] + 1) ) )
					{
						if ( '-' == *(argv[ i ] + 2) )
							enable_flag		= FALSE;
						else
							enable_flag		= TRUE;
					}
				}
			
				switch ( *(argv[ i ] + 1) )
				{
					case 'f' :
						++i;

						if ( i == argc )
							return ( NULL );
						else
							name_chain_push( &g_load_file_list_p, argv[ i ] );

						break;
					case 'L' :
						g_cafe_mode.use_preference.value			= enable_flag;
						g_cafe_mode.use_history.value				= enable_flag;
						g_cafe_mode.put_history.value				= enable_flag;
						break;
					case 'P' :
						g_cafe_mode.use_preference.value			= enable_flag;
						break;
					case 'H' :
						g_cafe_mode.use_history.value				= enable_flag;
						g_cafe_mode.put_history.value				= enable_flag;
						break;
					case 'S' :
						g_cafe_mode.put_history.value				= enable_flag;
						break;
					case 'B' :
						g_cafe_mode.interactive_mode.value			= enable_flag ? FALSE : TRUE;
						g_cafe_mode.get_stdin.value++;
						break;
					case '!' :
						g_cafe_mode.get_stdin.value					= 2;
						break;
					case 'h' : 
						g_cafe_mode.interactive_mode.value			= FALSE;
						g_cafe_mode.get_stdin.value					= 2;
						ui_information();
						exit( 0 );
						break;
					default  :
						g_cafe_mode.get_stdin.value++;
						str		= add_string( str, argv[ i ], WITH_SPACE );
						break;
				}
			}
			else
			{
				g_cafe_mode.get_stdin.value++;
				str		= add_string( str, argv[ i ], WITH_SPACE );
			}
		}
		else
		{
			g_cafe_mode.get_stdin.value++;
			str		= add_string( str, argv[ i ], WITH_SPACE );
		}
	}
	
#if 0	//	auto "." adding
	if ( 1 < g_cafe_mode.get_stdin.value )
		if ( *(str + strlen( str ) - 1) != '.' )
			str		= add_string( str, ".", WITH_SPACE );
#endif
	
	if ( g_cafe_mode.interactive_mode.value )
		g_cafe_mode.interactive_mode.value	= str ? FALSE : TRUE;

	return ( str );
}


string_object add_string( string_object base, string_object add, int with_space )
{
	string_object	r;
	int				base_len;
	int				add_len;
	
	if ( !base )
	{
		base		= make_string_object( "", -1 );
		with_space	= 0;
	}

	if ( !add )
		return ( base );

	base_len	= strlen( base );
	add_len		= strlen( add );
	
	r	= make_string_object( base, base_len + add_len + with_space );
	
	strncpy( r + base_len,              " ", with_space       );
	strncpy( r + base_len + with_space, add, add_len );

	dispose_string_object( base );
	
	return ( r );
}


string_object make_string_object( char *string, int length )
{
	string_object	new;
	
	length	= (length < 0) ? strlen( string ) : length;
	
	if ( NULL == (new	= (string_object)malloc( sizeof( char ) * length + 1 )) )
		return ( NULL );
	
	strncpy( new, string, length );
	*(new + length)		= '\0';

	return ( new );
}


void dispose_string_object( string_object string )
{
	free( string );
}


void initialize( void )
{
	if ( g_cafe_mode.interactive_mode.value )
	{
		initscr();					/* Start curses mode							*/		

		set_interrupt();


		raw();						/* Line buffering disabled						*/
//		cbreak();					/* "cbreak()" is used instead of raw() to get interrupt */
#if 0
		//		raw();						/* Line buffering disabled						*/
		cbreak();					/* "cbreak()" is used instead of raw() to get interrupt */
#endif
		keypad( stdscr, TRUE );		/* We get F1, F2 etc..							*/
		noecho();					/* Don't echo() while we do getch				*/
		scrollok(stdscr, TRUE);	
		ui_buildinfo();
		ui_suppress_cprintf( SUPPRESS_WARN );
	}
	else
	{
		ui_suppress_cprintf( SUPPRESS_ALL );
	}

	//	initialize built-in command table

	initialize_keywords();

	g_mode_label_list	= load_mode_label();
	intial_file_loads();
	
	ui_suppress_cprintf( SUPPRESS_OFF );
	
	
//set_interrupt();while (1);
}


void intial_file_loads( void )
{
	int		loading_order;
	
	loading_order		= check_file_loading_order();
	
	if ( NO_ERROR != fileop_use_by_name( STDLIB_LOCATION ) )
		cprintf( BOLD, CONT, "no cafe-standard library file found.\n" );
	
	if ( AUTO_PREF_FIRST == loading_order )
		if ( g_cafe_mode.use_history.value )
			fileop_use_by_name( PREFERENCE_FILE3 );
	
	if ( g_cafe_mode.use_preference.value )
	{
		if ( NO_ERROR == fileop_use_by_name( PREFERENCE_FILE1 ) )
			;
		else
			fileop_use_by_name( PREFERENCE_FILE2 );
	}
	
	if ( USER_PREF_FIRST == loading_order )
		if ( g_cafe_mode.use_history.value )
			fileop_use_by_name( PREFERENCE_FILE3 );
}


int check_file_loading_order( void )
{
	char	command_str[ MAX_SRC_LINE_LENGTH ];
	char	tmp[ MAX_SRC_LINE_LENGTH ];
	FILE	*fp;
	
	sprintf( command_str, "[ %s -nt %s ];echo $? ", PREFERENCE_FILE1, PREFERENCE_FILE3 );

	if ( NULL == (fp	= popen( command_str, "r" )) )
	{
		cprintf( ERROR, CONT, "popen @ load_file_names" );
		return ( USER_PREF_FIRST );
	}
	
	fgets( tmp, MAX_SRC_LINE_LENGTH, fp );
	pclose( fp );

	if ( *tmp == '0' )
		return ( AUTO_PREF_FIRST );

	return ( USER_PREF_FIRST );
}


//#define		PROFILER_ON

int evaluate( string_object string )
{
	int		return_code		= NO_ERROR;
#ifdef PROFILER_ON
	clock_t	start;
	char	prof_str[100];
	char	*evs;

	start	= clock();
	evs		= make_string_object( string, -1 );
#endif
	
	if ( g_cafe_mode.error_detected.value )
		return ( return_code );

	if ( g_cafe_mode.function_exe_abort.value )
		return ( return_code );

	if ( !strlen( string ) )
		return ( return_code );

	if ( 0 < g_cafe_mode.recursive_call_limit.value )
	{
//		cprintf( NORM, CONT, "%d / %d\n", g_recursive_level, g_cafe_mode.recursive_call_limit.value );
		
		if ( g_recursive_level++ == g_cafe_mode.recursive_call_limit.value )
		{
			cprintf( NORM, CONT, "function recursive call exceeded limit\n" );
//			recursive_level		= 0;
			return ( ERROR );
		}
	}

	return_code		= operation( &string );
	
	--g_recursive_level;
	
	debugger_set_level( g_recursive_level );

#ifdef PROFILER_ON
	snprintf( prof_str, 100, "%.3lf :: %s", 1e3 * (double)(clock() - start) / (double)CLOCKS_PER_SEC, evs );
	name_chain_push( &g_prof_list_p, prof_str );
#endif

	return ( return_code );
}


int operation( string_object *src_p )
{
	string_object	token_string;

	if ( g_cafe_mode.error_detected.value )
		return ( ERROR );

	if ( g_cafe_mode.force_function_return.value | g_cafe_mode.function_exe_abort.value )
		return ( NO_ERROR );

	while ( (token_string	= get_next_token( src_p, NORMAL )) )
	{
		int		t_type;
		
		t_type	= token_type( token_string );
	
		debugger( token_string );
	
//		switch ( (token_type( token_string )) )
		switch ( t_type )
		{
			case INTEGER :
			case INTEGER_HEX :
				{
					long	v;

					v	= strtol( token_string, NULL, 0 );
					
					if ( !errno ) 
					{
						push_item( &v, INTEGER, NULL );
						break;
					}
					else
					{
						if ( errno != ERANGE) 
						{
							cprintf( ERROR, CONT, "converting input string to integer" );
							break;
						}
						else
						{
							cprintf( BOLD, CONT, "warning : converted to float\n");
							//break;
						}
					}
				}
			case FLOAT :
				{
					double	d;
					
					d	= strtod( token_string, NULL );

					if ( !errno ) 
						push_item( &d, FLOAT, NULL );
					else
						cprintf( ERROR, CONT, "converting input string to float" );
				}
				break;
			case STRING :
				*(token_string + strlen( token_string ) - 1)	= '\0';
				push_item( token_string + 1, STRING, NULL );
				break;
			case COMMAND :
				if ( FALSE == try_token( token_string, src_p ) )
				{
					cprintf( ERROR, CONT, "unknown token : \'%s\'\n", token_string );
				}
				break;
			default : 
				break;
		}
		dispose_string_object( token_string );

		if ( g_cafe_mode.error_detected.value )
			return ( ERROR );

		if ( g_cafe_mode.force_function_return.value | g_cafe_mode.function_exe_abort.value )
			return ( NO_ERROR );
	}

	return ( 0 );
}


int compile( string_object s )
{
	string_object	token_string;
	string_object	*sp;

	sp	= &s;

	cprintf( BOLD, CONT, "C: in process...\n" );

	while ( (token_string	= get_next_token( sp, NORMAL )) )
	{
		int		t_type;
		
		t_type	= token_type( token_string );

		cprintf( BOLD, CONT, "token_string=[%s](%i)\n", token_string, t_type );
#if 1

		switch ( t_type )
		{
			case INTEGER :
			case INTEGER_HEX :
				{
					long long	v;
					int			i;
					
					v	= strtoll( token_string, NULL, 0 );
					
					if ( (llabs( v ) > (long long)ULONG_MAX) && (t_type == INTEGER_HEX) )
					{
						cprintf( BOLD, CONT, "warning : converted to float\n");
						/* FALLTHROUGH */
					}
					else if ( (llabs( v ) >> 31) && (t_type == INTEGER) )
					{
						cprintf( BOLD, CONT, "warning : converted to float\n");
						/* FALLTHROUGH */
					}
					else
					{
						i	= (int)v;

						cprintf( BOLD, CONT, "C: token evaluated as INTEGER = %ld\n", i );

						//push_item( &i, INTEGER, NULL );
	
					break;
					}
				}
			case FLOAT :
				{
					double	v;
					v	= strtod( token_string, NULL );
					
					cprintf( BOLD, CONT, "C: token evaluated as FLOAT = %lf\n", v );

					//push_item( &v, FLOAT, NULL );
				}
				break;
			case STRING :
				*(token_string + strlen( token_string ) - 1)	= '\0';
				
				cprintf( BOLD, CONT, "C: token evaluated as STRING = %s\n", token_string + 1 );

//				push_item( token_string + 1, STRING, NULL );
				break;
			case COMMAND :

				cprintf( BOLD, CONT, "C: token evaluated as TOKEN = %p\n", find_token( token_string, sp ) );
				break;
			default : 
				break;
		}
		dispose_string_object( token_string );
#endif
	}
	return ( 0 );
}


void error_message( string_object token_string )
{
//	file_show_line_number();
	cprintf( ERROR, CONT, "unknown token : \'%s\'\n", token_string );
}


enum	{
			PARSE_HEX,
			PARSE_NON_HEX,
		};
enum	{
			START,
			DIGIT,
			SIGN,
			EXPO,
			DOT,
		};


int	token_type( string_object s )
{
	char	c;
	char	prev_char	= START;
	int		count_e		= 0;
	int		count_dot	= 0;
	int		count_x		= 0;
	int		state		= PARSE_NON_HEX;

	if ( *s == '\"' )
		return ( STRING );

	if ( (!isdigit( *s )) && (strlen( s ) < 2) )
		return ( COMMAND );

	if ( ('0' == *s) && ('x' == tolower(*(s + 1))) )
	{
		state		 = PARSE_HEX;
		count_x		 = 1;
		s			+= 2;
	}
	
	while ( (c	= tolower( *s++ )) )
	{
		switch ( c )
		{
			case '0' :
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' :
				prev_char	= DIGIT;
				break;
				
			case 'a' :
			case 'b' :
			case 'c' :
			case 'd' :
			case 'f' :
			
				if ( PARSE_HEX != state )
					return ( COMMAND );

				prev_char	= DIGIT;			
				break;
				
			case '+' :
			case '-' :
			
				if ( PARSE_HEX == state )
					return ( COMMAND );

				if ( (prev_char != START) && (prev_char != EXPO) )
					return ( COMMAND );

				prev_char	= SIGN;			
				break;
				
			case '.' :
			
				if ( PARSE_HEX == state )
					return ( COMMAND );
			
				++count_dot;
				
				if ( (1 < count_dot) || count_e )
					return ( COMMAND );
					
					
				prev_char	= DOT;			
				break;
				
			case 'e' :
			
				if ( PARSE_HEX != state )
				{
					if ( !count_e )
						++count_e;
					else
						return ( COMMAND );
				}
				
				prev_char	= EXPO;			
				break;
				
			default : 
				return ( COMMAND );
				break;
				
		}
	}
	
	if ( count_dot || count_e )
		return ( FLOAT );

	return ( (state == PARSE_HEX) ? INTEGER_HEX : INTEGER );
}



enum	{
			IDLE,
			FIRST_CHAR,
			IN_WORD,
			QUOTE,
			TOKEN_REGISTER,
			SUBSTITUTION,
			MACRO,
		};
		
enum	{
			UNQUOTE,
			QUOTE_SINGLE,
			QUOTE_DOUBLE,
		};


string_object get_next_token( string_object *src_p, int flag )
{
	string_object	token_start		= NULL;		//	this initialization is to surpress warning
	string_object	token;
	string_object	src;
	int				state			= IDLE;
	int				token_length	= 0;
	int				in_process		= TRUE;
	int				quote			= UNQUOTE;
	int				fst_spcl_char	= FALSE;
	char			c;
	int				j;

	src		= *src_p;

	while ( in_process )
	{
		c		= *src;
		c		= isspace( c ) ? ' ' : c;
		
		if ( c == LINEFEED_REPLACED )
		{
			file_line_count( LINE_COUNT_INCREMENT );

			*src	= ' ';
			c		= ' ';
		}

		switch ( state )
		{
			case IDLE:

				switch ( c )
				{
					case '\0':
						return ( NULL );
					case ' ':
						src++;
						break;
					default : 
						token_length	= 1;
						state			= FIRST_CHAR;
						break;
				}
				break;
				
			case FIRST_CHAR:

				token_start		= src;

				switch ( c )
				{
					case '?':
					case '@':
						fst_spcl_char	= TRUE;
						src++;
						state	= IN_WORD;
						break;
					case '^':
						src++;
						if ( flag != FILE_PROCESSING )
							state	= SUBSTITUTION;
						else
							state	= IN_WORD;
						break;
					case '#':
						src++;
						if ( flag != FILE_PROCESSING && (*src == '\''))
							state	= MACRO;
						break;
					case '&':
						src++;
						state	= SUBSTITUTION;
						break;
					case '\'':
						src++;
						if ( flag != FILE_PROCESSING )
						{
							token_start		= src;
							token_length	= 0;
						}
						state	= QUOTE;
						quote	= QUOTE_SINGLE;
						break;
					case '\"':
						src++;
						state	= QUOTE;
						quote	= QUOTE_DOUBLE;
						break;
					case '\0':
					case '*':
					case '/':
					case ':':
					case ';':
					case '<':
					case '>':
					case '(':
					case ')':
					case '[':
					case ']':
						src++;
						state	= TOKEN_REGISTER;
						break;
					case '\\':
						src++;
						token_length++;
						if ( !(*src) )
						{
							state	= TOKEN_REGISTER;
							break;
						}
						/* FALLTHROUGH */
					default : 
						src++;
						state	= IN_WORD;
						break;
				}
				break;

			case IN_WORD:

				switch ( c )
				{
					case '+':
					case '-':
						if ( 'e' == tolower( *(src - 1) ) )
						{
							src++;
							token_length++;							
							break;
						}
						/* FALLTHROUGH */
					//case '\0':
					case '*':
					case '/':
					case ':':
					case ';':
					case '<':
					case '>':
					case '(':
					case ')':
					case '[':
					case ']':
//						if ( 0 )
							if ( fst_spcl_char )
						{
							src++;
							token_length++;
						}
						else
						{
							state	= TOKEN_REGISTER;
						}
						break;
					case '\0':
					case ' ':
						state	= TOKEN_REGISTER;
						break;
					case '\\':
						src++;
						token_length++;
						if ( !(*src) )
						{
							state	= TOKEN_REGISTER;
							break;
						}
						/* FALLTHROUGH */
					default : 
						src++;
						token_length++;
						break;
				}
				break;
				
			case QUOTE:

				switch ( c )
				{
					case '\\':		//	"\" must have higher priority than "'"
						src++;
						token_length++;

						if ( !(*src) )
						{
							state	= TOKEN_REGISTER;
						}
						else
						{
							src++;
							token_length++;
						}
						break;
					case '\'':
						src++;

						if ( quote == QUOTE_SINGLE )
						{
							if ( flag == FILE_PROCESSING )
								token_length++;

							state	= TOKEN_REGISTER;
							quote	= UNQUOTE;
						}
						else
						{
							token_length++;
						}
						
						break;
					case '\"':
						src++;
						token_length++;

						if ( quote == QUOTE_DOUBLE )
						{
							state	= TOKEN_REGISTER;
							quote	= UNQUOTE;
						}
						
						break;
					case '\0':
						token_length++;
						state	= TOKEN_REGISTER;
						break;
					default : 
						src++;
						token_length++;
						break;
				}
				break;

			case TOKEN_REGISTER:

				token	= make_string_object( token_start, token_length );

				if ( flag != FILE_PROCESSING )
				{
					j	= string_backslash_conversion( token );
					*(token + j)	= '\0';
				}

				if ( quote != UNQUOTE )
				{
					char	q[ 2 ]		= { 0, 0 };
					
					q[ 0 ]	= quote;
					token	= add_string( token, q, WITHOUT_SPACE );
				}

				*src_p	= src;

				return ( token );
			
			case SUBSTITUTION:
				
				if ( !(token	= pop_s()) )
					token	= make_string_object( "", -1 );
					
				*src_p	= src;

				return ( token );

			case MACRO:

				*src_p	= src;
				
				if ( !(token	= get_next_token( src_p, NORMAL )) )
					token	= make_string_object( "", -1 );

//cprintf( NORM, CONT, "macro processing: %s\n", token );
				
				evaluate( token );

				dispose_string_object( token );

				if ( !(token	= pop_s()) )
					token	= make_string_object( "", -1 );

//cprintf( NORM, CONT, "     --> \"%s)\"\n", token );
				
				return ( token );

			default : 
				break;
		}

	}
	
	return ( NULL );
}


#include	<time.h>


void time_now( char *s )
{
	time_t		now;
	struct tm	*date;
	
	time( &now );
	date	= localtime( &now );
//	strftime( s, 80, "%c", date );
	strftime( s, 80, "%T %A, %d-%B-%Y (week %V)", date );
}

char **load_mode_label( void )
{
	char			tmp2[ MAX_MODE_LABEL_LENGTH + 2 ];
	mode_item		*mip;
	char			**list;
	int				count	= 0;
	int				i;

	mip		= (mode_item *)(&g_cafe_mode);

	if ( NULL == (list	= (char **)malloc( sizeof( char * ) * (sizeof( g_cafe_mode ) / sizeof( mode_item )) + 1 )) )
	{
		cprintf( ERROR, CONT, "malloc @ load_file_names" );
		exit( 1 );
	}


	for ( i = 0; i < sizeof( g_cafe_mode ) / sizeof( mode_item ); i++ )
	{
		strcpy( tmp2, "\"" );
		strcat( tmp2, mip->name );
		strcat( tmp2, "\"" );
		list[ count++ ]	= make_string_object( tmp2, -1 );
		mip++;
	}

	list[ count ]	= NULL;
	
	return ( list );
}


int get_recursive_level( void )
{
	return ( g_recursive_level );
}


void name_chain_push( name_chain **ncpp, string_object s )
{
	name_chain	*new_nc;

	new_nc		= (name_chain *)malloc( sizeof( name_chain) );
	
	new_nc->name	= make_string_object( s, -1 );
	new_nc->prev	= *ncpp;
	*ncpp			= new_nc;
}


string_object name_chain_pop( name_chain **ncpp )
{
	name_chain		*nc_oldp;
	string_object	s;

	if ( !(*ncpp) )
		return ( NULL );

	nc_oldp		= *ncpp;
	*ncpp		= (*ncpp)->prev;
	
	s	= nc_oldp->name;	
	free( nc_oldp );
	
	return ( s );
}


#include	<signal.h>

static void interrupt_handler( int sig )
{
	int		c;
	int		x, y;
	
	refresh();
	
	getyx( stdscr, y, x );		

	do
	{
		cprintf( BOLD, CONT, "ABORT? Y/n/q/? : " );
		refresh();
		c	= getch();
		
		switch ( c ) 
		{
			case 'Y' : 
			case 'y' : 
				c	= 1;
				break;
			case '?' :
				cprintf( BOLD, CONT, "\n\n" );
				cprintf( BOLD, CONT, "  y : abort the calculation\n" );
				cprintf( BOLD, CONT, "  n : continue the calculation\n" );
				cprintf( BOLD, CONT, "  q : quit application\n" );
				cprintf( BOLD, CONT, "  ? : help\n" );
				refresh();
				/* FALLTHROUGH */
			case 'N' : 
			case 'n' : 
				c	= -1;
				break;
			case 'Q' : 
			case 'q' : 
				endwin();		/* End curses mode      */
				printf( "cafe : terminated by user.\n" );

				//cprintf( NORM, CONT, "\n" );
				exit( 2 );
				break;
		}
	}
	while ( !c );

	move( y, x );
	cprintf( BOLD, CONT, "                      " );
	move( y, x );
	refresh();

	if ( c > 0 )
		g_cafe_mode.function_exe_abort.value	= TRUE;
}

static void set_interrupt( void )
{
	signal( SIGINT, interrupt_handler );
}


void prof_dump( void )
{
	string_object	s;
	
	while ( (s	= name_chain_pop( &g_prof_list_p )) )
	{
		push_s( s );
		dispose_string_object( s );
	}
}




























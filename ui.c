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
#include	<math.h>
#include	<ctype.h>

#include	"ui.h"
#include	"stack.h"
#include	"string.h"

char	g_type_identifier[]		= TYPE_IDENTIFIER_LIST;

int				show_prompt_result( int bold );
string_object	show_value( int sp );
string_object format_analysis( string_object fmt );
int find_type( string_object s );



/* PRNSC */
void ui_user_print( string_object s )
{
	static int	in_postprocess	= FALSE;
	
	if ( g_cafe_mode.print_target.value )
		push_s( s );
	else
		cprintf( BOLD, NORM, "%s", s );
	
	if ( !in_postprocess )
	{
		if ( *(g_cafe_mode.post_print_screen.str) && !g_cafe_mode.print_target.value )
		{
			in_postprocess	= TRUE;
			evaluate( g_cafe_mode.post_print_screen.str );
			in_postprocess	= FALSE;
		}
		else if ( *(g_cafe_mode.post_print_stack.str) && g_cafe_mode.print_target.value )
		{
			in_postprocess	= TRUE;
			evaluate( g_cafe_mode.post_print_stack.str );
			in_postprocess	= FALSE;
		}
	}
}

/* PRNSC */
#if 1
void ui_format_print( void )
{
	string_object	format;
	string_object	format_base;
	string_object	sub_format;
	string_object	stringbuffer	= NULL;
	char			tmp[ MAX_SRC_LINE_LENGTH ];
	
	if ( !(format	= pop_s()) )
		return;
	
	format_base		= format;
	
	while ( (sub_format	= format_analysis( format )) )
	{
		switch ( find_type( sub_format ) )
		{
			case INTEGER : 
				snprintf( tmp, MAX_SRC_LINE_LENGTH, sub_format, pop_i() );
				break;
			case FLOAT : 
				snprintf( tmp, MAX_SRC_LINE_LENGTH, sub_format, pop() );
				break;
			case STRING : 
				{
					string_object	s;

					if ( !(s	= pop_s()) )
						make_string_object( "", -1 );

					snprintf( tmp, MAX_SRC_LINE_LENGTH, sub_format, s );
					
					dispose_string_object( s );
				}
				break;
			case TYPE_UNKNOWN : 
				break;
			case NOT_DEFINED : 
				snprintf( tmp, MAX_SRC_LINE_LENGTH, sub_format, "" );
				break;
			default : 
				break;
		}
		
		stringbuffer	= add_string( stringbuffer, tmp, WITHOUT_SPACE );

		format	+= strlen( sub_format );
		dispose_string_object( sub_format );
	}
	
	ui_user_print( stringbuffer );

	dispose_string_object( stringbuffer );
	dispose_string_object( format_base );
}
#else
void ui_format_print( void )
{
	string_object	format;
	string_object	format_base;
	string_object	sub_format;
	
	if ( !(format	= pop_s()) )
		return;
	
	format_base		= format;
	
	while ( (sub_format	= format_analysis( format )) )
	{
//cprintf( BOLD, CONT, "[%s] type=%d\n", sub_format , find_type( sub_format ));

		switch ( find_type( sub_format ) )
		{
			case INTEGER : 
//cprintf( BOLD, CONT, "INTEGER [%s] type=%d\n", sub_format , find_type( sub_format ));
				cprintf( BOLD, CONT, sub_format, pop_i() );
				break;
			case FLOAT : 
//cprintf( BOLD, CONT, "FLOAT   [%s] type=%d\n", sub_format , find_type( sub_format ));
				cprintf( BOLD, CONT, sub_format, pop() );
				break;
			case STRING : 
//cprintf( BOLD, CONT, "STRING  [%s] type=%d\n", sub_format , find_type( sub_format ));
				{
					string_object	s;

					if ( !(s	= pop_s()) )
						make_string_object( "", -1 );

					cprintf( BOLD, CONT, sub_format, s );
					
					dispose_string_object( s );
				}
				break;
			case TYPE_UNKNOWN : 
				break;
			case NOT_DEFINED : 
//cprintf( BOLD, CONT, "NOT_DEFINED [%s] type=%d\n", sub_format , find_type( sub_format ));
				cprintf( BOLD, CONT, sub_format );
				break;
			default : 
				break;
		}

		format	+= strlen( sub_format );
		dispose_string_object( sub_format );
	}
	
	dispose_string_object( format_base );
}
#endif

void show_top( void )
{
	string_object	s;

	if ( (s	= show_value( stack_pointer( NULL )  - 1 )) )
	{
		ui_user_print( s );
		dispose_string_object( s );
	}
}


string_object show_value( int sp )
{
	stack_item		*si_p;
	string_object	s;

	if ( NULL == (si_p	= peep_stack_item( NULL )) )
		return ( NULL );
	
	if ( (si_p->index < sp) || sp < 0 )
	{
		cprintf( BOLD, CONT, "   !!! stack pointer, out of range !!!\n" );
		return ( NULL );
	}
	
	while ( si_p )
	{
		if ( si_p->index == sp )
			break;
		
		si_p	= si_p->next;
	}
	
	s	= ui_stack_item_to_string( si_p, -1, STRING_WITHOUT_QUOTE );

	return ( s );
}


string_object ui_stack_item_to_string( stack_item *si_p, int length_limit, int mode )
{
	char			s[ MAX_SRC_LINE_LENGTH ];
	string_object	ns	= NULL;
	string_object	nns;				
	int				tmp_length;
	
	length_limit	= (length_limit < (MAX_SRC_LINE_LENGTH - 4)) ? length_limit : (MAX_SRC_LINE_LENGTH - 4);
	
	switch ( si_p->type )
	{
		case INTEGER :  
		
			snprintf( s, MAX_SRC_LINE_LENGTH, g_cafe_mode.format_int.str, *((long *)(si_p->item_p)) );
			break;
			
		case FLOAT : 
		
			snprintf( s, MAX_SRC_LINE_LENGTH, g_cafe_mode.format_float.str, *((double *)(si_p->item_p)) );
			break;
			
		case STACK : 
		
			{
				stack	*stackp;
				
				stackp	= *((stack **)(si_p->item_p));

//#define	DETAILED_STACK_INFO
#ifdef	DETAILED_STACK_INFO
				snprintf( s, MAX_SRC_LINE_LENGTH, "<0x%08X> =%d= (%d)", (unsigned)stackp, stackp->id, stackp->referenced );
#else
				snprintf( s, MAX_SRC_LINE_LENGTH, "=%d=", stackp->id );
#endif
			}
			break;
			
		case STRING : 
			ns		= make_string_object( (char *)(si_p->item_p), -1 );
			
			if ( mode )
			{
				nns		= string_quote( ns, '\"' );
				
				dispose_string_object( ns );
				ns	= nns;
			}
			break;
	}

	if ( ns )
	{
		if ( 0 <= length_limit )
		{
			nns		= make_string_object( ns, length_limit + 3 );
			strcpy( nns + length_limit, "..." );
			
			dispose_string_object( ns );
			ns	= nns;
		}
		
		return ( ns );
	}
	else
	{
		tmp_length	= strlen( s );

		if ( (0 <= length_limit) && (length_limit < tmp_length) && (tmp_length < (MAX_SRC_LINE_LENGTH - 4)) )
		{
			*(s + length_limit)		= '\0';
			strcat( s, "..." );
		
			length_limit	+= 3;
		}

		return ( make_string_object( s, length_limit ) );
	}
	
	
}


void show_stack( FILE *fp )
{
	stack			*stack_p;
	stack_item		*si_p;
	int				top_index;
	string_object	s;

	if ( NULL == peep_stack_item( NULL ) )
	{
		if ( !fp )
			cprintf( BOLD, CONT, "stack empty\n" );

		return;
	}
	
	if ( NULL == (stack_p	= make_stack()) )
	{
		cprintf( ERROR, CONT, "unrecoverable @ show_stack (1)\n" );
		exit ( 1 );
	}
	
	stack_rcopy( stack_p, NULL );
	
	si_p		= stack_p->stack_top_p;
	top_index	= si_p->index;


	while ( si_p )
	{
		if ( g_cafe_mode.function_exe_abort.value )
			break;

		if ( !fp )
			cprintf( NORM, CONT, "sp[%3d] (%c)    ", top_index - si_p->index, g_type_identifier[ si_p->type ] );

		s	= ui_stack_item_to_string( si_p, -1, STRING_WITH_QUOTE );

		if ( !fp )
			cprintf( BOLD, CONT, "%s\n", s );
		else
			fprintf( fp, "%s\n", s );
		
		dispose_string_object( s );
		
		si_p	= si_p->next;
	}
	dispose_stack( stack_p );
}


#include		<stdarg.h>


void cprintf( int bold, int mode, char *format, ... )
{
	char		str[ MAX_SRC_LINE_LENGTH ];
	va_list		args;

	if ( g_cafe_mode.messagelevel.value <= bold )
		return;
		
	if ( ERROR == bold )
	{
		string_object	tmp;
	
		if ( NULL == (tmp	= (string_object)alloca( strlen( format ) + 10 )) )
		{
		}

		{
			strcpy( tmp, "error : " );
			strcat( tmp, format );
			format	= tmp;				//  overwriting pointer : noproblem it is a local var.
		}
		
		bold	= BOLD;
		g_cafe_mode.error_detected.value	= TRUE;
	}
		
	va_start( args, format );
	
	vsnprintf( str, MAX_SRC_LINE_LENGTH, format, args );
	
	va_end( args );

	if ( g_cafe_mode.interactive_mode.value )
	{
		if ( bold )
			attron( A_BOLD );
		
		printw( "%s", str );
		attroff( A_BOLD );

		if ( g_cafe_mode.ontime_display.value )
			refresh();
	}
	else
	{
		fprintf( g_cafe_mode.error_detected.value ? stderr : stdout, "%s", str );
		fflush( NULL );
	}
	
	if ( ABORT == mode )
		exit ( 1 );
}



void ui_suppress_cprintf( int level )
{
	g_cafe_mode.messagelevel.value	= level;
}


void show_prompt( void )
{
	char			*form;
	stack_item		*si_p;
	int				bold	= FALSE;
	char			c;
	
	form	= g_cafe_mode.prompt.str;
		
	while ( (c	= *form++) )
	{
		switch ( c )
		{
			case '%' :
				{
					switch ( *form++ )
					{
						case 'b' :
							bold	= BOLD;
							break;
						case 'n' :
							bold	= NORM;
							break;
						case 'r' :
							show_prompt_result( bold );
							break;
						case 'p' :
							si_p	= peep_stack_item( NULL );
							if ( si_p )
								cprintf( bold, CONT, "%d", si_p->index );
							else
								cprintf( bold, CONT, "" );
							break;
						case 'P' :
							si_p	= peep_stack_item( NULL );
							if ( si_p )
								cprintf( bold, CONT, "%d", si_p->index );
							else
								cprintf( bold, CONT, "e" );
							break;
						case 's' :
								cprintf( bold, CONT, "%d", stack_get_current_stack_id() );
							break;
						case 'S' :
								cprintf( bold, CONT, "%d", stack_get_current_stack_id() );
							break;
						case '%' :
							cprintf( bold, CONT, "%%" );
							break;
						default : 
							cprintf( bold, CONT, "%%?" );
							break;
					}
				}
				break;
			default : 
				cprintf( bold, CONT, "%c", c );
				break;
		}
	}
}


int show_prompt_result( int bold )
{
	stack_item		*si_p;
	string_object	s;

	if ( g_cafe_mode.error_detected.value )
		s	= make_string_object( "error", -1 );
		
	else if ( NULL == (si_p	= peep_stack_item( NULL )) )
		s	= make_string_object( "empty", -1 );
		
	else
		s	= ui_stack_item_to_string( si_p, 16, STRING_WITH_QUOTE );

	cprintf( bold, CONT, "%s", s );

	dispose_string_object( s );
	
	return ( strlen( s ) );
}




string_object format_analysis( string_object fmt )
{
	int		i	= 0;
	char	c;
	
	while ( (c	= *(fmt + i)) )
	{
		if ( (*(fmt + i) == '%') && (*(fmt + i + 1) != '%') && i )
			break;
		
		i++;
	}
	
	if ( !i )
		return ( NULL );
	
	return ( make_string_object( fmt, i ) );
}


int find_type( string_object s )
{
	if ( '%' != *s++ )
		return ( NOT_DEFINED );
		
	while ( s )
	{
		switch ( *s++ )
		{
			case 'd' :
			case 'i' :
			case 'o' :
			case 'u' :
			case 'x' :
			case 'X' :
			case 'D' :
			case 'O' :
			case 'U' :
				return ( INTEGER );
				break;
			case 'e' :
			case 'E' :
			case 'f' :
			case 'g' :
			case 'G' :
				return ( FLOAT );
				break;

			case 'c' :
			case 's' :
				return ( STRING );
				break;
			
			case 'p' :
			case 'n' :
				return ( TYPE_UNKNOWN );
				break;
		}
	}
	return ( NOT_DEFINED );
}




/*
        ####          #####  #####    CaFE (Calculator, Function Expandable)
      ##     ####    ##     ##        version 1.7 [Apr 27 2004 @ 11:39:52]
     ##    ##  ##   ####   ####       
    ##    ##  ##   ##     ##          (c) Tsukimidai Communications Inc. 
    ####   ### #  ##     #####            All rights reserved. 1991-2004
*/




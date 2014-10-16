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
#include	<limits.h>

#include	"stack.h"
#include	"string.h"
#include	"ui.h"

char	get_hex( string_object s, int *offset, int length );


void string_to_chars( unsigned char n_option )
{
	char	*sb;
	char	*s;
	char	t[ 2 ]	= { 0, 0 };
	long	i;
	
	if ( n_option == N_OPTION_ON )
		i	= pop_i();
	else
		i	= LONG_MAX;
	
	if ( !(sb	= pop_s()) )
		return;
		
	s	= sb;


	if ( i < 0 )
	{
		int		count;
		char	*ss;
		
		count	= strlen( s ) + i;
		
		if ( 0 < count )
		{
			ss	= make_string_object( sb, count );
			push_s( ss );
			dispose_string_object( ss );

			s	+= count;
		}

		i	= INT_MAX;
	}



	while ( (*t	= *s) && i-- )
	{
		push_s( t );
		s++;
	}
	
	if ( *s )
		push_s( s );
	
	dispose_string_object( sb );
}

void char_to_int( void )
{
	char	*s;
	
	if ( !(s	= pop_s()) )
		return;
		
	push_i( (long)((unsigned char)(*s)) );
	
	dispose_string_object( s );
}

void int_to_char( void )
{
	char	i[ 2 ];
	
	i[ 0 ]	= (unsigned char)pop_i();
	i[ 1 ]	= 0;

	push_s( i );
}

void string_to_token( unsigned char n_option )
{
	char			*sb;
	char			*s;
	string_object	token_string;
	long			i;
	
	if ( n_option == N_OPTION_ON )
		i	= pop_i();
	else
		i	= LONG_MAX;
	
	if ( !(sb	= pop_s()) )
		return;
		
	s	= sb;
	
	if ( i < 0 )
	{
		int		count;
		char	*ss;
		
		count	= string_token_count( s ) + i;
		
		if ( 0 < count )
		{
			for ( i = 0; i < count; i++ )
				get_next_token( &s, NORMAL );
				
			ss	= make_string_object( sb, s - sb );
			
			push_s( ss );
			
			dispose_string_object( ss );
		}
		
		i	= INT_MAX;

	}
	
	while ( i-- && (token_string    = get_next_token( &s, NORMAL )) )
	{
		push_s( token_string );
		dispose_string_object( token_string );
	}

	if ( *s )
		push_s( s );
	
	dispose_string_object( sb );
}


void string_reverse( void )
{
	char	*s;
	char	*t;
	char	*sb;
	char	*tb;
	
	if ( !(sb	= pop_s()) )
		return;
		
	s	= sb;

	tb	= make_string_object( s, -1 );	
	t	= tb;
	
	t	+= strlen( t );
	
	while ( (*s) )
		*--t	= *s++;
		
	push_s( t );
	
	dispose_string_object( sb );
	dispose_string_object( tb );
}

void string_len( void )
{
	char	*s;

	if ( !(s	= pop_s()) )
		return;

	push_s( s );
	push_i( strlen( s ) );
	dispose_string_object( s );
}

void string_wc( void )
{
	char	*s;

	if ( !(s	= pop_s()) )
		return;

	push_s( s );
	push_i( string_token_count( s ) );			
	dispose_string_object( s );
}


int string_token_count( string_object s )
{
	int		count	= 0;

	while ( get_next_token( &s, NORMAL ) )
		count++;

	return ( count );
}


string_object string_quote( string_object s, char q )
{
	int				additional_char_count	= 0;
	int				length;
	string_object	new_s;
	string_object	ns;
	char			c;
	int				i;

	length	= strlen( s );
	
	for ( i = 0; i < length; i++ )
	{
		c	= *(s + i);
		
		if ( (c == '\\') || (c == q) )
		{
			additional_char_count++;
		}
		else
		{
			switch ( c )
			{
				case '\n' :
				case '\r' :
				case '\t' :
					additional_char_count++;
					break;
				default :
					if ( ! isprint( c ) )
						additional_char_count	+= 3;
					break;
			}
		}
	}

	new_s	= make_string_object( "", length + additional_char_count + 2 );
	ns		= new_s;
	
	*ns++	= q;

	for ( i = 0; i < length; i++ )
	{
		c	= *s++;
#if 1

		switch ( c )
		{
			case '\\' :
				*ns++	= '\\';
				*ns++	= '\\';
				break;
			case '\"' :
				if ( '\"' == q )
				{
					*ns++	= '\\';
					*ns++	= q;
				}
				else
				{
					*ns++	= c;
				}
				break;
			case '\'' :
				if ( '\'' == q )
				{
					*ns++	= '\\';
					*ns++	= q;
				}
				else
				{
					*ns++	= c;
				}
				break;
			case '\n' :
				*ns++	= '\\';
				*ns++	= 'n';
				break;
			case '\r' :
				*ns++	= '\\';
				*ns++	= 'r';
				break;
			case '\t' :
				*ns++	= '\\';
				*ns++	= 't';
				break;
			default : 
				if ( ! isprint( c ) )
				{
					sprintf( ns, "\\x%02X", c );
					ns	+= 4;
				}
				else
					*ns++	= c;
				break;
		}


#else


		if ( (c == '\\') || (c == q) )
		{
			*ns++	= '\\';
		}
		else
		{
			switch ( c )
			{
				case '\n' :
					*ns++	= '\\';
					c		= 'n';
					break;
				case '\r' :
					*ns++	= '\\';
					c		= 'r';
					break;
				case '\t' :
					*ns++	= '\\';
					c		= 't';
					break;
				default :
					break;
			}
		}
		*ns++	= c;
#endif
		
	}
	
	*ns++	= q;
	*ns++	= '\0';

	return ( new_s );
}




int string_backslash_conversion( string_object s )
{
	int		token_length;
	int		i;
	int		j;
	char	c;
	
	token_length	= strlen( s );

	for ( i = 0, j = 0; i < token_length; i++, j++ )
	{
		if ( '\\' == (c		= *(s + i)) )
		{
			i++;
			c		= *(s + i);
		
			switch ( c )
			{
				case ' ' :
					c	= ' ';
					break;
				case 'n' :
				case 'v' :
				case 'f' :
					c	= '\n';
					break;
				case 'r' :
					c	= '\r';
					break;
				case 't' :
					c	= '\t';
					break;
				case 'x' : 
					c	= get_hex( s, &i, 3 );
					break;
				default :
					break;
			}
		}
		*(s + j)	= c;
	}
	*(s + j)	= '\0';

	return ( j );
}

				

char get_hex( string_object s, int *offset, int length )
{
	int		v	 = 0;
	char	*r;
	char	c;
	int		i;

	if ( NULL == (r	= (char *)alloca( length + 1 )) ) 
		cprintf( ERROR, ABORT, "get_hex alloca failed\n" );
		
	for ( i = 0; i < length; i++ )
	{
		c	= *(s + *offset + i + 1);
	
		if ( ! isxdigit( c ) )
			break;
		
		*(r + i)	= c;
	}

	*(r + i)	= '\0';
	
	sscanf( r, "%X", &v );
	*offset		+= i;

	return ( (char)v );
}
































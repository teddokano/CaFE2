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

#ifndef		STRING_H
#define		STRING_H


#define		N_OPTION_OFF	0
#define		N_OPTION_ON		1

void string_to_chars( unsigned char n_option );
void char_to_int( void );
void int_to_char( void );
void string_to_token( unsigned char n_option );
void string_reverse( void );
void string_len( void );
void string_wc( void );
int		string_token_count( string_object s );
string_object string_quote( string_object s, char q );
int string_backslash_conversion( string_object s );



#endif	//	STRING_H


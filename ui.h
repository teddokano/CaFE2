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

#ifndef		UI_H
#define		UI_H

#include	"stack.h"


#define		MAX_LEN		40

enum	{
			DISP_DEC,
			DISP_HEX
		};
		
enum	{
			STRING_WITHOUT_QUOTE,
			STRING_WITH_QUOTE
		};
		

extern		char	g_build_time[ MAX_LEN ];
extern		char	g_build_date[ MAX_LEN ];
extern		char	g_build_version[ MAX_LEN ];
extern		int		g_build_number;


void	ui_user_print( string_object s );
void	show_top( void );
void	show_stack( FILE *fp );
void	cprintf( int bold, int mode, char *format, ... );
void	ui_suppress_cprintf( int sw );
void	show_prompt( void );

char	*version_string( char *str );
void	ui_version_information( void );

void			ui_buildinfo( void );
string_object	ui_stack_item_to_string( stack_item *si_p, int length_limit, int mode );

void ui_format_print( void );
void ui_information( void );



#endif	//	UI_H


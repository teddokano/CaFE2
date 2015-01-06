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


#include	<ncurses.h>


#ifndef		CAFE2_H
#define		CAFE2_H

#define		CAFE2

//#define	VERSION_INFO		"2.0 rel-0.7 (type-s)"	//	2005 APR 11
//#define	VERSION_INFO		"2.0 rel-0.8"			//	2005 MAY 03
//#define	VERSION_INFO		"2.0 rel-0.9"			//	2006 MAR 11
//#define	VERSION_INFO		"2.0 rel-0.5"			//	2006 JUN 04 -- number rewound
//#define	VERSION_INFO		"2.0 rel-0.11"			//	2010 MAR 07
#define		VERSION_INFO		"2.0 rel-0.12"			//	2010 SEP 24 -- Tempe

#define		PREFERENCE_FILE1		"~/.cafe"
#define		PREFERENCE_FILE2		"~/.CaFE"
#define		PREFERENCE_FILE3		"~/.cafe.auto_preference"


#define		PROMPT_STR			"CaFE % "



#define		ERROR		-1
#define		NO_ERROR	0
#define		NORM		0	/*	for cprintf()  */
#define		BOLD		1	/*	for cprintf()  */
#define		WARN		2
#define		DBG			3

#define		SUPPRESS_ALL	0
#define		SUPPRESS_WARN	2
#define		SUPPRESS_OFF	3
#define		SHOW_ALL		4


#define		FALSE		0
#define		TRUE		1

#define		NORMAL						0
#define		FILE_PROCESSING		1

#define		FUNCTION_REGIST	0
#define		FILE_REGIST		1

#define		WITHOUT_SPACE	0
#define		WITH_SPACE		1

#define		DEBUG

//#define		MAX_SRC_LINE_LENGTH				256
//#define		MAX_COMMAND_TOKENS_IN_LINE		128
#define		MAX_SRC_LINE_LENGTH				512
#define		MAX_COMMAND_TOKENS_IN_LINE		256
#define		MAX_TOKEN_LENGTH				MAX_SRC_LINE_LENGTH

#define		MAX_MODE_LABEL_LENGTH			40


#define		CONT		0
#define		ABORT		1
#define		INT			1


typedef	struct source	{
							char	*base_str;
							char	*curr_char;
						}
						source;


typedef	struct _mode_item
						{
							int		value;
							char	name[ MAX_MODE_LABEL_LENGTH ];
							char	*str;
						}
						mode_item;


typedef	struct mode		{
							mode_item	interactive_mode;
							mode_item	get_stdin;
							mode_item	use_preference;
							mode_item	use_history;
							mode_item	put_history;
							mode_item	recursive_call_limit;
							mode_item	error_detected;
							mode_item	n_history;
							mode_item	quit;
							mode_item	function_exe_abort;
							mode_item	force_function_return;
							mode_item	ontime_display;
							mode_item	prompt;
							mode_item	format_int;
							mode_item	format_float;
							mode_item	format_timeconvert;
							mode_item	messagelevel;
							mode_item	print_target;
							mode_item	post_print_screen;
							mode_item	post_print_stack;
							mode_item	loop_count;
							mode_item	loop_start;
							mode_item	loop_step;
							mode_item	directory_default;
							mode_item	directory_src_files;
						}
						cafe_mode;



typedef	char *			string_object;

typedef struct	fraction
						{
							int		numerator;
							int		denominator;
						}
						fraction;

typedef struct	_name_chain		name_chain;
struct	_name_chain		{
							string_object	name;
							name_chain		*prev;
							int				reference;
						};


						

int				evaluate( char *string );
string_object	get_next_token( string_object *src_p, int flag );
void			dispose_string_object( char *token_ptr );
void			time_now( char *s );

string_object	make_string_object( string_object string, int length );
void			dispose_string_object( string_object string );
string_object	add_string( string_object base, string_object add, int with_space );

int				get_recursive_level( void );

void			name_chain_push( name_chain **ncpp, string_object s );
string_object	name_chain_pop( name_chain **ncpp );

void			prof_dump( void );

int				compile( string_object s );
void			intial_file_loads( void );



extern	cafe_mode	g_cafe_mode;
extern	char		**g_mode_label_list;


#ifdef DEBUG
#define   dprintf( x )   cprintf( x )
#endif	//	DEBUG

#endif	// CAFE2_H




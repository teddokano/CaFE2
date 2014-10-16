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

#ifndef		CONTROLS_H
#define		CONTROLS_H

#include	"stack.h"
#include	"functions.h"


#define		AS_COMMAND		0
#define		AS_PIPE			1


#define		IF				0
#define		IFELSE			(0x1<<0)
#define		IF_WITH_POP		(0x1<<1)

#define		NORMAL_VAR		0
#define		GLOBAL_VAR		1


void controls_g_context_stack_ptr( void );

void	controls_if( char **src_p, int els );
void	controls_times( char **src_p );
void	controls_while( char **src_p );
void	controls_evaluate( void );
void	controls_read_variable( char **src_p );
void	controls_write_variable( char **src_p );
void	controls_print( char **src_p );
void	controls_execution_in_shell( int mode, char **src_p );


void	controls_make_base_context( void );
void	controls_make_base_global( void );

void controls_dispose_base_context( void );
void controls_global_variable_declare( void );

void controls_private( void );
void controls_global( void );

void controls_set_executiong_package( char *name );
void controls_package_remove( void );
void controls_set_package( char *s );

void controls_function_call_prologue( function *func );
void controls_function_call_epilogue( void );

void controls_list_var( int global_mode );
void controls_parent_context( void );
void controls_restore_context( void );
void controls_comment( string_object *s );

void control_add_mode_name( void );


#endif	//	CONTROLS_H



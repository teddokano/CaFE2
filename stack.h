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

#ifndef	STACK_H
#define	STACK_H

#define		STACK_SAVE		0
#define		STACK_REVERT	1

enum	{
			INTEGER, 
			INTEGER_HEX, 
			FLOAT, 
			FRACT, 
			STRING, 
			COMMAND, 
			STACK, 
			NOT_DEFINED, 
			TYPE_UNKNOWN
		};
		
#define		TYPE_IDENTIFIER_LIST	{ 'i', 'i', 'f', '/', 's', 'c', '=', 'n', '?' };
#define		NUMBER_OF_TYPES			(TYPE_UNKNOWN + 1)

//extern char	g_type_identifier[];

typedef struct _stack_item	stack_item;

struct	_stack_item	{
						void		*item_p;
						int			type;
						int			index;
						stack_item	*next;
					};




typedef struct _stack	stack;

struct _stack	{
					stack_item		*stack_top_p;
					stack			*prev;
					int				id;				//	not used for default stack
					int				referenced;		//	not used for default stack
				};
				

void		push( double v );
double		pop( void );

void		stack_items_compare( void );

void		push_i( long v );
long		pop_i( void );
void		push_s( char *s );
string_object	pop_s( void );
stack		*pop_stack( void );

void		stack_new( void );
void		stack_stackcopy( void );
void		stack_parent( void );
void		stack_set_target( void );

void		push_item( void *item_p, int type, stack *targ_stk_p );
stack_item	*pop_item( stack *targ_stk_p );
stack_item	*peep_stack_item( stack *targ_stk_p );
stack_item	*make_stack_item( void *item_p, int type );
void		dispose_stack_item( stack_item *si_p );
int			stack_pointer( stack *targ_stk_p );
void		stack_clear( stack *targ_stk );
int			stack_save( void );
int			stack_undo( void );
void		stack_rcopy( stack *trg_p, stack *src_p );
void		stack_rotate( stack *targ_stk_p );
void		stack_npush( int mode );
void		stack_reverse( void );
stack		*make_stack( void );
void		dispose_stack( stack *s_p );

int stack_get_current_stack_id( void );



#endif	// STACK_H



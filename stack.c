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

#include	"stack.h"
#include	"ui.h"

#define		DEPTH		256


stack		g_stack									= { NULL, NULL, 0, 0 };
stack		*g_current_target_stack_p				= NULL;

int			g_touch									= FALSE;


int		compare( void );
stack	*current_target_stack( void );
void	set_target_stack( stack *sp );


void push( double v )
{
	push_item( &v, FLOAT, NULL );
}


double pop( void )
{
	stack_item	*si_p;
	double		v;
	
	if ( NULL == (si_p	= pop_item( NULL )) )
		return ( 0.00 );
	
	switch ( si_p->type )
	{
		case INTEGER : 
			v	= (double)(*((long *)(si_p->item_p)));
			break;
		case FLOAT : 
			v	= *((double *)(si_p->item_p));
			break;
		default : 
			cprintf( ERROR, CONT, "the stack top does not have numerical value\n" );
			v	= 0.00;
			break;
	}
	
	dispose_stack_item( si_p );
	
	return ( v );
}


void push_i( long v )
{
	push_item( &v, INTEGER, NULL );
}


long pop_i( void )
{
	stack_item	*si_p;
	int			v;
	
	if ( NULL == (si_p	= pop_item( NULL )) )
		return ( 0 );
	
	switch ( si_p->type )
	{
		case INTEGER : 
			v	= *((long *)(si_p->item_p));
			break;
		case FLOAT : 
			v	= (long)(*((double *)(si_p->item_p)));
			break;
		default : 
			cprintf( ERROR, CONT, "the stack top does not have numerical value\n" );
			v	= 0;
			break;
	}
	
	dispose_stack_item( si_p );
	
	return ( v );
}


void push_s( char *s )
{
	push_item( s, STRING, NULL );
}


string_object pop_s( void )
{
	stack_item		*si_p;
	string_object	s			= NULL;
	
	if ( NULL == (si_p	= pop_item( NULL )) )
		return ( NULL );

	switch ( si_p->type )
	{
		case INTEGER : 
		case FLOAT : 
		case STRING : 
		case STACK : 
			s	= ui_stack_item_to_string( si_p, -1, STRING_WITHOUT_QUOTE );
			break;
		default : 
			cprintf( ERROR, CONT, "the stack top could not converted to string\n" );
			break;
	}
	
	dispose_stack_item( si_p );
	
	if ( !s )
		cprintf( ERROR, CONT, "can't make string_object in pop_s\n" );
	
	return ( s );
}


stack *pop_stack( void )
{
	stack_item		*si_p;
	stack			*s_p;
	
	if ( NULL == (si_p	= pop_item( NULL )) )
		return ( NULL );

	if ( si_p->type == STACK )
		s_p		= *((stack **)si_p->item_p);
	else
		s_p		= NULL;
	
	dispose_stack_item( si_p );
	
	return ( s_p );
}


void stack_new( void )
{
	stack		*new_stack;
	static int	serial	= 1;

	if ( !(new_stack	= make_stack()) )
	{
		cprintf( ERROR, CONT, "coudn't make new stack\n" );
		return;
	}
	
	new_stack->id			= serial++;
	new_stack->referenced	= 0;

	push_item( (void *)(&new_stack), STACK, NULL );
}


void push_item( void *item_p, int type, stack *targ_stk_p )
{
	stack_item	*new_si_p;
	
	if ( !targ_stk_p )
		targ_stk_p	= current_target_stack();

	if ( NULL == (new_si_p	= make_stack_item( item_p, type )) )
	{
		cprintf( ERROR, CONT, "can't push stack any more\n" );
		return;
	}
	
	new_si_p->next	=  targ_stk_p->stack_top_p;
	new_si_p->index	= (targ_stk_p->stack_top_p == NULL) ? 0 : targ_stk_p->stack_top_p->index + 1;
	
	targ_stk_p->stack_top_p	=  new_si_p;
	g_touch		= TRUE;
}


stack_item *make_stack_item( void *item_p, int type )
{
	stack_item	*new_si_p;
	int			size;
	
	if ( NULL == (new_si_p	= (stack_item *)malloc( sizeof( stack_item ) )) )
	{
		cprintf( ERROR, CONT, "can't push stack any more\n" );
		return ( NULL );
	}
	
	new_si_p->type	= type;

	switch ( type )
	{
		case INTEGER : 
			size	= sizeof( long );
			break;
		case FLOAT : 
			size	= sizeof( double );
			break;
		case STRING : 
			size	= sizeof( char ) * (strlen( (char *)item_p ) + 1);
			break;
		case STACK : 
			size	= sizeof( stack * );
			break;
		default :
			size	= 0;
			break;
	}
	
	if ( !size )
	{
		cprintf( ERROR, CONT, "can't push, data is unknown type\n" );
		free( new_si_p );
		
		return ( NULL );
	}
	else
	{
		if ( NULL == (new_si_p->item_p	= (void *)malloc( size )) )
		{
			cprintf( ERROR, CONT, "can't push, can not allocate in stack item\n" );
			free( new_si_p );
			
			return ( NULL );
		}
	}
	
	switch ( type )
	{
		case INTEGER : 
			*((long *)new_si_p->item_p)		= *((long *)item_p);
			break;
		case FLOAT : 
			*((double *)new_si_p->item_p)	= *((double *)item_p);
			break;
		case STRING : 
			strcpy( (char *)(new_si_p->item_p), (char *)item_p );
			break;
		case STACK : 
			*((stack **)new_si_p->item_p)	= *((stack **)item_p);
			((*((stack **)item_p))->referenced)++;
			break;
	}

	return ( new_si_p );
}


stack_item *pop_item( stack *targ_stk_p )
{
	stack_item	*si_p;

	if ( !targ_stk_p )
		targ_stk_p	= current_target_stack();

	if ( NULL == (si_p	= targ_stk_p->stack_top_p) )
	{
		cprintf( WARN, CONT, "   !!! stack empty !!!\n" );
		return ( NULL );
	}
		
	targ_stk_p->stack_top_p	= si_p->next;

	g_touch		= TRUE;

	return ( si_p );
}


stack_item *peep_stack_item( stack *targ_stk_p )
{
	if ( !targ_stk_p )
		targ_stk_p	= current_target_stack();
		
	return ( targ_stk_p->stack_top_p );
}


void dispose_stack_item( stack_item *si_p )
{
	if ( si_p->type == STACK )
	{
		stack	*stackp;
	
		stackp	= *((stack **)(si_p->item_p));
		
//		cprintf( BOLD, CONT, "   !!! stack refcheck !!!  <0x%08X> =%d= (%d)", (unsigned)stackp, stackp->id, stackp->referenced );

		
		if ( !( --(stackp->referenced) ) )
			dispose_stack( stackp );
	}

	free( si_p->item_p );
	free( si_p );
}


void stack_items_compare( void )
{
	push_i( compare() );
}


int compare( void )
{
	stack_item	*s0;
	stack_item	*s1;

	if ( NULL == (s0	= pop_item( NULL )) )
		return ( FALSE );

	if ( NULL == (s1	= pop_item( NULL )) )
		return ( FALSE );

	if ( s0->type != s1->type )
		return ( FALSE );

	switch ( s0->type )
	{
		case INTEGER : 
			if ( *((long *)(s0->item_p)) == *((long *)(s1->item_p)) )
				return ( TRUE );
			else
				return ( FALSE );
			//break;
		case FLOAT : 
			if ( *((double *)(s0->item_p)) == *((double *)(s1->item_p)) )
				return ( TRUE );
			else
				return ( FALSE );
			//break;
		case STRING : 
#if 0
{
	int		i;
	char	*a;
	char	*b;
	
	a		= (char *)(s0->item_p);
	b		= (char *)(s1->item_p);
	
	cprintf( NORM, CONT, "\n" );
	
	do
	{
		cprintf( NORM, CONT, " %02X", *a );
	}
	while ( *a++ );
	
	cprintf( NORM, CONT, "\n" );

	do
	{
		cprintf( NORM, CONT, " %02X", *b );
	}
	while ( *b++ );
	
	cprintf( NORM, CONT, "\n" );
}
#endif
		
			if ( !strcmp( (char *)(s0->item_p), (char *)(s1->item_p) ) )
				return ( TRUE );
			else
				return ( FALSE );
			//break;
		default :
			break;
	}
	
	return ( FALSE );
}


int stack_pointer( stack *targ_stk_p )
{
	if ( !targ_stk_p )
		targ_stk_p	= current_target_stack();
		
	if ( !targ_stk_p->stack_top_p )
		return ( -1 );
		
	return ( targ_stk_p->stack_top_p->index + 1 );
}


void stack_clear( stack *targ_stk_p )
{
	stack_item	*si_p;
	stack_item	*si_pp;

	if ( !targ_stk_p )
		targ_stk_p	= current_target_stack();
		
	si_p					= targ_stk_p->stack_top_p;
	targ_stk_p->stack_top_p	= NULL;

	while ( si_p )
	{
		si_pp	= si_p->next;
		dispose_stack_item( si_p );
		si_p	= si_pp;
	}

	g_touch		= TRUE;
}


int stack_save( void )
{
	stack	*stkp;
	stack	*current_p;

	if ( !g_cafe_mode.interactive_mode.value )
		return ( NO_ERROR );
	
	if ( !g_touch )
		return ( NO_ERROR );
	
	if ( NULL == (stkp	= make_stack()) )
	{
		cprintf( ERROR, CONT, "unrecoverable @ stack_copy (1)\n" );
		exit ( 1 );
	}
	
	current_p		= current_target_stack();
	
	stkp->prev		= current_p->prev;
	current_p->prev	= stkp;

	stack_rcopy( stkp, current_p );

	g_touch		= FALSE;

	return ( NO_ERROR );
}


int stack_undo( void )
{
	stack	*s_p;
	stack	*current_p;

	current_p	= current_target_stack();

	if ( !current_p->prev )
		return ( NO_ERROR );	

	s_p		= (current_p->prev)->prev;
	
	if ( !s_p )
		return ( NO_ERROR );
		
	stack_clear( current_p );
	stack_rcopy( current_p, s_p );
	
	dispose_stack( current_p->prev );

	current_p->prev	= s_p->prev;
	
	dispose_stack( s_p );

	g_touch		= TRUE;

	return ( NO_ERROR );
}


void stack_rcopy( stack *trg_p, stack *src_p )
{
	stack_item	*si_p;

	if ( !src_p )
		src_p	= current_target_stack();

	si_p	= src_p->stack_top_p;

	while ( si_p )
	{
		push_item( si_p->item_p, si_p->type, trg_p );
		si_p	= si_p->next;
	}
}


void stack_rotate( stack *targ_stk_p )
{
	stack_item	*top_p;
	stack_item	*bottom_p;
	int			depth;

	if ( !targ_stk_p )
		targ_stk_p	= current_target_stack();
	
	top_p	= targ_stk_p->stack_top_p;

	if ( !top_p )
		return;

	depth	= top_p->index;
	
	if ( depth < 1 )
		return;
	
	bottom_p	= top_p;
	
	do
	{
		bottom_p	= bottom_p->next;
	}
	while ( bottom_p->next );

	targ_stk_p->stack_top_p		= top_p->next;
	bottom_p->next				= top_p;
	top_p->next					= NULL;
	
	//	renumber index

	top_p	= targ_stk_p->stack_top_p;
	
	while ( top_p )
	{
		top_p->index	= depth--;
		top_p			= top_p->next;
	}
}


void stack_stackcopy( void )
{
/****************************************************************************

s1 s2 stackcopy
^^^^^^^^^^^^^^^
	[ 1  2  3  4  5  s1  s2 ] <<
					 |   [  a  b  c  d ]
					 [ 6  7  8  9  0 ]

	[ 1  2  3  4  5  s1  s2 ]
					 |   [  a  b  c  d  6  7  8  9  0 ]
					 [ 6  7  8  9  0 ]



s2 s1 stackcopy
^^^^^^^^^^^^^^^
	[ 1  2  3  4  5  s2  s1 ] <<
					 |   [ 6  7  8  9  0 ]
					 [  a  b  c  d ]

	[ 1  2  3  4  5  s2  s1 ]
					 |   [ 6  7  8  9  0  a  b  c  d ]
					 [  a  b  c  d ]



 0 s1 stackcopy
^^^^^^^^^^^^^^^
	[ 1  2  3  4  5  0  s1 ] <<
					  [ 6  7  8  9  ]

	[ 1  2  3  4  5  s1 ]
				   [ 6  7  8  9  1  2  3  4  5 ]



s1  0 stackcopy
^^^^^^^^^^^^^^^
	[ 1  2  3  4  5  s1  0 ] <<
				   [ 6  7  8  9 ]

	[ 1  2  3  4  5  6  7  8  9  s1 ]
							   [ 6  7  8  9 ]

****************************************************************************/

	stack_item	*target_posision_itemp;
	stack_item	*source_posision_itemp;
	stack		*target		= NULL;
	stack		*source		= NULL;
	stack		*stkp1;
	stack		*current;
	
	//	make a temp. stack for copy the contents
	
	if ( NULL == (stkp1	= make_stack()) )
	{
		cprintf( ERROR, CONT, "unrecoverable @ stackcopy (1)\n" );
		exit ( 1 );
	}


	//	pop two items from current stack

	if ( !(target_posision_itemp	= pop_item( NULL )) )
	{
		cprintf( ERROR, CONT, "stackcopy : no stack to operate\n" );
		return;
	}
	if ( !(source_posision_itemp	= pop_item( NULL )) )
	{
		cprintf( ERROR, CONT, "stackcopy : no stack to operate\n" );
		dispose_stack_item( target_posision_itemp );
		return;
	}
	
	

	if ( target_posision_itemp->type == STACK )
		target		= *((stack **)target_posision_itemp->item_p);

	if ( source_posision_itemp->type == STACK )
		source		= *((stack **)source_posision_itemp->item_p);

	//cprintf( BOLD, CONT, "stacks <0x%08X> >> <0x%08X>\n", source, target );

	stack_rcopy( stkp1, source );
	stack_rcopy( target, stkp1 );
	
	dispose_stack( stkp1 );

	current	= current_target_stack();

	//cprintf( BOLD, CONT, "stacks <0x%08X> >> <0x%08X>\n", source, target );

	if ( source && (source != current->prev) )
		push_item( source_posision_itemp->item_p, source_posision_itemp->type, current );

	if ( target && (target != current->prev) )
		push_item( target_posision_itemp->item_p, target_posision_itemp->type, current );

	dispose_stack_item( target_posision_itemp );
	dispose_stack_item( source_posision_itemp );
}


void stack_npush( int mode )
{
	stack		*source;
	stack		*target;
	stack		*s_p;
	stack		*r_p;
	stack_item	*si_p;
	long		n;
	long		i;
	
	if ( NULL == (s_p	= make_stack()) )
	{
		cprintf( ERROR, CONT, "fail to npush\n" );
		return;
	}
	
	if ( NULL == (r_p	= make_stack()) )
	{
		cprintf( ERROR, CONT, "fail to npush\n" );
		dispose_stack( s_p );
		return;
	}

	n	= pop_i();
	
	if ( 0 == mode )
	{
		source	= NULL;
		target	= NULL;
	}
	else
	{
		target	= pop_stack();
		source	= pop_stack();
	}
	
	// pop N items from source stack to current
	
	for ( i = 0; i < n; i++ )
	{
		if ( NULL == (si_p	= pop_item( source )) )
			break;
		
		push_item( si_p->item_p, si_p->type, s_p );
		dispose_stack_item( si_p );
	}
	
	n	= i;

	stack_rcopy( r_p, s_p );
	
	for ( i = 0; i < n; i++ )
	{
		si_p	= pop_item( s_p );
		push_item( si_p->item_p, si_p->type, source );
		dispose_stack_item( si_p );
	}
	
	stack_rcopy( s_p, r_p );

	for ( i = 0; i < n; i++ )
	{
		si_p	= pop_item( s_p );
		push_item( si_p->item_p, si_p->type, target );
		dispose_stack_item( si_p );
	}
	
	dispose_stack( s_p );
	dispose_stack( r_p );
}


void stack_reverse( void )
{
	stack	*stkp1;
	stack	*stkp2;
	
	if ( NULL == (stkp1	= make_stack()) )
	{
		cprintf( ERROR, CONT, "unrecoverable @ stack_reverse (1)\n" );
		exit ( 1 );
	}
	if ( NULL == (stkp2	= make_stack()) )
	{
		cprintf( ERROR, CONT, "unrecoverable @ stack_reverse (1)\n" );
		exit ( 1 );
	}

	stack_rcopy( stkp1, current_target_stack() );
	stack_rcopy( stkp2, stkp1 );
	
	stack_clear( current_target_stack() );
	
	stack_rcopy( current_target_stack(), stkp2 );
	
	dispose_stack( stkp1 );
	dispose_stack( stkp2 );
}


stack *make_stack( void )
{
	stack	*s_p;
	
	s_p		= (stack *)malloc( sizeof( stack ) );
	
	if ( !s_p )
		return ( NULL );
	
	s_p->stack_top_p	= NULL;
	s_p->prev			= current_target_stack();
//printf( "sp %p, sp->prev %p\n", s_p, s_p->prev );

	return ( s_p );
}


void dispose_stack( stack *stackp )
{
	if ( !stackp || (stackp == &g_stack) )
		return;
		
	stack_clear( stackp );
	free( stackp );
}


void stack_parent( void )
{
	stack	*stackp;

	stackp	= current_target_stack();
	
	if ( stackp->prev )
		push_item( &(stackp->prev), STACK, stackp );
	else
		push_i( 0 );
}


void stack_set_target( void )
{
	stack		*stackp		= NULL;
	stack_item	*si_p;
	
	si_p	= peep_stack_item( NULL );
	
	if ( si_p )
	{
		if ( si_p->type == STACK )
		{
			stackp	= *((stack **)si_p->item_p);
			
			if ( current_target_stack()->prev == stackp )
//			if ( current_target_stack()->prev )
			{
				pop_stack();
			}
		}
		else
		{
			pop_stack();
		}
	}
	
	set_target_stack( stackp );
}

int stack_get_current_stack_id( void )
{
	return ( current_target_stack()->id );
}


stack *current_target_stack( void )
{
	if ( g_current_target_stack_p )
		return ( g_current_target_stack_p );	
	else
		return ( &g_stack );
}


void set_target_stack( stack *sp )
{
//	stack	*stackp;
	
//	stackp	= current_target_stack();

//	cprintf( BOLD, CONT, "@@@@  stack switched  <0x%08lX>  >>  ", stackp );
	
	if ( sp )
		g_current_target_stack_p		= sp;
	
//	cprintf( BOLD, CONT, "<0x%08lX>\n", g_current_target_stack_p  );

}

























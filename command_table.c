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
#include	<math.h>
#include	<time.h>
#include	<limits.h>

#include	"command_table.h"
#include	"stack.h"
#include	"functions.h"
#include	"controls.h"
#include	"fileop.h"
#include	"ui.h"
#include	"key.h"
#include	"string.h"
#include	"debugger.h"
#include	"key.h"

#define		Pi					3.141592653589793238462643383279502884197169399375105820974944592

enum	{
			OP_ADD,
			OP_SUB,
			OP_MUL,
			OP_DIV,
			OP_IGNORE
		};


typedef struct _keywd		keywd;

struct _keywd			{
							string_object	key;
							int				comm_table_index;
							function		*function_ptr;
							keywd			*next;
							keywd			*prev;
							keywd			*hash_chain;
						};


keywd			*g_keywd				= NULL;


void		four_operations( int operator );
void		bit_operations( int operation );

void		initialize_keywords( void );
void		key_list( void );
keywd		*create_keywd( char *key );
void		dispose_keywd( keywd *kwp );
void		insert_key( keywd *prev, keywd *new );
void		sort_kw( keywd *kwp_base );
void		remap( keywd *kwp, keywd **kwp_array, int length );
keywd		*find_key_insert_point( char *key );
keywd		*find_keyword( char *key );
int hash( string_object s );
void add_to_hash_table( keywd *kwp );
void remove_from_hash_table( char *key );

int general_compare( stack_item *s1, stack_item *s2 )
{
	int		result;
	double	v;
	
	if ( (result	= s2->type - s1->type) )
		return result;
	
	switch ( s2->type )
	{
		case INTEGER : 
			result	= *((long *)(s2->item_p)) - *((long *)(s1->item_p));
			break;
		case FLOAT : 
			v		= *((double *)(s2->item_p)) - *((double *)(s1->item_p));
			result	= (v    < 0.00) ? -1 : 0;
			result	= (0.00 < v   ) ?  1 : result;
			break;
		case STRING : 
			result	= strcmp( (char *)(s2->item_p), (char *)(s1->item_p) );
			break;
		default : 
			result	= 0;
			break;
	}

	return result;
}

void command_sort( string_object *src_p )
{
	int		depth;
	int		i;
	int		j;
	
	depth	= stack_pointer( NULL );
	stack_item	*sip[ depth ];
	stack_item	*ts;

	if ( depth < 2 )
		return;

	for ( i = 0; i < depth; i++ )
	{
		sip[ i ]	= pop_item( NULL );
	}
	
	for ( i = 0; i < depth; i++ )
	{
		for ( j = 1; j < (depth - i); j++ )
		{
			if ( general_compare( sip[ j -1 ], sip[ j ] ) < 0 )
			{
				ts				= sip[ j     ];
				sip[ j     ]	= sip[ j - 1 ];
				sip[ j - 1 ]	= ts;
			}
		}
	}

	for ( i = 0; i < depth; i++ )
		push_item( sip[ i ]->item_p, sip[ i ]->type, NULL );
}

void command_uniq( string_object *src_p )
{
	int		depth;
	int		i;
	
	depth	= stack_pointer( NULL );
	stack_item	*sip[ depth ];

	if ( depth < 2 )
		return;

	for ( i = 0; i < depth; i++ )
		sip[ i ]	= pop_item( NULL );
		
	for ( i = (depth - 1); i > 0; i-- )
	{
		if ( general_compare( sip[ i -1 ], sip[ i ] ) )
			push_item( sip[ i ]->item_p, sip[ i ]->type, NULL );
		else
			dispose_stack_item( sip[ i ] );
	}

	push_item( sip[ 0 ]->item_p, sip[ 0 ]->type, NULL );
}


void command_nop( string_object *src_p )
{
}


void command_add( string_object *src_p )
{
	four_operations( OP_ADD );
}

void command_sub( string_object *src_p )
{
	four_operations( OP_SUB );
}

void command_mul( string_object *src_p )
{
	four_operations( OP_MUL );
}

void command_dev( string_object *src_p )
{
	four_operations( OP_DIV );
}




void four_operations( int operator )
{
	stack_item	*s2;
	stack_item	*s1;
	int			return_type		= INTEGER;

	if ( NULL == (s2	= pop_item( NULL )) )
		return;
	
	if ( s2->type == STRING )
	{
		cprintf( ERROR, CONT, "the stack top does not have numerical value\n" );
		dispose_stack_item( s2 );
		
		return;
	}
	
	if ( NULL == (s1	= pop_item( NULL )) )
	{
		dispose_stack_item( s2 );
		return;
	}
	
	if ( s1->type == STRING )
	{
		cprintf( ERROR, CONT, "the stack top does not have numerical value\n" );
		dispose_stack_item( s1 );
		dispose_stack_item( s2 );
		
		return;
	}
	
	if ( (s1->type == FLOAT) || (s2->type == FLOAT) )
		return_type	= FLOAT;

	if ( operator == OP_DIV )
	{
		if (   ((s2->type == INTEGER) && (0   == *((long   *)(s2->item_p)))) 
			|| ((s2->type == FLOAT  ) && (0.0 == *((double *)(s2->item_p))))
			)
		{
			operator	= OP_IGNORE;
			cprintf( ERROR, CONT, "zero devisor\n");
		}		
	}

	if ( (operator == OP_DIV) && (return_type == INTEGER) && *((long *)(s1->item_p)) % *((long *)(s2->item_p)) )
		return_type	= FLOAT;

	if ( return_type == INTEGER )
	{
		long	v2;
		long	v1;
		
		v2	= *((long *)(s2->item_p));
		v1	= *((long *)(s1->item_p));

//cprintf( NORM, CONT, "%ld, %ld, %ld, %ld\n", LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN );
		
		//	
		//	integer overflow detection added
		//		source: https://www.jpcert.or.jp/sc-rules/c-int32-c.html
		//

		switch ( operator )
		{
			case OP_ADD : 
			  	if (	((v2 > 0) && (v1 > (LONG_MAX - v2))) ||
      					((v2 < 0) && (v1 < (LONG_MIN - v2)))
      				)
				{
					cprintf( NORM, CONT, "result is converted to float\n" );
    				return_type	= FLOAT;
 				}
 				else
 				{
					push_i( v1 + v2 );
				}
				break;
			case OP_SUB : 
				if (	(v2 > 0 && v1 < LONG_MIN + v2) ||
    					(v2 < 0 && v1 > LONG_MAX + v2)
    				)
				{
					cprintf( NORM, CONT, "result is converted to float\n" );
    				return_type	= FLOAT;
				}
				else
				{
					push_i( v1 - v2 );
				}
				break;
			case OP_MUL : 
				if ( v1 > 0 )
				{
        			if ( v2 > 0 )
        			{
            			if ( v1 > (LONG_MAX / v2) )
            			{
                			return_type	= FLOAT;
            			}
        			}
        			else
        			{
            			if ( v2 < (LONG_MIN / v1) )
            			{
                			return_type	= FLOAT;
            			}
        			}
    			}
    			else
    			{
					if ( v2 > 0 )
					{
						if ( v1 < (LONG_MIN / v2) )
						{
                			return_type	= FLOAT;
            			}
        			}
        			else
        			{
						if ( (v1 != 0) && (v2 < (LONG_MAX / v1)) )
						{
                			return_type	= FLOAT;
            			}
        			}
    			}

    			if ( return_type == FLOAT )
    			{
    				cprintf( NORM, CONT, "result is converted to float\n" );
    			}
    			else
				{
					push_i( v1 * v2 );
				}

				break;
			case OP_DIV : 
				if ( (v1 == LONG_MIN) && (v2 == -1) ) 
				{
					return_type	= FLOAT;
                    cprintf( NORM, CONT, "result is converted to float\n" );
  				}
  				else
  				{
					push_i( v1 / v2 );
				}
				break;
			case OP_IGNORE :
				//	may be trying devide by zero
				break;
		}
	}
	
	if ( return_type == FLOAT )
	{
		double	v2;
		double	v1;

		if ( s2->type == INTEGER )
			v2	= (double)(*((long *)(s2->item_p)));
		else
			v2	= *((double *)(s2->item_p));

		if ( s1->type == INTEGER )
			v1	= (double)(*((long *)(s1->item_p)));
		else
			v1	= *((double *)(s1->item_p));
		
		switch ( operator )
		{
			case OP_ADD : 
				push( v1 + v2 );
				break;
			case OP_SUB : 
				push( v1 - v2 );
				break;
			case OP_MUL : 
				push( v1 * v2 );
				break;
			case OP_DIV : 
				push( v1 / v2 );
				break;
			case OP_IGNORE :
				//	may be trying devide by zero
				break;
		}
	}
	else if ( return_type == STRING )
	{
#if 0
		string_object	s2;
		string_object	s1;
		int				total_length;
#endif
	}
	
	
	dispose_stack_item( s1 );
	dispose_stack_item( s2 );
}

void command_modulo( string_object *src_p )
{
	long	op1;
	long	op2;
	
	op2		= pop_i();
	op1		= pop_i();

	if ( !op2 )
		cprintf( ERROR, CONT, "zero devisor at modulo\n");
	else if ( (op1 == LONG_MIN) && (op2 == -1) )
		cprintf( ERROR, CONT, "modulo cannot handle this \"%ld %ld %%\"\n", op1, op2 );
	else
		push_i( op1 % op2 );
}


void command_pow( string_object *src_p )
{
	double	op2;
	
	op2		= pop();
	
	push( pow( pop(), op2 ) );
}


void command_recipro( string_object *src_p )
{
	double	op;
	
	op	= pop();
	
	if (op != 0.00)
		push( 1 / op );
	
	else
		cprintf( ERROR, CONT, "zero devisor in revrse\n");
}


void command_para( string_object *src_p )
{
	double	op1,
			op2;
	
	op1		= pop();
	op2		= pop();
	
	push( (op1 * op2) / (op1 + op2) );
}


void command_log( string_object *src_p )
{
	double	op;
	
	op	= pop();
	
	if ( op > 0 )
		push( log( op ) );

	else
		cprintf( ERROR, CONT, "op <= 0 in log\n");
}


void command_logt( string_object *src_p )
{
	double	op;
	
	op	= pop();
	
	if ( op > 0 )
		push( log10( op ) );

	else
		cprintf( ERROR, CONT, "op <= 0 in log\n");
}


void command_sin( string_object *src_p )
{
	push( sin( pop() ) );
}


void command_cos( string_object *src_p )
{
	push( cos( pop() ) );
}


void command_tan( string_object *src_p )
{
	push( tan( pop() ) );
}


void command_asin( string_object *src_p )
{
	push( asin( pop() ) );
}


void command_acos( string_object *src_p )
{
	push( acos( pop() ) );
}


void command_atan( string_object *src_p )
{
	push( atan( pop() ) );
}

#define		BITOP_SHIFT_LEFT	0
#define		BITOP_SHIFT_RIGHT	1
#define		BITOP_AND			2
#define		BITOP_OR			3
#define		BITOP_EXOR			4
#define		BITOP_NOT			5

void command_nleft( string_object *src_p )
{
	bit_operations( BITOP_SHIFT_LEFT );
}


void command_nright( string_object *src_p )
{
	bit_operations( BITOP_SHIFT_RIGHT );
}


void command_and( string_object *src_p )
{
	bit_operations( BITOP_AND );
}


void command_or( string_object *src_p )
{
	bit_operations( BITOP_OR );
}


void command_exor( string_object *src_p )
{
	bit_operations( BITOP_EXOR );
}


void command_not( string_object *src_p )
{
	bit_operations( BITOP_NOT );
}


void bit_operations( int operation )
{
	long	op1;
	long	op2		= 0;

	op1		= pop_i();
	
	if ( operation != BITOP_NOT )
		op2		= pop_i();
	
	switch ( operation )
	{
		case BITOP_SHIFT_LEFT	:
			push_i( op2 << op1 );
			break;
		case BITOP_SHIFT_RIGHT	:
			push_i( op2 >> op1 );
			break;
		case BITOP_AND			:
			push_i( op2 & op1 );
			break;
		case BITOP_OR			:
			push_i( op2 | op1 );
			break;
		case BITOP_EXOR			:
			push_i( op2 ^ op1 );
			break;
		case BITOP_NOT			:
			push_i( ~op1 );
			break;
		default					: 
			break;
	}
}





void command_trunc( string_object *src_p )
{
	push_i( pop_i() );
}


void command_float( string_object *src_p )
{
	push( pop() );
}


void command_string( string_object *src_p )
{
	string_object	s;

	if ( !(s	= pop_s()) )
		return;
	
	push_s( s );

	dispose_string_object( s );
}


void command_quote( string_object *src_p )
{
	string_object	s;
	string_object	new_s;

	if ( !(s	= pop_s()) )
		return;
	
	new_s	= string_quote( s, '\"' );
	push_s( new_s );

	dispose_string_object( new_s );
	dispose_string_object( s );
}


void command_squote( string_object *src_p )
{
	string_object	s;
	string_object	new_s;

	if ( !(s	= pop_s()) )
		return;
	
	new_s	= string_quote( s, '\'' );
	push_s( new_s );

	dispose_string_object( new_s );
	dispose_string_object( s );
}


void command_type( string_object *src_p )
{
	stack_item		*si_p;

	if ( NULL == (si_p	= peep_stack_item( NULL )) )
		return;
		
	switch ( si_p->type )
	{
		case INTEGER :
			push_s( "INTEGER" );
			break;
		case FLOAT : 
			push_s( "FLOAT" );
			break;
		case STRING : 
			push_s( "STRING" );
			break;
		case STACK : 
			push_s( "STACK" );
			break;
	}
}

void command_pi( string_object *src_p )
{
	push( Pi );
}


void command_function_define( string_object *src_p )
{
	function_define( src_p );
}


void command_function_forget( string_object *src_p )
{
	function_forget( src_p );
}


void command_function_rename( string_object *src_p )
{
	function_rename( src_p );
}


void command_function_remove( string_object *src_p )
{
	function_remove( src_p );
}


void command_function_replace( string_object *src_p )
{
	function_replace( src_p );
}


void command_function_tokenize( string_object *src_p )
{
	function_tokenize();
}


void command_function_self( string_object *src_p )
{
	function_self();
}


void command_function_body( string_object *src_p )
{
	function_body( src_p );
}


void command_fshow( string_object *src_p )
{
	show_function( src_p );
}


void command_fisdef( string_object *src_p )
{
	function_isdefined( src_p );
}




void command_function_list( string_object *src_p )
{
	function_list( NULL );
}


void command_limitrecursive( string_object *src_p )
{
	g_cafe_mode.recursive_call_limit.value	= pop_i();
}





void command_if( string_object *src_p )
{
	controls_if( src_p, IF );
}


void command_ifelse( string_object *src_p )
{
	controls_if( src_p, IFELSE );
}


void command_ifp( string_object *src_p )
{
	controls_if( src_p, IF | IF_WITH_POP );
}


void command_ifpelse( string_object *src_p )
{
	controls_if( src_p, IFELSE | IF_WITH_POP );
}


void command_times( string_object *src_p )
{
	controls_times( src_p );
}


void command_while( string_object *src_p )
{
	controls_while( src_p );
}


void command_evaluate( string_object *src_p )
{
	controls_evaluate();
}


void command_die( string_object *src_p )
{
	g_cafe_mode.function_exe_abort.value	= TRUE;
}


void command_readvar( string_object *src_p )
{
	controls_read_variable( src_p );
}


void command_writevar( string_object *src_p )
{
	controls_write_variable( src_p );
}

void command_exclamation( string_object *src_p )
{
	controls_execution_in_shell( AS_COMMAND, src_p );
}


void command_double_exclamation( string_object *src_p )
{
	controls_execution_in_shell( AS_PIPE, src_p );
}


void command_tax( string_object *src_p )
{
	long	n,
			t;
			
	t		=  (n = pop())  * (.05 / 1.05);
	n		-= t;
	push( n + t );
	
	cprintf( BOLD, CONT, "net : %ld,\ttax : %ld\n", n, t);
}


void command_use( string_object *src_p )
{
	fileop( src_p, FILEOP_USE );
}


void command_put( string_object *src_p )
{
	fileop( src_p, FILEOP_PUT );
}


void command_save( string_object *src_p )
{
	fileop( src_p, FILEOP_SAVE );
}


void command_cd( string_object *src_p )
{
	fileop( src_p, FILEOP_CHDIR );
}


void command_stack( string_object *src_p )
{
	show_stack( NULL );
}


void command_dot( string_object *src_p )
{
	show_top();
}


void command_clear( string_object *src_p )
{
	stack_clear( NULL );
}


void command_pop( string_object *src_p )
{
	stack_item	*si_p;

	if ( (si_p	= pop_item( NULL )) )
		dispose_stack_item( si_p );
}

void command_push( string_object *src_p )
{
	stack_item	*si_p;

	si_p	= peep_stack_item( NULL );
	
	if ( !si_p )
		return;

	push_item( si_p->item_p, si_p->type, NULL );
}


void command_newstack( string_object *src_p )
{
	stack_new();
}


void command_target( string_object *src_p )
{
	stack_set_target();
}


void command_stackcopy( string_object *src_p )
{
	stack_stackcopy();
}


void command_stackncopy( string_object *src_p )
{
	stack_npush( 1 );
}


void command_parent( string_object *src_p )
{
	stack_parent();
}


void command_compare( string_object *src_p )
{
	stack_items_compare();
}

void command_npush( string_object *src_p )
{
	stack_npush( 0 );

#if 0

	stack		*s_p;
	stack		*r_p;
	stack_item	*si_p;
	int			n;
	int			i;
	
	n	= pop_i();
	
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
	
	for ( i = 0; i < n; i++ )
	{
		if ( NULL == (si_p	= pop_item( NULL )) )
			break;
		
		push_item( si_p->item_p, si_p->type, s_p );
		dispose_stack_item( si_p );
	}
	
	n	= i;

	stack_rcopy( r_p, s_p );
	
	for ( i = 0; i < n; i++ )
	{
		si_p	= pop_item( s_p );
		push_item( si_p->item_p, si_p->type, NULL );
		dispose_stack_item( si_p );
	}
	
	stack_rcopy( s_p, r_p );

	for ( i = 0; i < n; i++ )
	{
		si_p	= pop_item( s_p );
		push_item( si_p->item_p, si_p->type, NULL );
		dispose_stack_item( si_p );
	}
	
	dispose_stack( s_p );
	dispose_stack( r_p );
#endif
}

void command_swap( string_object *src_p )
{
	stack_item	*s1;
	stack_item	*s2;

	if ( !(s1	= pop_item( NULL )) )
		return;

	if ( !(s2	= pop_item( NULL )) )
	{
		push_item( s1->item_p, s1->type, NULL );
		return;
	}
	
	push_item( s1->item_p, s1->type, NULL );
	push_item( s2->item_p, s2->type, NULL );

	dispose_stack_item( s1 );
	dispose_stack_item( s2 );
}



void command_rot( string_object *src_p )
{
	stack_rotate( NULL );
}


void command_reverse( string_object *src_p )
{
	stack_reverse();
}

void command_depth( string_object *src_p )
{
	push_i( stack_pointer( NULL ) );
}


void command_undo( string_object *src_p )
{
	cprintf( ERROR, CONT, "\"undo\" command is not supported in this version.\n" );
//	stack_undo();
}






void command_aa( string_object *src_p )
{
	if ( 1 >= stack_pointer( NULL ) )
		return;
	
	while ( 1 < stack_pointer( NULL ) )
		four_operations( OP_ADD );
}


void command_ma( string_object *src_p )
{
	if ( 1 >= stack_pointer( NULL ) )
		return;

	while ( 1 < stack_pointer( NULL ) )
		four_operations( OP_MUL );
}


void command_help( string_object *src_p )
{
	cprintf( NORM, CONT, "help command\n" );
}


void command_quit( string_object *src_p )
{
	g_cafe_mode.quit.value	= TRUE;
}


void command_beep( string_object *src_p )
{
	putchar( '\7' );
}


void command_print( string_object *src_p )
{
	controls_print( src_p );
}


void command_format_print( string_object *src_p )
{
	ui_format_print();
}


void command_mode( string_object *src_p )
{
//	mode_controls();
}


void command_keywd( string_object *src_p )
{
	key_list();
}


void command_history( string_object *src_p )
{
	char	*s;

	if ( NULL == (s		= pop_s()) )
		return;

	key_register_history( s );
	
	dispose_string_object( s );
}


void command_time( string_object *src_p )
{
	time_t	now;
	
	time( &now );
	push_i( (int)now );
}


void command_time_convert( string_object *src_p )
{
	time_t		now;
	struct tm	*date;
	char		s[ 80 ];

	now		= (time_t)pop_i();
	
	date	= localtime( &now );
	strftime( s, 80, g_cafe_mode.format_timeconvert.str, date );
	
	push_s( s );
}


void command_ptime( string_object *src_p )
{
	push_i( clock() );
}


void command_glue( string_object *src_p )
{
	string_object	s2;
	string_object	s1;
	string_object	s;
	string_object	tmp;
	
	if ( !(s2	= pop_s()) )
		return;
		
	if ( !(s1	= pop_s()) )
	{
		push_s( s2 );
		dispose_string_object( s2 );
		return;
	}	
	
	if ( NULL == (tmp	= (string_object)alloca( strlen( s1 ) + strlen( s2 ) + 1 )) )
	{
		cprintf( ERROR, CONT, "alloca @ command_glue\n" );
		return;
	}
	
	strcpy( tmp, s1 );
	strcat( tmp, s2 );
	
	dispose_string_object( s2 );
	dispose_string_object( s1 );

	if ( NULL == (s	= make_string_object( tmp, -1 )) )
	{
		cprintf( ERROR, CONT, "make_string_object @ command_glue\n" );
		return;
	}
	
	push_s( s );
}

void command_debugger_enable( string_object *src_p )
{
	debugger_enable();
}


void command_debugger_disable( string_object *src_p )
{
	debugger_disable();
}




void command_pause( string_object *src_p )
{
#if 0
	int		c;

	cprintf( NORM, CONT, "PAUSE (press [Return] key to resume, [a] or [A] for abort.)" );
	c	= getch();
	cprintf( NORM, CONT, "\r                                                           \r" );
	
	if ( (c == 'a') || (c == 'A') )
		g_cafe_mode.function_exe_abort.value	= TRUE;

	cprintf( NORM, CONT, "\r                                 \r" );
#endif

	cprintf( NORM, CONT, "******** debugging mode turned on by \"pause\" command ********\n" );

	debugger_enable();
}


void command_debugger_custom_commmand( string_object *src_p )
{
	debugger_set_custom_command();
}

void command_return( string_object *src_p )
{
	g_cafe_mode.force_function_return.value		= TRUE;
}

void command_list_var( string_object *src_p )
{
	controls_list_var( GLOBAL_VAR );
}


void command_list_std_var( string_object *src_p )
{
	controls_list_var( NORMAL_VAR );
}


void command_edit( string_object *src_p )
{
	string_object	s;
	string_object	new_s;

	if ( !(s	= pop_s()) )
		return;
	
	new_s	= string_quote( s, '\"' );
	
	key_set_pre_given_str( new_s );
	
	dispose_string_object( s );
}

void command_prof_dump( string_object *src_p )
{
	prof_dump();
}


void command_compile( string_object *src_p )
{
	functions_compile( src_p );
}


void command_reload( string_object *src_p )
{
	intial_file_loads();
}


void command_package( string_object *src_p )
{
	file_set_package_name();
}


void command_global_declare( string_object *src_p )
{
	controls_global_variable_declare();
}


void command_private( string_object *src_p )
{
	controls_private();
}

void command_global( string_object *src_p )
{
	controls_global();
}


void command_pkg_remove( string_object *src_p )
{
	controls_package_remove();
}


void command_parent_context( string_object *src_p )
{
	controls_parent_context();
}


void command_restore_context( string_object *src_p )
{
	controls_restore_context();
}


void command_comment( string_object *src_p )
{
	controls_comment( src_p );
}



void command_zzz( string_object *src_p )
{
	cprintf( NORM, CONT, "   === testing purpose command called. ===\n" );
	
//	file_path_name( "~", NULL );

}



void command_s2c( string_object *src_p )
{
	string_to_chars( N_OPTION_OFF );
}


void command_s2cn( string_object *src_p )
{
	string_to_chars( N_OPTION_ON );
}


void command_c2i( string_object *src_p )
{
	char_to_int();
}


void command_i2c( string_object *src_p )
{
	int_to_char();
}


void command_s2w( string_object *src_p )
{
	string_to_token( N_OPTION_OFF );
}


void command_s2wn( string_object *src_p )
{
	string_to_token( N_OPTION_ON );
}


void command_strrev( string_object *src_p )
{
	string_reverse();
}


void command_strlen( string_object *src_p )
{
	string_len();
}


void command_wc( string_object *src_p )
{
	string_wc();
}


void command_versioninfo( string_object *src_p )
{
	ui_version_information();
}


command		g_command_table[]	=	{

//	dummy
		{	"",			command_nop,		"nop" }, 

//	arithmetic operations

		{	"+",		command_add,		"(n1 n2 -- n1+n2) addition"		}, 
		{	"-",		command_sub,		"(n1 n2 -- n1-n2) subtraction"		}, 
		{	"*",		command_mul,		"(n1 n2 -- n1*n2) multiplication"	}, 
		{	"/",		command_dev,		"(n1 n2 -- n1/n2) division"		}, 
		{	"%",		command_modulo,		"(n1 n2 -- n1%n2) modulo"			}, 
		{	"pow",		command_pow,		"(f1 f2 -- f1^f2) power"		}, 
		{	"recipro",	command_recipro,	"(f -- 1/f) reciprocal"	}, 
		{	"para",		command_para,		"(f1 f2 -- (f1+f2)/(f1*f2))parallel {f} : (X+Y)/(X*Y)"	}, 
		{	"log",		command_log,		"(f -- log(f)) log"				}, 
		{	"logt",		command_logt,		"(f -- logt(f)) log (10 base)"		}, 
		{	"sin",		command_sin,		"(f -- sin(f)) sin"				}, 
		{	"cos",		command_cos,		"(f -- cos(f)) cos"				}, 
		{	"tan",		command_tan,		"(f -- tan(f)) tan"				}, 
		{	"asin",		command_asin,		"(f --asin(f)) arc-sin"			}, 
		{	"acos",		command_acos,		"(f --acos(f)) arc-cos"			}, 
		{	"atan",		command_atan,		"(f --atan(f)) arc-tan"			}, 

//	bit operations

		{	"nleft",	command_nleft,		"(i1 i2 -- i1<<i2) bit shift left"		}, 
		{	"nright",	command_nright,		"(i1 i2 -- i1>>i2) bit shift right"		}, 
		{	"and",		command_and,		"(i1 i2 -- i1&i2) bit shift right"		}, 
		{	"or",		command_or,			"(i1 i2 -- i1|i2) bit shift right"		}, 
		{	"exor",		command_exor,		"(i1 i2 -- i1^i2) bit shift right"		}, 
		{	"invert",	command_not,		"(i -- ~i) bit shift right"		}, 

//	type conversions

		{	"trunc",	command_trunc,		"(n -- (int)n) truncation : convert to integer"}, 
		{	"float",	command_float,		"(n -- (float)n) convert to float"}, 
		{	"string",	command_string,		"(n -- s) string : convert to string"}, 
		{	"quote",	command_quote,		"(s -- s) string : add quote"}, 
		{	"squote",	command_squote,		"(s -- s) string : add quote"}, 

//	type detection

		{	"type",		command_type,		"( -- s) type : returns type identifier INTEGER, FLOAT, STRING, STACK"}, 

//	constants

		{	"pi",		command_pi,			"( -- f) pi constant : circle ratio" }, 
		
//	functions

		{	":",		command_function_define,	"( -- ) function define : \": [name] ([body]) ;\""	}, 
		{	";",		command_nop,				"( -- ) function difinition terminator"}, 
		{	"forget",	command_function_forget,	"( -- ) function delete"	},
		{	"frename",	command_function_rename,	"( -- ) function rename"	},
		{	"fdelete",	command_function_remove,	"( -- ) function rename"	},
		{	"freplace",	command_function_replace,	"( -- ) function name replaced by body"	},

		{	"fself",	command_function_self,		"( -- s) push self function name"		},
		{	"fbody",	command_function_body,		"( -- s) push function body"		},		
		
		{	"fstack",	command_function_list,		"( -- ) shows function stack"		},
		{	"fedit",	command_fshow,				"( -- ) edit function"	},
		{	"fisdef",	command_fisdef,				"( -- ) check if functiondefined"	},

		{	"limitrecursive",	command_limitrecursive,		"( -- ) changing recursive call limit"	},

//	controls
		{	"times",	command_times,				"(i -- ) loop : \"X times [token]\" : execute token X times"}, 
		{	"while",	command_while,				"(i -- ) loop : \"X times [token]\" : execute token X times"}, 
		{	"eval",		command_evaluate,			"(s -- ) evaluate X"}, 
		{	"die",		command_die,	"( -- i) terminate execution"		}, 
		{	"if",		command_if,					"( -- ) condition : X if [cond] [token] : execute token, if X is cond" }, 
		{	"ifelse",	command_ifelse,				"( -- ) condition : X if [cond] [token1] [token2] : execute token1/token2, if X is/isn't cond"}, 
		{	"ifp",		command_ifp,				"( -- ) condition : X if [cond] [token] : execute token, if X is cond" }, 
		{	"ifpelse",	command_ifpelse,			"( -- ) condition : X if [cond] [token1] [token2] : execute token1/token2, if X is/isn't cond"}, 

		{	"return",	command_return,				"return from function"	}, 

		{	"<",		command_readvar,			"( -- *) read variable : \"< [var_name]\""	}, 
		{	">",		command_writevar,			"(* -- ) write variable : \"> [var_name]\""	}, 
		{	"variable",	command_global_declare,		"(s-as-variable-name s-as-context -- ) global variable declaring"	}, 
		{	"private",	command_private,			"( -- s-as-current-package-name) get current package name for variable declaration" 	}, 
		{	"global",	command_global,				"( -- "") constant string for global variable declaration"	}, 
		{	"!",		command_exclamation,		"(s -- ) execute in shell : a string executed in shell"	}, 
		{	"!!",		command_double_exclamation,	"(s -- ) execute in shell and get result : a string executed in shell and output will be loaded"	}, 
		{	"package",	command_package,			"(s-as-package-name -- ) set custom package name for function/variable defining"	}, 
		{	"pkg_remove",	command_pkg_remove,		"(s-as-package-name -- ) remove function/variables defined in the package"	}, 
		{	"[",		command_parent_context,		"( -- ) set function context as parent function (caller) context"	}, 
		{	"]",		command_restore_context,	"( -- ) end of \"[\""	}, 
		{	"(",		command_comment,			"( -- ) comment start till next ')'"	}, 

//	file operations

		{	"use",		command_use,				"(s-as-file-name -- ) load a file"	}, 
		{	"put",		command_put,				"(s-as-file-name -- ) saving functions and history"	},  
		{	"save",		command_save,				"(s-as-file-name -- ) saving stack contents"	}, 
		{	"cd",		command_cd,					"(s-as-directory-name -- ) change directory"	}, 

//	stack operations (basic stack operations)

		{	"cl",		command_clear,				"( -- !) stack clearing"}, 
		{	"stack",	command_stack,				"( -- ) stack status"}, 
		{	"pop",		command_pop,				"(* -- ) delete stack top"}, 
		{	"push",		command_push,				"(* -- * *) copy stack item"}, 
		{	"compare",	command_compare,			"(* * -- i) compare stack items. returns TURE if same"}, 
		{	"dup",		command_push,				"(* -- * *) copy stack item"},
		{	"npush",	command_npush,				"copy N pcs stack item"}, 
		{	"mdup",		command_npush,		"copy N pcs stack item"},
		{	"swap",		command_swap,		"swap stack top"}, 
		{	"sort",		command_sort,		"sort stack"}, 
		{	"uniq",		command_uniq,		"remove duplicated item"}, 
		{	"rot",		command_rot,		"rotate stack. top goes to bottom"}, 
		{	"reverse",	command_reverse,	"reverse stack order"}, 
		{	"depth",	command_depth,		"depth of stack"}, 
		{	"undo",		command_undo,		"revert to last stack state"}, 

//	stack operations (user stack handling)

		{	"=new",		command_newstack,	"make a new stack and put on current sack"}, 
		{	"=target",	command_target,		"switch stack"}, 
		{	"=copy",	command_stackcopy,	"copy stack contents"}, 
		{	"=ncopy",	command_stackncopy,	"copy stack contents"}, 
		{	"=parent",	command_parent,	"get parent stack"}, 


//	user utilities

		{	".",		command_dot,		"print stack top"}, 

		{	"quit",		command_quit,		"quit application"}, 
		{	"qq",		command_quit,		"quit application"}, 
		{	"beep",		command_beep,		"beep"}, 
		{	"versioninfo",		command_versioninfo,		"beep"}, 
		{	"print",	command_print,		"print string : \"print [string]\""}, 
		{	"format_print",	command_format_print,		"print string : \"print [string]\""}, 

		{	"mode",		command_mode,		"mode setting"}, 
		{	"keywd",	command_keywd,		"( -- ) keyword list"}, 
		{	"history",	command_history,	""}, 

		{	"help",		command_help,		"help command"}, 

		{	"time",		command_time,		"currnt calender time"	}, 
		{	"timeconvert",		command_time_convert,	"convert calender time" }, 
		{	"ptime",	command_ptime,		"currnt processor time"	}, 
		{	"glue",		command_glue,		"cat string {s} : \"XY\""	}, 

		{	"pause",	command_pause,		"pause the execution, drop into debugging mode"	}, 
		{	"debugger_cust_key",	command_debugger_custom_commmand,	"execution pause (for function execution)"	}, 
		{	"debugger_enable",		command_debugger_enable,			"debugger_enable"	}, 
		{	"debugger_disable",		command_debugger_disable,			"debugger_disable"	}, 
		{	"vl",		command_list_var,	"listing variables"	}, 
		{	"vls",		command_list_std_var,	"listing normal variables"	}, 

		{	"edit",		command_edit,		"edit a string"	}, 
		
		{	"prof_dump",	command_prof_dump,		"dump profiler data"	}, 
		{	"compile",	command_compile,		"compile"	}, 
	
		{	"reload_init_files",	command_reload,		"reload initializing files"	}, 
	
	
//	integrated calc

		{	"tax",		command_tax,		"calc tax"}, 
		{	"aa",		command_aa,			"add all"}, 
		{	"ma",		command_ma,			"mul all"}, 

//	string operation
		{	"s2c",		command_s2c,		"disassemble into characters"}, 
		{	"s2cn",		command_s2cn,		"disassemble into characters with N"}, 
		{	"s2w",		command_s2w,		"disassemble into words (tokens)"}, 
		{	"s2wn",		command_s2wn,		"disassemble into words (tokens) with N"}, 
		{	"c2i",		command_c2i,		"convert character to ASCII index"}, 
		{	"i2c",		command_i2c,		"convert ASCII index to character"}, 
		{	"strlen",	command_strlen,		"length of string"}, 
		{	"strwc",	command_wc,			"word count"}, 
		{	"strrev",	command_strrev,		"reverse string"}, 
//		{	"edit",		command_stringedit,	"( -- ) edit stacktop"	},
		
		{	"zzz",		command_zzz,		""}, 
					
									};


int try_token( char *key, char **src_p )
{
	keywd	*kwp;
	int		flag_force_command		= FALSE;
	int		flag_function_expansion	= FALSE;
	int		flag_help_request		= FALSE;
	
	if ( *key == '\0' )
	{
		cprintf( ERROR, CONT, "no command specified\n" );
		return ( FALSE );
	}

	switch ( *key )
	{
		case '?' : 
			flag_help_request		= TRUE;
			key++;
			break;
		case '@' : 
			if ( *(key + 1) == '@' )
			{
				flag_function_expansion	= TRUE;
				key		+= 2;
			}
			else
			{
				flag_force_command		= TRUE;
				key++;
			}
			break;
	}

	if ( !(*key) )
		return ( FALSE );
	
#define	NEW_KEYWORD_FINDING_METHOD
#ifdef NEW_KEYWORD_FINDING_METHOD

	if ( !(kwp	= find_keyword( key )) )
		return ( FALSE );

#else

	kwp		= g_keywd;

	while ( kwp )
	{
		///cprintf( NORM, CONT, " (%s)", kwp->key );

		if ( !(strcmp( key, kwp->key )) )
			break;
			
		kwp		= kwp->next;
	}


	if ( !kwp )
		return ( FALSE );

#endif

	if ( (kwp->comm_table_index == 0) && flag_force_command )
	{
		kwp		= kwp->next;

		if ( !kwp )
			return ( FALSE );
		
		if ( strcmp( key, kwp->key ) )
			return ( FALSE );
	}
	

	if ( flag_help_request )
	{
		cprintf( NORM, CONT, "help : " );
		cprintf( BOLD, CONT, "%s", kwp->key );

		if ( kwp->comm_table_index == 0 )
		{
			cprintf( NORM, CONT, " (function)\n" );
			cprintf( BOLD, CONT, "  :%s %s;\n", kwp->key, (kwp->function_ptr)->body );
		}
		else
		{
			cprintf( BOLD, CONT, ", @\n  %s\n", (g_command_table[ kwp->comm_table_index ]).help );
		}
		return ( TRUE );
	}
	else if ( flag_function_expansion )
	{
		if ( kwp->comm_table_index == 0 )
		{
			push_s( (kwp->function_ptr)->body );
			return ( TRUE );
		}
		else
		{
			cprintf( ERROR, CONT, "\"@@\" is valid for function only\n" );
			return ( FALSE );
		}
	}
	else
	{
		if ( kwp->comm_table_index == 0 )
		{
			controls_function_call_prologue( kwp->function_ptr );
			evaluate( (kwp->function_ptr)->body );
			controls_function_call_epilogue();
			
			return ( TRUE );
		}

		((g_command_table[ kwp->comm_table_index ]).buildin_command)( src_p );
			return ( TRUE );
	}
}



void initialize_keywords( void )
{
	keywd		*kwp;
	keywd		*kwp_prev	= NULL;
	int			i;
	
	for ( i = 0; i < sizeof( g_command_table ) / sizeof( command ) ; i++ )
	{
		if ( NULL == (kwp	= create_keywd( g_command_table[i].str )) )
		{
			cprintf( ERROR, CONT, "@initialize_keywords (create_keywd)\n" );
			exit( 1 );
		}
		
		kwp->comm_table_index	= i;
		kwp->function_ptr		= NULL;

		insert_key( kwp_prev, kwp );

		kwp_prev	= kwp;
	}


	sort_kw( g_keywd );

	control_add_mode_name();
#if 0

	kwp		= kwp_prev;
	
	while ( kwp )
	{
		cprintf( NORM, CONT, "(0x%08X) \"%s\"\n", kwp, kwp->key );
		kwp		= kwp->prev;
	}
#endif
}


void key_list( void )
{
	keywd	*kwp;

	kwp		= g_keywd;
	
	while ( kwp )
	{
#if 0 //	2008-Feb-01  modified 

		cprintf( NORM, CONT, "(0x%08X) %s [%d]\n", kwp, kwp->key, kwp->comm_table_index );

#else

		push_s( kwp->key );

#endif
		kwp		= kwp->next;
	}
}


keywd *create_keywd( char *key )
{
	keywd	*kwp;

	if ( NULL == (kwp	= (keywd *)malloc( sizeof( keywd ) )) )
		return ( NULL );
	
	kwp->key	= make_string_object( key, -1 );
	


	add_to_hash_table( kwp );

	return ( kwp );
}


void dispose_keywd( keywd *kwp )
{
	remove_from_hash_table( kwp->key );
	dispose_string_object( kwp->key );
	free( kwp );
}


#define		HASH_TABLE_SIZE		8191

keywd *g_kw_hash_table[ HASH_TABLE_SIZE ];


void add_to_hash_table( keywd *new )
{
	int		hash_index;

	hash_index	= hash( new->key );
	
	new->hash_chain					= g_kw_hash_table[ hash_index ];
	g_kw_hash_table[ hash_index ]	= new;

#if 0
	if ( new->hash_chain )
		cprintf( NORM, CONT, "(%04d)  [%s]oo[%s] hash_chain applied.\n", hash_index, new->key, (new->hash_chain)->key );
#endif
}


void remove_from_hash_table( char *key )
{
	keywd	*kwp;
	keywd	**pkwp;
	int		hash_index;
	
	hash_index	= hash( key );

	pkwp	= &(g_kw_hash_table[ hash_index ]);
	kwp		= g_kw_hash_table[ hash_index ];
	
	while ( kwp && strcmp( key, kwp->key ) )
	{
		pkwp	= &(kwp->hash_chain);
		kwp		= kwp->hash_chain;
	}
	
	*pkwp	= kwp->hash_chain;	
}


void insert_key( keywd *prev, keywd *new )
{
	keywd	*next;
	
	/* insert keyword into keyword chain */
		
	if ( prev )
	{
		next		= prev->next;
		prev->next	= new;
	}
	else
	{
		if ( g_keywd )
		{
			next		= g_keywd;
		}
		else
		{
			next		= NULL;
			g_keywd		= new;
		}
	}
	
	if ( next )
		next->prev	= new;

	new->prev	= prev;
	new->next	= next;
}

#define		USE_HASH_TABLE_TO_FIND


keywd *find_keyword( char *key )
{
	keywd	*kwp;

	kwp		= g_kw_hash_table[ hash( key ) ];
	
	while ( kwp && strcmp( key, kwp->key ) )
		kwp		= kwp->hash_chain;

	return ( kwp );
}


int hash( string_object s )
{
	int		h	= 0;
	
	while ( *s )
		h	= (h * 137 + *s++) % HASH_TABLE_SIZE;

	return ( h );
}




void sort_kw( keywd *kwp_base )
{
	keywd	*kwp;
	keywd	**kwp_array;	
	int		length	= 0;
	int		i;
	int		j;
	
	//	check_length
	
	kwp		= kwp_base;
	while ( kwp )
	{
		length++;
		kwp		= kwp->next;
	}

	//	allocate array area

	if ( NULL == (kwp_array	= (keywd **)malloc( sizeof( keywd * ) * length )) )
	{
		cprintf( ERROR, CONT, "@initialize_keywords (malloc : 3)\n" );
		exit( 1 );
	}

	//	map into array

	kwp		= kwp_base;
	for ( i = 0; i < length; i++ )
	{
		kwp_array[ i ]	= kwp;
		kwp		= kwp->next;
	}
	
	//	buble sort  (^_^;
	
	for ( i = 0; i < length; i++ )
	{
		for ( j = 1; j < (length - i); j++ )
		{
			if ( 0 > strcmp( (kwp_array[ j ])->key, (kwp_array[ j - 1 ])->key ) )
			{
				kwp					= kwp_array[ j     ];
				kwp_array[ j     ]	= kwp_array[ j - 1 ];
				kwp_array[ j - 1 ]	= kwp;
			}
		}
	}

	//	check

#if 0
	for ( i = 0; i < length; i++ )
		cprintf( NORM, CONT, "(0x%08X) \"%s\"\n", kwp_array[ i ], kwp_array[ i ]->key );
#endif

	remap( kwp_base, kwp_array, length );
	
	//	bye

	free( kwp_array );
}


void remap( keywd *kwp_base, keywd **kwp_array, int length )
{
	int		i;
	
	/*	
	 *	"insert_key()" function should not be used. 
	 *	all link information need to be overwritten. 
	 */

	kwp_base	= kwp_array[ 0 ];
	
	for ( i = 1; i < length; i++ )
	{
		(kwp_array[ i - 1 ])->next	= kwp_array[ i    ];
		(kwp_array[ i     ])->prev	= kwp_array[ i - 1];
	}

	(kwp_array[ length - 1 ])->next	= NULL;
}


void add_function_to_key( function *fnc_ptr )
{	
	keywd	*kwp;
	keywd	*insert_point;
	
	if ( NULL == (kwp	= create_keywd( fnc_ptr->name )) )
	{
		cprintf( ERROR, CONT, "@initialize_keywords (create_keywd)\n" );
		exit( 1 );
	}
	
	insert_point	= find_key_insert_point( kwp->key );
	
	kwp->comm_table_index	= 0;
	kwp->function_ptr		= fnc_ptr;
	
	insert_key( insert_point, kwp );
}


void add_keywd_to_key( char *key )
{	
	keywd	*kwp;
	keywd	*insert_point;
	
	insert_point	= find_key_insert_point( key );

	if ( !strcmp( (insert_point->next)->key, key ) )
		return;

	if ( NULL == (kwp	= create_keywd( key )) )
	{
		cprintf( ERROR, CONT, "@initialize_keywords (create_keywd)\n" );
		exit( 1 );
	}
	
	kwp->comm_table_index	= -1;
	kwp->function_ptr		= NULL;
	
	insert_key( insert_point, kwp );
}


void remove_function_from_key( function *fnc_ptr )
{	
	keywd	*target_key;
	keywd	*insert_point;
	
	insert_point	= find_key_insert_point( fnc_ptr->name );
	
	if ( insert_point )
		target_key	= insert_point->next;	/*	"insert_point->next == NULL" should not be happen	*/
	else
		target_key	= g_keywd;
	
	if ( target_key->prev )
		(target_key->prev)->next	= target_key->next;
	else
		g_keywd	= target_key->next;
	
	if ( target_key->next )
		(target_key->next)->prev	= target_key->prev;

//	remove_from_hash_table( target_key->key );

	dispose_keywd( target_key );
}


void remove_keywd_from_key( char *key )
{	
	keywd	*target_key;
	keywd	*insert_point;
	
	insert_point	= find_key_insert_point( key );
	
	if ( insert_point )
		target_key	= insert_point->next;	/*	"insert_point->next == NULL" should not be happen	*/
	else
		target_key	= g_keywd;
	
	if ( target_key->prev )
		(target_key->prev)->next	= target_key->next;
	else
		g_keywd	= target_key->next;
	
	if ( target_key->next )
		(target_key->next)->prev	= target_key->prev;

//	remove_from_hash_table( target_key->key );

	dispose_keywd( target_key );
}

keywd *find_key_insert_point( char *key )
{
	keywd	*kwp;
	keywd	*kwp_p	= NULL;
	int		state	= 0;

	kwp		= g_keywd;
	
	while ( kwp )
	{
//		if ( 0 > (state	= strcmp( key, kwp->key )) )
		if ( !(0 < (state	= strcmp( key, kwp->key ))) )
			break;
			
		kwp_p	= kwp;
		kwp		= kwp->next;
	}
	
	return ( kwp_p );
}


typedef	struct keywd_list	{
								char	*string;
								int		type;
							}
							keywd_list;


#include	<ncurses.h>

int find_n_match_candidate( char *key )
{
	int		length;
	int		key_length;
	keywd	*kwp;
	keywd	*other_kwp;

	length	= strlen( key );
	
	kwp		= g_keywd;
	
	//	find function/command name that matchs to the given key string
	
	while ( kwp )
	{
		if ( !strncmp( key, kwp->key, length ) )	//	first match found
			break;
			
		kwp		= kwp->next;
	}

#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif
    
	if ( kwp == NULL )	//	no matching key
		return ( 0 );
		
	{
		int			max_length;
		int			common_length;
		keywd_list	*list;
		char		fmt[ 80 ];
		int			candidates	= 1;
		int			row;
		int			col;
		int			i;
		int			j;
		int			pos;
		int VARIABLE_IS_NOT_USED height;    //  volatile is to suppress warning
		int			width;

		other_kwp	= kwp->next;

		while ( !strncmp( key, other_kwp->key, length ) )
		{
			candidates++;
			
			other_kwp	= other_kwp->next;
			
			if ( !other_kwp )
				break;
		}
		
		if ( candidates == 1 )
		{
			strcpy( key, kwp->key );

			if ( *(key + strlen(key) - 1) != '/' )
				strcat( key, " " );
				
			return ( 1 );
		}
		
		if ( NULL == (list	= (keywd_list *)alloca( sizeof( keywd_list ) * candidates )) )
		{
			cprintf( ERROR, CONT, "alloca @ find_n_match_candidate\n" );
			exit ( 1 );
		}

		other_kwp	= kwp;
		max_length	= length;

		for ( i = 0; (i < candidates) && other_kwp;  )
		{
			if ( !(*key) && ((*(other_kwp->key) == '>') || (*(other_kwp->key) == '<')) )
			{
			}
			else
			{
				list[ i ].string	= other_kwp->key;
				list[ i ].type		= other_kwp->comm_table_index;
				
				i++;
			}
			
			key_length	= strlen( other_kwp->key );
			max_length	= (max_length < key_length) ? key_length : max_length;
			
			other_kwp	= other_kwp->next;
		}
		
		candidates	= i;
		
		getmaxyx( stdscr, height, width );

		--width;

		max_length	= (max_length < 10) ? 10 : max_length;
		
		col			= width / (max_length + 4);
		row			= (candidates / col) + ((candidates % col) ? 1 : 0);
		
		sprintf( fmt, "%%c%%-%ds   ", max_length );
		cprintf( NORM, CONT, "\n" );

		for ( i = 0; i < row; i++ )
		{
			for ( j = 0; j < col; j++ )
			{
				pos		= i + (j * row);
				
				if ( pos < candidates )
					cprintf( (list[ pos ].type < 0) ? BOLD : NORM, CONT, fmt, (list[ pos ].type > 0) ? '@' : ' ', list[ pos ].string );
			}
			cprintf( NORM, CONT, "\n" );
		}
		
		common_length	= max_length;
		
		while ( strncmp( list[ 0 ].string, list[ candidates - 1 ].string, common_length ) )
			--common_length;
	
		strncpy( key, list[ 0 ].string, common_length );
		*(key + common_length)	= '\0';

		return ( candidates );
	}
}



void *find_token( char *key, char **src_p )
{
	keywd	*kwp;
	
	if ( *key == '\0' )
	{
		cprintf( ERROR, CONT, "no command specified\n" );
		return ( NULL );
	}

	switch ( *key )
	{
		case '?' : 
		case '@' : 
			return ( NULL );
			break;
	}

	if ( !(kwp	= find_keyword( key )) )
		return ( NULL );
	
	return ( (void *)kwp );
}

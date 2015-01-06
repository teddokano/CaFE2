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
#include	<ncurses.h>


#include	"stack.h"
#include	"functions.h"
#include	"controls.h"
#include	"fileop.h"
#include	"debugger.h"
#include	"ui.h"
#include	"string.h"
#include	"command_table.h"

enum	{
			VAR_READ,
			VAR_WRITE,
		};


typedef struct	_variable		variable;
struct	_variable	{
						char		*name;
						stack_item	*item;
						variable	*prev;
					};


typedef struct _context		context;
struct	_context	{
						char		*package_name;
						variable	*var;
						context		*next;
						context		*prev;
					};


context		g_context_stack			=	{
											NULL,
											NULL,
											NULL
										};
context		g_global_base			=	{
											"",
											NULL,
											NULL
										};


context		*g_context_stack_ptr	= &g_context_stack;
context		*g_context_temporary	= NULL;
context		*g_global_base_ptr		= &g_global_base;


static void		controls( int times, char **src_p );

static double	check_stack_top( void );
static void		dispose_variable( variable *vp );
variable		*find_var( char *s, context *cp );
context			*find_context( char *s );
void			variable_access( char **src_p, char mode );
context			*create_func_context( context **targ_cp_p, char *name_p );
void			dispose_func_context( context **context_ptr_ptr );

void			make_a_variable( variable **vpp, context *cp, char *varname );

void list_var( context *cp );
context *get_current_context( void );

void mode_controls( string_object label, int rw );


void controls_if( char **src_p, int els )
{
	char		*condition;
	double		value;
	int			flag;
	int			macro_option_used	= FALSE;

	if ( NULL == (condition	= get_next_token( src_p, NORMAL )) )
	{
		cprintf( ERROR, CONT, "no \"condition\" for \"if\"\n" );
		return;
	}
	
#if 1
//cprintf( NORM, CONT, "(*condition = %s)\n", condition );

	if ( *condition == '?' )
	{
		dispose_string_object( condition ); 
	
		if ( NULL == (condition	= get_next_token( src_p, NORMAL )) )
		{
			cprintf( ERROR, CONT, "no expression after \"?\" of \"if\"\n" );
			return;
		}
		
		evaluate( condition );
		
		macro_option_used	= TRUE;
	}
#endif

	if ( els & IF_WITH_POP )
		value	= pop();		
	else
		value	= check_stack_top();
	
	switch ( *condition )
	{
		case 'p' : 
			flag	= (0 < value) ? TRUE : FALSE;
			break;
		case 'n' : 
		case 'm' : 
			flag	= (value < 0) ? TRUE : FALSE;
			break;
		case 'z' : 
			flag	= (value == 0.00) ? TRUE : FALSE;
			break;
		default : 
			flag	= (value != 0.00) ? TRUE : FALSE;
			break;
	}
	
	dispose_string_object( condition );
	
	if ( els & IFELSE )
	{
		controls(  flag, src_p );
		controls( !flag, src_p );
	}
	else
	{
		controls( flag, src_p );
	}
	
	if ( macro_option_used )
	{
		pop();
	}
}


static double check_stack_top( void )
{	
	stack_item	*si_p;
	double		value;

	if ( NULL == (si_p	= peep_stack_item( NULL )) )
	{
//		cprintf( ERROR, CONT, "no stack item for \"if\"\n" );
		return ( 0.00 );
	}

	switch ( si_p->type )
	{
		case INTEGER :
			value	= (double)(*((long *)(si_p->item_p)));
			break;
		case FLOAT :
			value	= *((double *)(si_p->item_p));
			break;
		case STRING :
			if ( *((char *)(si_p->item_p)) )
				value	= 1.00;
			else
				value	= 0.00;
			break;
		case STACK : 
			value	= 1;
			break;
		default : 
			//	unknown type
			value	= 0;
			break;
	}
	
	return ( value );
}


void controls_times( char **src_p )
{
	long	repetition;
	
	repetition	= pop_i();
	
	repetition	= (repetition < 0) ? 0 : repetition;
	
	controls( repetition, src_p );
}


void controls_while( char **src_p )
{
	controls( -1, src_p );
}


static void controls( int times, char **src_p )
{
	char	*token;
	int		tmp_fnc;
	int		gbglevel;
	
	int		i;
	
	token	= get_next_token( src_p, NORMAL );

	if ( !token )
	{
		cprintf( ERROR, CONT, "no \"token\" to evaluate\n" );
		return;
	}

	if ( *token == ':' )
	{
		tmp_fnc		= function_define( src_p );
		
		if ( tmp_fnc == TEMPORAL_FUNC_DEFINED )
		{
			dispose_string_object( token );
			token	= function_get_temporal_function_body();
		}
	}
	
	if ( times < 0 )
	{
		while ( (int)check_stack_top() )
		{
			if ( g_cafe_mode.function_exe_abort.value )
				break;
			
			gbglevel	= debugger_get_debugger_exec_level();
			evaluate( token );
			
			if ( gbglevel )
				debugger_set_debugger_exec_level( gbglevel );
		}
	}
	else
	{
		for ( i = 0; i < times; i++ )
		{
			if ( g_cafe_mode.function_exe_abort.value )
				break;

			gbglevel	= debugger_get_debugger_exec_level();
			evaluate( token );
			
			if ( gbglevel )
				debugger_set_debugger_exec_level( gbglevel );
		}
	}
	
	dispose_string_object( token );
}


void controls_evaluate( void )
{
	char	*s;

	if ( NULL == (s		= pop_s()) )
		return;

	evaluate( s );
	
	dispose_string_object( s );
}


void controls_print( char **src_p )
{
	char	*str;
	
	str		= get_next_token( src_p, NORMAL );
	
	if ( !str )
	{
		cprintf( ERROR, CONT, "no \"string\" to print\n" );
		return;
	}

	ui_user_print( str );

	dispose_string_object( str );
}


void controls_execution_in_shell( int mode, char **src_p )
{
	char			command_str[ MAX_SRC_LINE_LENGTH ];
	char			tmp[ MAX_SRC_LINE_LENGTH ];
	string_object	r	= NULL;
	FILE			*fp;
	char			*str;


	if ( !(str	= pop_s()) )
		return;
	
	sprintf( command_str, "%s 2> /dev/null", str );

	if ( NULL == (fp	= popen( command_str, "r" )) )
	{
		cprintf( ERROR, CONT, "popen @ load_file_names" );
		return;
	}
	
	while ( 1 )
	{
		fgets( tmp, MAX_SRC_LINE_LENGTH, fp );

		if ( feof( fp ) )
			break;
		
		if ( mode == AS_COMMAND )
		{
			cprintf( BOLD, NORM, "%s", tmp );
			
			if ( g_cafe_mode.interactive_mode.value )
				refresh();
		}
		else if ( mode == AS_PIPE )
		{
//			evaluate( tmp );


#ifdef LINE_BY_LINE
			if ( '\n' == *(tmp + strlen( tmp ) - 1) )
				*(tmp + strlen( tmp ) - 1)	= '\0';
				
			push_s( tmp );
#else
			r	= add_string( r, tmp, WITHOUT_SPACE );
#endif
		}
	}

#ifdef LINE_BY_LINE
#else
	if ( r )
	{
		push_s( r );
		dispose_string_object( r );
	}
#endif
	
	
	
	dispose_string_object( str );
	pclose( fp );
}






typedef struct	_package_name_chain		package_name_chain;
struct	_package_name_chain	{
								string_object		name;
								package_name_chain	*prev;
							};


package_name_chain	g_executing_package		=	{ "", NULL };
package_name_chain	*g_executing_package_p	=	&g_executing_package;


void controls_set_executiong_package( char *name )
{
	if ( name )
	{
		package_name_chain	*newp;

		if ( !(newp	= (package_name_chain *)malloc( sizeof( package_name_chain ) )) )
			cprintf( ERROR, ABORT, "malloc error @ controls_set_executiong_package()" );
		
		newp->name	= name;
		newp->prev	= g_executing_package_p;
		g_executing_package_p	= newp;
	}
	else
	{
		package_name_chain	*oldp;

		oldp	= g_executing_package_p;
		g_executing_package_p		= oldp->prev;
		free( oldp );
	}
}




context *create_func_context( context **targ_cp_p, char *name_p )
{
	context		*cp;
	
	if ( NULL == (cp	= (context *)malloc( sizeof ( context ) )) )
		cprintf( NORM, ABORT, "error @ controls_push_func_context\n" );

	cp->prev			= *targ_cp_p;
	cp->next			= NULL;
	
	if ( cp->prev )
		(cp->prev)->next	= cp;
	
	cp->var				= NULL;
	*targ_cp_p			= cp;
	
	if ( !name_p )
		cp->package_name	= NULL;
	else
	{
		if ( NULL == (cp->package_name	= make_string_object( name_p, -1 )) )
			cprintf( NORM, ABORT, "error @ create_func_context\n" );
	}

//cprintf( NORM, CONT, "create cp-prev 0x%08X  cp 0x%08X\n", (unsigned)cp->prev, (unsigned)cp );
//cprintf( NORM, CONT, "cp->package_name=\"%s\"\n", cp->package_name );

	return ( cp );
}


void controls_package_remove( void )
{
	context		*cp;
	char		*ctxt_name_p;

	if ( NULL == (ctxt_name_p	= pop_s()) )
		return;
	
	if ( NULL == (cp	= find_context( ctxt_name_p )) )
		cprintf( ERROR, CONT, "no package to remove\n" );

	dispose_func_context( &cp );

	//	REMOVE FUNCTIONS
	function_remove_by_package_name( ctxt_name_p );
	
	dispose_string_object( ctxt_name_p );
}


void dispose_func_context( context **context_ptr_ptr )
{
	context		*cp;

	cp					= *context_ptr_ptr;
	
	if ( !cp )
		return;

	if ( cp->prev )
	{
		(cp->prev)->next	= cp->next;
		*context_ptr_ptr	= cp->prev;
	}
	
	if ( cp->next )
		(cp->next)->prev	= cp->prev;
	
	if ( cp->package_name )
		dispose_string_object( cp->package_name );
		
	if ( cp->var )
		dispose_variable( cp->var );
		
	free( cp );
	
//	*context_ptr_ptr	= NULL;
//	*context_ptr_ptr	= &g_context_stack;
}


void dispose_variable( variable *vp )
{
	if ( vp->prev )
		dispose_variable( vp->prev );
	
	if ( vp->item )
		dispose_stack_item( vp->item );

	dispose_string_object( vp->name );
	free( vp );
}


void controls_make_base_context( void )
{
	if ( NULL == create_func_context( &g_context_stack_ptr, NULL ) )
		cprintf( NORM, ABORT, "error @ controls_make_base_context\n" );
}


void controls_dispose_base_context( void )
{
	dispose_func_context( &g_context_stack_ptr );
}


void controls_read_variable( char **src_p )
{
	variable_access( src_p, VAR_READ );
}


void controls_write_variable( char **src_p )
{
	variable_access( src_p, VAR_WRITE );
}


variable *find_var( char *s, context *cp )
{
	variable	*vp;

	if ( !cp )
		return ( NULL );

	vp	= cp->var;

	while ( vp )
	{
		if ( !(strcmp( s, vp->name )) )
			break;

		vp		= vp->prev;
	}
	
	return ( vp );
}


void controls_list_var( int global_mode )
{
	context			*cp;
	string_object	s;

	if ( global_mode )
	{
		ui_suppress_cprintf( SUPPRESS_WARN );	//	eliminating "stack empty" message 
		s	= pop_s();
		ui_suppress_cprintf( SUPPRESS_OFF );	
	}
	else
	{
		s	= NULL;
	}
	
	if ( !s )
	{
		cp	= get_current_context();
		cprintf( BOLD, CONT, "    valiable list (function local)\n", s );
	}
	else
	{
		if ( !(cp	= find_context( s )) )
		{
			cprintf( WARN, CONT, "no context (\"%s\") found for variable listing\n", s );
			dispose_string_object( s );
			return;
		}
	
		cprintf( BOLD, CONT, "    valiable list (%s)\n", *s ? s : "global" );

	}

	list_var( cp );
	
	if ( s )
		dispose_string_object( s );
}


void list_var( context *cp )
{
	variable		*vp;
	string_object	s;

	if ( !cp )
		return;

	vp	= cp->var;

	while ( vp )
	{
		if ( vp->item )
		{
			s	= ui_stack_item_to_string( vp->item, -1, STRING_WITH_QUOTE );
			cprintf( BOLD, CONT, "        %-20s == %s\n", vp->name, s );
			dispose_string_object( s );
		}
		
		vp		= vp->prev;
	}
}


context *find_context( char *s )
{
	context		*cp;

	cp	= g_global_base_ptr;

	while ( cp )
	{
		if ( !(strcmp( s, cp->package_name )) )
			break;

		cp		= cp->prev;
	}

	return ( cp );
}


void controls_global_variable_declare( void )
{
	variable	*vp;
	context		*cp;
	char		*pkg_name_p;
	char		*var_name_p;
	
	if ( NULL == (pkg_name_p	= pop_s()) )
		return;

	if ( NULL == (var_name_p	= pop_s()) )
		return;
	
	if ( NULL == (cp	= find_context( pkg_name_p )) )
	{
		if ( NULL == (cp	= create_func_context( &g_global_base_ptr, pkg_name_p )) )
			cprintf( NORM, ABORT, "error @ global_variable_declare\n" );
	}
	else
	{
//		cprintf( NORM, CONT, "cp exists already\n" );
	}

	if ( NULL != (vp	= find_var( var_name_p, cp )) )
	{
		cprintf( WARN, CONT, "the variable \"%s\" has been already defined\n", var_name_p );

		dispose_string_object( var_name_p );
		dispose_string_object( pkg_name_p );
		
		return;
	}
	
	make_a_variable( &vp, cp, var_name_p );

	dispose_string_object( var_name_p );
	dispose_string_object( pkg_name_p );
}


void make_a_variable( variable **vpp, context *cp, char *varname )
{
	if ( NULL == (*vpp	= (variable *)malloc( sizeof ( variable ) )) )
		cprintf( NORM, ABORT, "error @ variable_access\n" );
	
	if ( NULL == ((*vpp)->name	= make_string_object( varname, -1 )) )
		cprintf( NORM, ABORT, "error @ variable_access\n" );
	
	(*vpp)->prev	= cp->var;
	(*vpp)->item	= NULL;
	cp->var			= *vpp;
	
	{
		string_object s;

		s	= make_string_object( ">", -1 );
		s	= add_string( s, varname, WITHOUT_SPACE );

		add_keywd_to_key( s );
		
		*s	= '<';

		add_keywd_to_key( s );
		
		dispose_string_object( s );
	}

}


void variable_access( char **src_p, char mode )
{
	stack_item	*popped_stack_item_p;
	variable	*vp;
	char		*varname;
	
	if ( !get_current_context() )
		g_context_stack_ptr		= find_context( "" );

	varname		= get_next_token( src_p, NORMAL );

	if ( !varname )
	{
		cprintf( ERROR, CONT, "no \"varname\" to operate\n" );
		return;
	}
	else if ( (*(varname + 0) == '$') && (*(varname + 1) == '$') ) 
	{
		mode_controls( varname, mode );
		return;
	}

	//	1st, try to find the variable in function
	
	vp	= find_var( varname, get_current_context() );

	//	2nd, try to find the variable which was defined in package
	
	if ( !vp && g_executing_package_p->name )
		vp	= find_var( varname, find_context( g_executing_package_p->name ) );

	//	3rd, try to find in global
	
	if ( !vp )
		vp	= find_var( varname, find_context( "" ) );

	if ( !vp )
	{
		if (mode == VAR_WRITE)
		{		
			make_a_variable( &vp, get_current_context(), varname );
		}
		else
		{
			cprintf( ERROR, CONT, "no variable available for \"%s\"\n", varname );
			return;
		}
	}
	
	dispose_string_object( varname );

	if ( mode == VAR_WRITE )
	{
		if ( !(popped_stack_item_p	= pop_item( NULL )) )
			return;					//	no instance to copy

		if ( vp->item )
			dispose_stack_item( vp->item );
			
		vp->item	= make_stack_item( popped_stack_item_p->item_p, popped_stack_item_p->type );

		dispose_stack_item( popped_stack_item_p );
	}
	else if ( mode == VAR_READ )
	{
		if ( !(vp->item) )
		{
			cprintf( ERROR, CONT, "no content to read from variable for \"%s\"\n", varname );
			return;	
		}

		push_item( (vp->item)->item_p, (vp->item)->type, NULL );
	}
}


void controls_private( void )
{
	char	*s;
	
	if ( (s	= file_get_loading_file_name()) )
		push_s( s );
	else
		controls_global();
}


void controls_global( void )
{
	push_s( "" );
}


void controls_set_package( char *s )
{
	if ( NULL == find_context( s ) )
	{
		if ( NULL == create_func_context( &g_global_base_ptr, s ) )
			cprintf( NORM, ABORT, "error @ global_variable_declare\n" );
	}
}


void controls_function_call_prologue( function *func )
{
	debugger_push_name_chain( func->name  );
	controls_make_base_context();
	controls_set_executiong_package( func->package_name );
}
		
void controls_function_call_epilogue( void )
{
	controls_set_executiong_package( NULL );
	controls_dispose_base_context();
	debugger_pop_name_chain();
	
	g_cafe_mode.force_function_return.value		= FALSE;
}			


void controls_parent_context( void )
{
	context		*p;

	if ( (p		= get_current_context()) )
		g_context_temporary		= p->prev;
}


void controls_restore_context( void )
{
	g_context_temporary		= NULL;
}


context *get_current_context( void )
{
	if ( g_context_temporary )
		return ( g_context_temporary );
	else
		return ( g_context_stack_ptr );
}


void controls_comment( string_object *s )
{
	string_object	token;
	int				depth	= 1;

	while ( (token	= get_next_token( s, NORMAL )) && depth )
	{
		switch ( *token )
		{
			case '(' : 
				depth++;
				break;
			case ')' : 
				depth--;
				
				if ( !depth )
				{
					if ( token )
						dispose_string_object( token );
					
					return;
				}
				
				break;
			default : 
				break;
		}
	}

	if ( token )
		dispose_string_object( token );
}


cafe_mode		g_cafe_mode			= { 
										{ TRUE,     "$$INTERACTIVE_MODE"	}, 
										{ 0,		"$$GET_STDIN"			}, 
										{ TRUE,     "$$USE_PREFERENCE"		}, 
										{ TRUE,     "$$USE_HISTORY"			}, 
										{ TRUE,     "$$PUT_HISTORY"			}, 
										{ 2000,     "$$RECURSIVE_LIMIT"		}, 
										{ FALSE,    "$$ERROR_DETECTED"		},
										{ 100,		"$$N_HISTORY"			},
										{ FALSE,    "$$QUIT"				},
										{ FALSE,    "$$FUNC_EXE_ABORT"		},
										{ FALSE,    "$$FORCE_FUNCTION_RTN"	},
										{ TRUE,		"$$ONTIME_DISPLAY"	    },
										{ FALSE,	"$$PROMPT$"				,	"CaFE (%b%r%n) %% " },
										{ FALSE,	"$$FORMAT_INT$"			,	"%ld" },
										{ FALSE,	"$$FORMAT_FLOAT$"		,	"%lg" },
										{ FALSE,	"$$FORMAT_TIMECONVERT$"	,	"%T %A, %d-%B-%Y (week %V)" },
										{ SUPPRESS_OFF,	"$$MESSAGE_LEVEL"		},
										{ 0,		"$$PRINT_TARGET"		},
										{ 0,		"$$POST_PRINT_SCREEN$"	,	"" },
										{ 0,		"$$POST_PRINT_STACK$"	,	"" },
										{ 0,		"$$LOOPCOUNT"			},
										{ 0,		"$$LOOP_START"			},
										{ 1,		"$$LOOP_STEP"			},
										{ 0,		"$$DEFAULT_DIRECTORY$"	,	"" },
										{ 0,		"$$SOURCE_FILES_DIRECTORY$"	, SRC_FILE_DIR_PATH }	//	SRC_FILE_DIR_PATH will be replaced by Makefile
									   };



char	**list	= NULL;	//	surpressing warning

void mode_controls( string_object label, int rw )
{
	mode_item		*mip;
	int				i;
	
	mip		= (mode_item *)(&g_cafe_mode);
	
	for ( i = 0; i < (sizeof( g_cafe_mode ) / sizeof( mode_item )); i++ )
	{
		if ( !strcmp( label, mip->name ) )
		{
			if ( *(mip->name + strlen( mip->name ) - 1) == '$' )
			{
				if ( rw == VAR_WRITE )
				{
					char	*s;
				
					if ( !(s	= pop_s()) )
						return;
					
					if ( mip->value )
						dispose_string_object( mip->str );
					
					mip->str	= s;
					mip->value	= TRUE;
				}
				else
					push_s( mip->str );
			}
			
			else
			{
				if ( rw == VAR_WRITE )
					mip->value		= pop_i();
				else
					push_i( mip->value );
			}
			
			break;
		}
		mip++;
	}
	
	if ( i == (sizeof( g_cafe_mode ) / sizeof( mode_item )) )
	{
		mip		= (mode_item *)(&g_cafe_mode);
	
		cprintf( BOLD, CONT, "current mode status\n" );

		for ( i = 0; i < sizeof( g_cafe_mode ) / sizeof( mode_item ); i++ )
		{
			if ( *(mip->name + strlen( mip->name ) - 1) == '$' )
				cprintf( BOLD, CONT, "  %-30s : \"%s\"\n", mip->name, mip->str );
			else
				cprintf( BOLD, CONT, "  %-30s : %d\n", mip->name, mip->value );
			mip++;
		}
	}
}


void control_add_mode_name( void )
{
	mode_item		*mip;
	string_object	s;
	int			i;
	
	mip		= (mode_item *)(&g_cafe_mode);

	for ( i = 0; i < sizeof( g_cafe_mode ) / sizeof( mode_item ); i++ )
	{
		s	= make_string_object( ">", -1 );
		s	= add_string( s, mip->name, WITHOUT_SPACE );

		add_keywd_to_key( s );
		*s	= '<';
		add_keywd_to_key( s );

		dispose_string_object( s );
		mip++;
	}
}


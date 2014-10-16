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

#ifndef		KEY_H
#define		KEY_H



char	*key_input( char *prompt );
int		key_register_history( char *src );
void	history_strings( char **strp, int n );
void	key_set_pre_given_str( char *s );


#endif	//	KEY_H


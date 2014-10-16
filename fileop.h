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

#ifndef		FILEOP_H
#define		FILEOP_H


enum	{
			FILEOP_USE,		//	to load a file
			FILEOP_PUT,		//	to store functions
			FILEOP_SAVE,	//	to store stack
			FILEOP_CHDIR,	//	change directory
		};

enum	{
			LINE_COUNT_RESET,
			LINE_COUNT_INCREMENT,
			LINE_COUNT_STAY,
		};
		
#define		LINEFEED_REPLACED	'\x8'

extern char		*g_loading_file_name_ptr;


int		fileop( char **src_p, int mode );

int		fileop_use_by_name( char *file_name );
int		fileop_put_by_name( char *file_name );
int		fileop_save_by_name( char *file_name );

void	file_handle_path( char *path, int length );

void	file_set_package_name( void );
char	*file_get_loading_file_name( void );



int		file_path_name( char *src, char *trg );

int		file_line_count( int mode );


#endif	//	FILEOP_H


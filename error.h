
#ifndef _ERROR_H
#define _ERROR_H

#define ERR_EXIT(m) (perror(m), exit(EXIT_FAILURE))
#define Check( num,  errObj ) do  \
{							     \
	if ( num )                   \
	{							 \
		ERR_EXIT(errObj);		 \
	}							 \
} while ( 0 )

#endif


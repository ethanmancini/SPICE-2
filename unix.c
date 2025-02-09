/*
 * SCCSID=unix.c 3/15/83
 */
#include <stdlib.h>
#include <sys/times.h>

/* declarations to avoid warnings */
void mcopy(char*, char*, int);
void mclear(char*,int);
char  *itoc( int );

/*
 * loc_ - return the address of arg
 */
unsigned long
loc_( long int *arg )
{
	return( (unsigned long) arg );
}



/*
 * times_ - c routine to call library routine times
 */
void times_( int *iarg )
{
  times( (struct tms *)iarg );
}


#include <time.h>

/*
 * xtime_ - fortran routine for character time
 */
void xtime_( char *chr )
{
	char		*character;
	long		tloc,	scum;
	int		i;

        tloc = time( & scum );
        character = asctime( localtime(& tloc) );
        for( i = 11; i < 19; i++ )
                *chr++ = *( character + i );
}

/*
 * xdate_ - fortran routine for character date
 */
void xdate_( char *chr )
{
	struct	tm	*buffer;
	char		*month,	*day,	*year;
	long		tloc,	scum;

        tloc = time( & scum );
        buffer = localtime( & tloc);
        month = itoc( buffer->tm_mon + 1 );   /* month is zero based */
        while( *month )
                *chr++ = *month++;
        *chr++ = '/';
        day = itoc( buffer->tm_mday );
        while( *day )
                *chr++ = *day++;
        *chr++ = '/';
        year = itoc( buffer->tm_year );
        while( *year )
                *chr++ = *year++;
}
/*
 * itoc
 */
char  *
itoc( int number )
{
  static char string[3];

	/*
	 * make a two digit string from the least significant digits of number
	 */
        string[2] = '\0';
        string[1] = number%10 + '0';
        number /= 10;
        string[0] = number%10 + '0';
        return( string );
}


/*
 * dblsgl - convert a complex double precision array into
 *  a single precision complex array.

 * Note that as written here, this function actually does nothing, it is
 * provide strictly so that the fortran call in spice.f works
 */
void dblsgl_( double *cstar16, int *numwds )
{
	float	*cstar8;
	int	i;

	return;
	cstar8 = (float *) cstar16;
	for ( i = 0; i < (*numwds)/4; i++ ) {
		cstar8[ i ] = cstar16[ 2*i ];
	}
}


#include <stdio.h>
FILE	*rawfile;  /* pointer to raw file  */

static int xargc;    /* number of arguments in UNIX command */
static char **xargv; /* pointer to an array of pointers to
                        arguments in UNIX command line  */

/*
 * Open raw data file.  Return 1 if file is opened,
 *  return 0 if file is not opened
 */
int iopraw_()
{
	int	i;
	char	*filename = NULL;/* name of raw file */

	for ( i=1; i < xargc; i++ ) {
		if ( *xargv[i] == '-' )
			switch ( xargv[i][1] )  {
			case 'r':
				if ( ++i < xargc )
					filename = xargv[i];
				else
					filename = "rawspice";
				break;
			default:
				fprintf( stderr, "SPICE: illegal option -%c - ignored\n",
						xargv[i][1] );
				break;
			}
	}
	if ( filename == NULL )
		return( 0 );
	if  ( (rawfile=fopen( filename, "w" )) == NULL ) {
		fprintf( stderr, "SPICE: unable to open file %s\n", filename );
		fprintf( stderr, "SPICE:  *** program terminated ***\n" );
		exit( 1 );  /* terminate program */
	}
	return( 1 );  /* normal termination */
}
/*
 * Close raw file.
 */
void clsraw_()
{
	fclose( rawfile );
}
/*
 * Write into raw file numwds 16 bit words starting
 *  at location data
 */
void fwrite_( char* data, int *numwds )
{
	fflush( stderr );
	fwrite( data, 2, *numwds, rawfile );
	fflush( rawfile );
}

/*
 * Zero, copy and move for vax unix.
 */
void move_( char * array1, int *index1, char *array2, int *index2, int *length )
{
	array1 += *index1 - 1;
	array2 += *index2 - 1;
	mcopy( array2, array1, *length );
}


/*
  Super obnoxious: they are assuming that ints are 4 bytes, doubles 8,
  complex 16, even though there are supposedly pains taken to handle when
  they aren't.  Then they call "zero4" and friends on all ints.

  On a modern 64 bit system we have 8 byte pointers, and for a variety of
  reasons this makes us need to so 8 byte ints as well (because spice
  is hamfistedly accessing pointer and then storing them in integers).

  So let's fake this out and make "zero4" and the other "4" functions
  actually copy 8, because that's what integers will be

  These should really be named for the data types they zero instead of
  the sizes!
*/
void zero4_( char *array, unsigned *length )
{
	mclear( array, *length * 8 );
}


void zero8_( char *array, unsigned *length )
{
	mclear( array, *length * 8 );
}


void zero16_( char *array, unsigned *length )
{
	mclear( array, *length * 8 );
}


void copy4_( char *from, char *to, int *length )
{
	mcopy( from, to, *length * 8 );
}


void copy8_( char *from, char *to, int *length )
{
	mcopy( from, to, *length * 8 );
}


void copy16_( char *from, char *to, int *length )
{
	mcopy( from, to, *length * 8 );
}

/*
 * misc.c - miscellaneous utility routines.
 * sccsid @(#)unix.c	6.1	(Splice2/Berkeley) 3/15/83
 */

#ifndef lint
// #define VAXUNIXASM
#endif
#define VAXMAXSIZE ((2<<15) - 1);

/*
 * mclear - clear memory.
 */
void mclear( char *data, int size )
{
#ifdef	VAXUNIXASM
	register int	i = VAXMAXSIZE;

	for ( ; size > i; size -= i, data += i ) {
		asm( "	movc5 $0,*4(ap),$0,r11,*4(ap)" );
	}
	asm( "	movc5 $0,*4(ap),$0,8(ap),*4(ap)" );
#else
	for ( ; size > 0; size--, data++ ) {
		*data = '\0';
	}
#endif
}




/*
 * mcopy - copy memory.
 */
void mcopy( char *from, char *to, int size )
{
#ifdef	VAXUNIXASM
	register int		i = VAXMAXSIZE;

	if ( size < i ) {
		asm( "	movc3 12(ap),*4(ap),*8(ap)" );
		return;
	}
	else if ( from >= to ) {
		for ( ; size > i; size -= i, to += i, from += i ) {
			asm( "	movc3 r11,*4(ap),*8(ap)" );
		}
		asm( "	movc3 12(ap),*4(ap),*8(ap)" );
		return;
	}
	else {
		to   += size;
		from += size;
		size -= i;
		for ( ; size > 0; size -= i ) {
			to   -= i;
			from -= i;
			asm( "	movc3 r11,*4(ap),*8(ap)" );
		}
		size += i;
		to   -= size;
		from -= size;
		asm( "	movc3 12(ap),*4(ap),*8(ap)" );
		return;
	}
#else
	if ( from >= to ) {
		for ( ; size > 0; size-- ) {
			*to++ = *from++;
		}
	}
	else {
		to   += size;
		from += size;
		for ( ; size > 0; size-- ) {
			*--to = *--from;
		}
	}
#endif
}



/*
 * mcmp - compare memory.
 */
int
mcmp( char *from, char *to, int size )
{
#ifdef	VAXUNIXASM
	register int	i = VAXMAXSIZE;

	for ( ; size > i; size -= i, to += i, from += i ) {
		asm( "	cmpc3 r11,*4(ap),*8(ap)" );
		asm( "	jeql L_zot_" );
		asm( "	ret" );
		asm( "L_zot_:" );
	}
	asm( "	cmpc3 12(ap),*4(ap),*8(ap)" );
#else
	for ( ; size > 0; size-- ) {
		if ( *to++ != *from++ ) {
			return( 1 );
		}
	}
	return( 0 );
#endif
}

static void GrabArgs(int argc, char* argv[], char* envp[])
{
    xargc = argc;
    xargv = argv;
    /* printf("GrabArgs: copied argc/argv to local variables\n"); */
}

#ifdef __APPLE__
    __attribute__((section("__DATA,__mod_init_func"))) void (* PtrGrabArgs)(int,char*[],char*[]) = &GrabArgs;
#else
    __attribute__((section(".init_array"))) void (* PtrGrabArgs)(int,char*[],char*[]) = &GrabArgs;
#endif 

#ifdef MISCTEST

#define TESTSIZE1   31683
#define TESTSIZE2  147755
#define TESTSIZE3   69683

#define MISCASSERT(cond) {if(!(cond))\
printf("Assertion botched line %d: cond\n", __LINE__); }

/*
 * Exercise for misc routines to check that they really work.
 */
main()
{
	int		i,	j;
	static	char	buff1[ TESTSIZE1 ],	buff2[ TESTSIZE1 ];
	static	char	buff3[ TESTSIZE2 ],	buff4[ TESTSIZE2 ];

	/*
	 * First check that everything is zeroed.
	 *  Assume TESTSIZE1 < TESTSIZE3 < TESTSIZE2
	 */
	MISCASSERT( mcmp(buff1, buff2, TESTSIZE1) == 0 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) == 0 );
	/*
	 * Set to all ones, check, clear, check
	 */
	for ( i = 0; i < TESTSIZE1; i++ ) {
		buff1[ i ] = -1;
	}
	MISCASSERT( mcmp(buff1, buff2, TESTSIZE1) != 0 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) != 0 );
	mcopy( buff1, buff2, TESTSIZE1 );
	MISCASSERT( mcmp(buff1, buff2, TESTSIZE1) == 0 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	for ( i = 0; i < TESTSIZE2; i++ ) {
		buff3[ i ] = -1;
	}
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) != 0 );
	MISCASSERT( mcmp(buff1, buff2, TESTSIZE1) == 0 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) == 0 );
	mcopy( buff3, buff4, TESTSIZE2 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	MISCASSERT( mcmp(buff1, buff4, TESTSIZE1) == 0 );

	mclear( buff3, TESTSIZE2 );
	mcopy( buff4, buff3, TESTSIZE2 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) == 0 );

	mclear( buff2, TESTSIZE1 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	mclear( buff3, TESTSIZE1 );
	MISCASSERT( mcmp(buff2, buff3, TESTSIZE1) == 0 );
	mcopy( buff1, buff3 + TESTSIZE3, TESTSIZE1 );
	MISCASSERT( mcmp(buff1, buff3 + TESTSIZE3, TESTSIZE1) == 0 );
	mclear( buff3 + TESTSIZE3, TESTSIZE1 );
	MISCASSERT( mcmp(buff2, buff3 + TESTSIZE3, TESTSIZE1) == 0 );
	mcopy( buff3 + TESTSIZE3, buff3, TESTSIZE1 );
	MISCASSERT( mcmp(buff2, buff3, TESTSIZE1) == 0 );
	mcopy( buff1, buff3, TESTSIZE1 );
	mcopy( buff1, buff3 + TESTSIZE3, TESTSIZE1 );
	mcopy( buff3 + TESTSIZE3, buff3, TESTSIZE1 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) == 0 );
	/*
	 * Repeat with sequence of numbers in mem locs.
	 */
	for ( i = 0; i < TESTSIZE1; i++ ) {
		buff1[ i ] = i;
	}
	MISCASSERT( mcmp(buff1, buff2, TESTSIZE1) != 0 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) != 0 );
	mcopy( buff1, buff2, TESTSIZE1 );
	MISCASSERT( mcmp(buff1, buff2, TESTSIZE1) == 0 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	for ( i = 0; i < TESTSIZE2; i++ ) {
		buff3[ i ] = i;
	}
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) != 0 );
	MISCASSERT( mcmp(buff1, buff2, TESTSIZE1) == 0 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) == 0 );
	mcopy( buff3, buff4, TESTSIZE2 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	MISCASSERT( mcmp(buff1, buff4, TESTSIZE1) == 0 );

	mclear( buff3, TESTSIZE2 );
	mcopy( buff4, buff3, TESTSIZE2 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) == 0 );

	mclear( buff2, TESTSIZE1 );
	MISCASSERT( mcmp(buff3, buff4, TESTSIZE2) == 0 );
	mclear( buff3, TESTSIZE1 );
	MISCASSERT( mcmp(buff2, buff3, TESTSIZE1) == 0 );
	mcopy( buff1, buff3 + TESTSIZE3, TESTSIZE1 );
	MISCASSERT( mcmp(buff1, buff3 + TESTSIZE3, TESTSIZE1) == 0 );
	mclear( buff3 + TESTSIZE3, TESTSIZE1 );
	MISCASSERT( mcmp(buff2, buff3 + TESTSIZE3, TESTSIZE1) == 0 );
	mcopy( buff3 + TESTSIZE3, buff3, TESTSIZE1 );
	MISCASSERT( mcmp(buff2, buff3, TESTSIZE1) == 0 );
	mcopy( buff1, buff3, TESTSIZE1 );
	mcopy( buff1, buff3 + TESTSIZE3, TESTSIZE1 );
	mcopy( buff3 + TESTSIZE3, buff3, TESTSIZE1 );
	MISCASSERT( mcmp(buff1, buff3, TESTSIZE1) == 0 );
}
#endif

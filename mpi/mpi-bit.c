/* mpi-bit.c  -  MPI bit level fucntions
 *	Copyright (c) 1997 by Werner Koch (dd9jn)
 *
 * This file is part of G10.
 *
 * G10 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * G10 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mpi-internal.h"
#include "longlong.h"


/****************
 * Return the number of bits in A.
 */
unsigned
mpi_get_nbits( MPI a )
{
    unsigned n;

    if( a->nlimbs ) {
	mpi_limb_t alimb = a->d[a->nlimbs-1];
	if( alimb )
	    count_leading_zeros( n, alimb );
	else
	    n = BITS_PER_MPI_LIMB;
	n = BITS_PER_MPI_LIMB - n + (a->nlimbs-1) * BITS_PER_MPI_LIMB;
    }
    else
	n = 0;
    return n;
}


/****************
 * Test wether bit N is set.
 */
int
mpi_test_bit( MPI a, unsigned n )
{
    unsigned limbno, bitno;
    mpi_limb_t limb;

    limbno = n / BITS_PER_MPI_LIMB;
    bitno  = n % BITS_PER_MPI_LIMB;

    if( limbno >= a->nlimbs )
	return 0; /* too far left: this is a 0 */
    limb = a->d[limbno];
    return (limb & (1 << bitno))? 1: 0;
}


/****************
 * Set bit N of A.
 */
void
mpi_set_bit( MPI a, unsigned n )
{
    unsigned limbno, bitno;

    limbno = n / BITS_PER_MPI_LIMB;
    bitno  = n % BITS_PER_MPI_LIMB;

    if( limbno >= a->nlimbs ) { /* resize */
	if( a->alloced >= limbno )
	    mpi_resize(a, limbno+1 );
	a->nlimbs = limbno+1;
    }
    a->d[limbno] |= (1<<bitno);
}

/****************
 * Set bit N of A. and clear all bits above
 */
void
mpi_set_highbit( MPI a, unsigned n )
{
    unsigned limbno, bitno;

    limbno = n / BITS_PER_MPI_LIMB;
    bitno  = n % BITS_PER_MPI_LIMB;

    if( limbno >= a->nlimbs ) { /* resize */
	if( a->alloced >= limbno )
	    mpi_resize(a, limbno+1 );
	a->nlimbs = limbno+1;
    }
    a->d[limbno] |= (1<<bitno);
    for( bitno++; bitno < BITS_PER_MPI_LIMB; bitno++ )
	a->d[limbno] &= ~(1 << bitno);
    a->nlimbs = limbno+1;
}

/****************
 * Clear bit N of A.
 */
void
mpi_clear_bit( MPI a, unsigned n )
{
    unsigned limbno, bitno;

    limbno = n / BITS_PER_MPI_LIMB;
    bitno  = n % BITS_PER_MPI_LIMB;

    if( limbno >= a->nlimbs )
	return; /* don't need to clear this bit, it's to far to left */
    a->d[limbno] &= ~(1 << bitno);
}


void
mpi_set_bytes( MPI a, unsigned nbits, byte (*fnc)(int), int opaque )
{
    byte *p;
    unsigned nlimbs, nlimbs2, xbits, xbytes;
    unsigned n;
    int i;

    nlimbs = nbits / BITS_PER_MPI_LIMB;
    xbits = nbits % BITS_PER_MPI_LIMB;
    nlimbs2 = xbits? (nlimbs+1):nlimbs;
    xbytes = xbits / 8;
    xbits = xbits % 8;
    if( a->alloced < nlimbs2 )
	mpi_resize(a, nlimbs2 );
    a->nlimbs = nlimbs2;
    for(n=0; n < nlimbs; n++ ) {
	p = (byte*)(a->d+n);
      #ifdef LITTLE_ENDIAN_HOST
	for(i=0; i < BYTES_PER_MPI_LIMB; i++ )
	    p[i] = fnc(opaque);
      #else
	for(i=BYTES_PER_MPI_LIMB-1; i>=0; i-- )
	    p[i] = fnc(opaque);
      #endif
    }
    if( xbytes ) {
	p = (byte*)(a->d+n);
      #ifdef LITTLE_ENDIAN_HOST
	for(i=0; i < xbytes; i++ )
	    p[i] = fnc(opaque);
      #else
	for(i=xbytes-1; i>=0; i-- )
	    p[i] = fnc(opaque);
      #endif
    }
  #if 0 /* fixme: set complete random byte and clear out the unwanted ones*/
    if( xbits ) {
	p = (byte*)(a->d+n);
      #ifdef LITTLE_ENDIAN_HOST
	for(i=0; i < xbytes; i++ )
	    p[i] = fnc(opaque);
      #else
	for(i=xbytes-1; i>=0; i-- )
	    p[i] = fnc(opaque);
      #endif
    }
  #endif
}

/****************
 * Shift A by N bits to the right
 * FIXME: should use alloc_limb if X and A are same.
 */
void
mpi_rshift( MPI x, MPI a, unsigned n )
{
    mpi_ptr_t xp;
    mpi_size_t xsize;

    xsize = a->nlimbs;
    x->sign = a->sign;
    RESIZE_IF_NEEDED(x, xsize);
    xp = x->d;

    if( xsize ) {
	mpihelp_rshift( xp, a->d, xsize, n);
	MPN_NORMALIZE( xp, xsize);
    }
    x->nlimbs = xsize;
}


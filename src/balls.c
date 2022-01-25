/* balls -- Functions for complex ball arithmetic.

Copyright (C) 2018, 2020, 2021, 2022 INRIA

This file is part of GNU MPC.

GNU MPC is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

GNU MPC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this program. If not, see http://www.gnu.org/licenses/ .
*/

#include "mpc-impl.h"


void mpcb_out_str (FILE *f, mpcb_srcptr op)
{
   mpc_out_str (f, 10, 0, op->c, MPC_RNDNN);
   fprintf (f, " ");
   mpcr_out_str (f, op->r);
   fprintf (f, "\n");
}


void
mpcb_init (mpcb_ptr rop)
{
   mpc_init2 (rop->c, 2);
   mpcr_set_inf (rop->r);
}


void
mpcb_clear (mpcb_ptr rop)
{
   mpc_clear (rop->c);
}


mpfr_prec_t
mpcb_get_prec (mpcb_srcptr op)
{
   return mpc_get_prec (op->c);
}


void
mpcb_set_prec (mpcb_ptr rop, mpfr_prec_t prec)
{
   mpc_set_prec (rop->c, prec);
   mpcr_set_inf (rop->r);
}


void
mpcb_set_c (mpcb_ptr rop, mpc_srcptr op)
{
   mpc_set_prec (rop->c, MPC_MAX_PREC (op));
   mpc_set (rop->c, op, MPC_RNDNN);
   mpcr_set_zero (rop->r);
}


void
mpcb_set (mpcb_ptr rop, mpcb_srcptr op)
{
   mpc_set_prec (rop->c, mpc_get_prec (op->c));
   mpc_set (rop->c, op->c, MPC_RNDNN);
   mpcr_set (rop->r, op->r);
}


void
mpcb_init_set_c (mpcb_ptr rop, mpc_srcptr op)
{
   mpc_init2 (rop->c, MPC_MAX_PREC (op));
   mpc_set (rop->c, op, MPC_RNDNN);
   mpcr_set_zero (rop->r);
}


void
mpcb_neg (mpcb_ptr z, mpcb_srcptr z1)
{
   mpfr_prec_t p;
   int overlap = (z == z1);

   if (!overlap) {
      p = mpcb_get_prec (z1);
      if (mpcb_get_prec (z) != p)
         mpcb_set_prec (z, p);
   }

   mpc_neg (z->c, z1->c, MPC_RNDNN); /* exact */
   mpcr_set (z->r, z1->r);
}


void
mpcb_mul (mpcb_ptr z, mpcb_srcptr z1, mpcb_srcptr z2)
{
   mpcr_t r;
   mpfr_prec_t p = MPC_MIN (mpcb_get_prec (z1), mpcb_get_prec (z2));
   int overlap = (z == z1 || z == z2);
   mpc_t zc;

   if (overlap)
      mpc_init2 (zc, p);
   else {
      zc [0] = z->c [0];
      mpc_set_prec (zc, p);
   }
   mpc_mul (zc, z1->c, z2->c, MPC_RNDNN);
   if (overlap)
      mpc_clear (z->c);
   z->c [0] = zc [0];

   /* generic error of multiplication */
   mpcr_mul (r, z1->r, z2->r);
   mpcr_add (r, r, z1->r);
   mpcr_add (r, r, z2->r);
   /* error of rounding to nearest */
   mpcr_add_rounding_error (r, p, MPFR_RNDN);
   mpcr_set (z->r, r);
}


void
mpcb_add (mpcb_ptr z, mpcb_srcptr z1, mpcb_srcptr z2)
{
   mpcr_t r, s, denom;
   mpfr_prec_t p = MPC_MIN (mpcb_get_prec (z1), mpcb_get_prec (z2));
   int overlap = (z == z1 || z == z2);
   mpc_t zc;

   if (overlap)
      mpc_init2 (zc, p);
   else {
      zc [0] = z->c [0];
      mpc_set_prec (zc, p);
   }
   mpc_add (zc, z1->c, z2->c, MPC_RNDZZ);
      /* rounding towards 0 makes the generic error easier to compute,
         but incurs a tiny penalty for the rounding error */

   /* generic error of addition:
      r <= (|z1|*r1 + |z2|*r2) / |z1+z2|
        <= (|z1|*r1 + |z2|*r2) / |z| since we rounded towards 0 */
   mpcr_mpc_abs (denom, zc, MPFR_RNDD);
   mpcr_mpc_abs (r, z1->c, MPFR_RNDU);
   mpcr_mul (r, r, z1->r);
   mpcr_mpc_abs (s, z2->c, MPFR_RNDU);
   mpcr_mul (s, s, z2->r);
   mpcr_add (r, r, s);
   mpcr_div (r, r, denom);
   /* error of directed rounding */
   mpcr_add_rounding_error (r, p, MPFR_RNDZ);

   if (overlap)
      mpc_clear (z->c);
   z->c [0] = zc [0];
   mpcr_set (z->r, r);
}


void
mpcb_sqrt (mpcb_ptr z, mpcb_srcptr z1)
{
   mpcr_t r;
   mpfr_prec_t p = mpcb_get_prec (z1);
   int overlap = (z == z1);

   /* Compute the error first in case there is overlap. */
   /* generic error of square root for z1->r <= 0.5:
      0.5*epsilon1 + (sqrt(2)-1) * epsilon1^2
      <= 0.5 * epsilon1 * (1 + epsilon1),
      see eq:propsqrt in algorithms.tex, together with a Taylor
      expansion of 1/sqrt(1-epsilon1) */
   if (!mpcr_lt_half_p (z1->r))
      mpcr_set_inf (r);
   else {
      mpcr_set_one (r);
      mpcr_add (r, r, z1->r);
      mpcr_mul (r, r, z1->r);
      mpcr_div_2ui (r, r, 1);
      /* error of rounding to nearest */
      mpcr_add_rounding_error (r, p, MPFR_RNDN);
   }

   if (!overlap)
      mpcb_set_prec (z, p);
   mpc_sqrt (z->c, z1->c, MPC_RNDNN);
   mpcr_set (z->r, r);
}


void
mpcb_div (mpcb_ptr z, mpcb_srcptr z1, mpcb_srcptr z2)
{
   mpcr_t r, s;
   mpfr_prec_t p = MPC_MIN (mpcb_get_prec (z1), mpcb_get_prec (z2));
   int overlap = (z == z1 || z == z2);
   mpc_t zc;

   if (overlap)
      mpc_init2 (zc, p);
   else {
      zc [0] = z->c [0];
      mpc_set_prec (zc, p);
   }
   mpc_div (zc, z1->c, z2->c, MPC_RNDNN);
   if (overlap)
      mpc_clear (z->c);
   z->c [0] = zc [0];

   /* generic error of division */
   mpcr_add (r, z1->r, z2->r);
   mpcr_set_one (s);
   mpcr_sub_rnd (s, s, z2->r, MPFR_RNDD);
   mpcr_div (r, r, s);
   /* error of rounding to nearest */
   mpcr_add_rounding_error (r, p, MPFR_RNDN);
   mpcr_set (z->r, r);
}


void
mpcb_div_2ui (mpcb_ptr z, mpcb_srcptr z1, unsigned long int e)
{
   mpc_div_2ui (z->c, z1->c, e, MPC_RNDNN);
   mpcr_set (z->r, z1->r);
}


int
mpcb_can_round (mpcb_srcptr op, mpfr_prec_t prec_re, mpfr_prec_t prec_im)
{
   mpfr_srcptr re, im;
   mpfr_exp_t exp_re, exp_im, exp_err;

   re = mpc_realref (op->c);
   im = mpc_imagref (op->c);
   /* The question makes sense only if neither the real nor the imaginary
      part of the centre are 0: Otherwise, we can either round this part
      to 0 or we cannot round. But to round to 0, we need to have an
      absolute error that is less than the smallest representable number;
      otherwise said, the precision needs to be about as big as the negative
      of the minimal exponent, which is astronomically large. */
   if (mpfr_zero_p (re) || mpfr_zero_p (im))
      return 0;

   exp_re = mpfr_get_exp (re);
   exp_im = mpfr_get_exp (im);

   /* Absolute error of the real part, as given in the proof of
      prop:comrelerror of algorithms.tex:
      |x-x~|  = |x~*theta_R - y~*theta_I|
             <= |x~ - y~| * epsilon, where epsilon is the complex
                                     relative error
             <= (|x~|+|y~|) * epsilon
             <= 2 * max (|x~|, |y~|) * epsilon
      To call mpfr_can_round, we only need the exponent in base 2,
      which is then bounded above by
                1 + max (exp_re, exp_im) + exponent (epsilon) */
   exp_err = 1 + MPC_MAX (exp_re, exp_im) + mpcr_get_exp (op->r);

   return (   mpfr_can_round (re, exp_re - exp_err, MPFR_RNDN, MPFR_RNDN,
                              prec_re)
           && mpfr_can_round (im, exp_im - exp_err, MPFR_RNDN, MPFR_RNDN,
                              prec_im));
}


void
mpcb_round (mpc_ptr rop, mpcb_srcptr op)
{
   mpc_set (rop, op->c, MPC_RNDNN);
}

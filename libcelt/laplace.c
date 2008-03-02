/* (C) 2007 Jean-Marc Valin, CSIRO
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "laplace.h"

static int ec_laplace_get_start_freq(int decay)
{
   return (((ec_uint32)32767)*(16384-decay))/(16384+decay);
}

void ec_laplace_encode(ec_enc *enc, int value, int decay)
{
   int i;
   int fl, fs, ft;
   int s = 0;
   if (value < 0)
   {
      s = 1;
      value = -value;
   }
   ft = 32767;
   fs = ec_laplace_get_start_freq(decay);
   fl = -fs;
   for (i=0;i<value;i++)
   {
      int tmp_l, tmp_s;
      tmp_l = fl;
      tmp_s = fs;
      fl += fs*2;
      fs = (fs*(ec_int32)decay)>>14;
      if (fs == 0)
      {
         fs = tmp_s;
         fl = tmp_l;
         break;
      }
   }
   if (fl < 0)
      fl = 0;
   if (s)
      fl += fs;
   /*DEBUG*/
   /*printf ("enc: %d %d %d\n", fl, fs, ft);*/
   ec_encode(enc, fl, fl+fs, ft);
}

int ec_laplace_decode(ec_dec *dec, int decay)
{
   int val=0;
   int fl, fh, fs, ft, fm;
   fl = 0;
   ft = 32767;
   fs = ec_laplace_get_start_freq(decay);
   fh = fs;
   fm = ec_decode(dec, ft);
   /*DEBUG*/
   /*printf ("fm: %d/%d\n", fm, ft);*/
   while (fm >= fh && fs != 0)
   {
      fl = fh;
      fs = (fs*(ec_int32)decay)>>14;
      fh += fs*2;
      val++;
   }
   if (fl>0)
   {
      if (fm >= fl+fs)
      {
         val = -val;
         fl += fs;
      } else {
         fh -= fs;
      }
   }
   /* Preventing an infinite loop in case something screws up in the decoding */
   if (fl==fh)
      fl--;
   /*DEBUG*/
   /*printf ("update: %d %d %d\n", fl, fh, ft);*/
   ec_dec_update(dec, fl, fh, ft);
   return val;
}



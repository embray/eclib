// interface.cc: implementation of non-inline functions from interface.h
//////////////////////////////////////////////////////////////////////////
//
// Copyright 1990-2012 John Cremona
// 
// This file is part of the eclib package.
// 
// eclib is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
// 
// eclib is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
// 
// You should have received a copy of the GNU General Public License
// along with eclib; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
// 
//////////////////////////////////////////////////////////////////////////
 
#include <eclib/interface.h>

// integers and rationals

int I2int(const ZZ& x)
{
  if(IsZero(x)) return 0;
  if(!is_int(x)) 
    {
      cerr<<"Attempt to convert "<<x<<" to int fails!"<<endl;
      return 0;
    }
  switch(sign(x)) {
  case 0: 
    return (int)0;
  case 1: 
    return (x==MAXINT? (int)MAXINT : (int)rem(x,(long)MAXINT));
  default:
    return (x==MININT? (int)MININT : -I2int(-x));
  }
}


long I2long(const bigint& x) 
{
  if(IsZero(x)) return 0;
  if(!is_long(x))
    {
      cerr<<"Attempt to convert "<<x<<" to long fails!"<<endl;
      return 0;
    }
  switch(sign(x)) {
  case 0: 
    return 0;
  case 1: 
    return (x==MAXLONG? (long)MAXLONG : (long)rem(x,(long)MAXLONG));
  default: 
    return (x==MINLONG? (long)MINLONG : -I2long(-x));
  }
}

// Reals and Complexes

#ifdef MPFP

RR Pi() 
{
  static long pr;
  static RR pi; 
  if((pi<to_RR(1))            // then we have not computed it yet
     ||(pr<RR::precision()))    // or precision has increased
    {
      pr = RR::precision();
      ComputePi(pi); 
      //      std::cout<<"Computing pi to precision "<<pr<<": "<<pi<<std::endl;
    }
  return pi;
}

void Compute_Euler(RR&);
RR Euler() //{return to_bigfloat(0.57721566490153286060651209008240243104);}
{
  static long pr;
  static RR gamma; 
  if((gamma<to_RR(1))            // then we have not computed it yet
     ||(pr<RR::precision()))    // or precision has increased
    {
      pr = RR::precision();
      Compute_Euler(gamma); 
    }
  return gamma;
}

#define LOG2   0.69314718055994530942

void Compute_Euler(RR& y) 
{
  long l, n, k, x;
  bigfloat u, v, a, b, c;

  l = RR::precision();

  x = 1 + static_cast<long>((0.25 * (l - 3)) * (NTL_BITS_PER_LONG * LOG2));
  n = 1 + static_cast<long>(3.591 * x);

  a=x;
  log(u, a);
  if (sign(u) > 0)  u=-u;
  a=u;
  v=b=to_bigfloat(1);
  
  for (k = 1; k <= n; k++) {
    mul(b, b, x);
    mul(b, b, x);
    div(b, b, (k * k));
    mul(a, a, x);
    mul(a, a, x);
    div(a, a, k);
    add(c, a, b);
    div(a, c, k);
    add(u, u, a);
    add(v, v, b);
  }
  div(y, u, v);
}

long prec() {return RR::precision();}
void setprec(long p) { RR::SetPrecision(p);}

RR atan(const RR& x)   // translated from LiDIA
{
  long i, j, ex, t = prec(); // t stores input precision for restoring later
  long m, u, f;
  RR y;
  
  if (IsZero(x))    // atan(0)=0
    return to_RR(0);

  ex = x.exponent() + t;

  if (ex > t) 
    setprec(ex + NTL_BITS_PER_LONG - ex % NTL_BITS_PER_LONG);

  RR a=to_RR(1);
  RR tmp(x);

  m = 0;
  if (sign(tmp)<0) // compute atan(|x|) and negate later
    {
      m = 1;
      tmp=-tmp;
    }

  if (compare(tmp,a) == 0)  // atan(1)-Pi/4
    {
      y=Pi()/to_RR(4);
      if (m) NTL::negate(y,y);
      setprec(t);
      return y;
    }

  ex = tmp.exponent() + prec();
  u = 0;
  if (ex > 0) {
    inv(tmp, tmp);
    u = 1;
  }

  ex = tmp.exponent() + prec();
  f = 0;
  RR q(tmp);
  if (ex > -10)
    while (tmp.exponent() + prec() > -10) {
      mul(q, q, tmp);
      add(q, q, a);
      SqrRoot(q, q);
      add(q, q, a);
      div(tmp, tmp, q);
      q=tmp;
      f++;
    }
  power(a, tmp, 2);

  ex = tmp.exponent() + prec();
  if (ex < 0) ex = -ex;
  ex <<= 1;
  j = prec() / ex;
  if (j & 1) j++;
  y = inv(to_RR(2 * j + 1));
  setprec(4 * ex);

  for (i = j; i >= 1; i--) {
    mul(y, y, a);
    q=inv(to_RR(2 * i - 1));
    setprec(prec() + 2 * ex);
    if (prec() > t) setprec(t);
    NTL::negate(y,y);
    add(y, y, q);
  }
  setprec(t);
  mul(y, y, tmp);
  y*=power2_RR(f);

  if (u) {
    a=Pi()/to_RR(2);
    sub(y, y, a);
    NTL::negate(y,y);
  }
  if (m) NTL::negate(y,y);
  return y;
}

RR asin (const RR & x)
{
  if (sign(x-1) == 0) return Pi()/2;
  if (sign(x+1) == 0) return -Pi()/2;

  RR t = 1-x*x;
  if (sign(t)<0) 
    {
      cout<<"asin called with arguments "<<x<<" > 1"<<endl;
      return to_RR(0);
    }
  return atan(x/sqrt(t));
}

namespace NTL {
RR atan2 (const RR & y, const RR & x)
{
  RR z,w;
  int ys = sign(y), xs = sign(x);
  int yss = (ys < 0), xss = (xs < 0);
  char code = yss + (xss << 1);
  
  if (xs == 0) {
    if (ys != 0)      
      {
	z=Pi()/2;
	if (ys < 0) NTL::negate(z,z);
      }
    return z;
  }

  if (ys == 0) {
    if (xs < 0)
      z=Pi();
    return z;
  }

  switch(code) {
  case 0:
  case 1:
    w=0;
    break;
  case 2:
    w=Pi();
    break;
  case 3:
    w=-Pi();
    break;
  }
  
  z=y;
  div(z, z, x);
  z=atan(z);
  add(z, z, w);
  
  return z;
}
}

// The template version requires an automatic conversion from 0 to an
// RR, so cannot be used as is.  We have manually instantiated it
// here.
istream& operator>>(istream& is, CC& z)
{
  RR r, i;
  char c;
  is >> c;
  if (c == '(') 
    {
      is >> r >> c;
      if (c == ',') 
	{
	  is >> i >> c;
	  if (c == ')') 
	    z = CC(r, i);
	  else
	    is.setstate(ios_base::failbit);
	}
      else if (c == ')') 
	z = CC(r, to_RR(0));
      else
	is.setstate(ios_base::failbit);
    }
  else 
    {
      is.putback(c);
      is >> r;
      z = CC(r, to_RR(0));
    }
  return is;
}

// return value is 1 for success, else 0
int longify(const bigfloat& x, long& a, int rounding)
{
  if ((x<=MAXLONG)&&(x>=MINLONG))
    {
      switch(rounding)
        {
        case 0: // round to nearest
          a = I2long(RoundToZZ(x)); return 1;
        case 1: // round up
          a = I2long(CeilToZZ(x)); return 1;
        case -1: // round down
          a = I2long(FloorToZZ(x)); return 1;
        }
    }
  else
    {
      cerr<<"Attempt to convert "<<x<<" to long fails!"<<endl;
      return 0;
    }
}

#else // not MPFP

// return value is 1 for success, else 0
int longify(double x, long& a, int rounding)
// assume that direct coercion to long truncates fraction part
// disregarding sign so rounds down if x>0, up if x<0
{
  if ((x<=MAXLONG)&&(x>=MINLONG))
    {
      if (x>0)
        {
          switch(rounding)
            {
            case 0: // round to nearest
              a = (long)(x+0.5); return 1;
            case 1: // round up
              a = - (long)(-x+0.5); return 1;
            case -1: // round down
              a = (long)x; return 1;
            }
        }
      else
        {
          switch(rounding)
            {
            case 0: // round to nearest
              a = - (long)(-x+0.5); return 1;
            case 1: // round up
              a = (long)x; return 1;
            case -1: // round down
              a = (long)(x-0.5); return 1;
            }
        }
    }
  else
    {
      cerr<<"Attempt to convert "<<x<<" to long fails!"<<endl;
      return 0;
    }
}

#endif // MPFP

string getenv_with_default(string env_var, string def_val)
{
  stringstream s;
  if (getenv(env_var.c_str()) != NULL) {
    s << getenv(env_var.c_str());
  } else {
    s<<def_val;
  }
  return s.str();
}

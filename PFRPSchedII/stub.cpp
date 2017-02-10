#include "stdafx.h"
#include "PFRPSchedII.h"

INT64 gcd(INT64 a, INT64 b)
{
  if (b == 0)
    return a;
  else
    return gcd(b, a%b);
}

INT64 lcm(INT64 a, INT64 b)
{
  return (a*b)/gcd(a,b);
}
#ifndef RANDOM_H
#define RANDOM_H

#include <cstdlib>

#include "Range.h"

namespace Graphics
{
  template< typename T = float >
  class Random
  {
  public:
    Random(){}

    T next()
    {
      return( rand() );
    }

    T next( T max )
    {
      return( max * (T(rand()) / RAND_MAX) );
    }

    T next( T min, T max )
    {
      return( min + ((max - min) * (T(rand()) / RAND_MAX)) );
    }

    T next( Range<T> r )
    {
      return( r.min + ((r.max - r.min) * (T(rand()) / RAND_MAX)) );
    }
  };
}

#endif
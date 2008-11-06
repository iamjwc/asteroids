#ifndef RANGE_H
#define RANGE_H

namespace Graphics
{
  template< typename T = float >
  class Range
  {
  public:
    T min;
    T max;

    Range(){}

    Range( T min, T max )
    {
      this->min = min;
      this->max = max;
    }
  };
}

#endif
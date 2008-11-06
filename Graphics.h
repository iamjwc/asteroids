#include "Range.h"
#include "Random.h"
#include "Point.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

namespace Graphics
{
  const float PI_OVER_180 = 0.0174532925f;  // One Degree (in Radians).      //

  void garbage_collect( void *v )
  {
    delete v;
    v = NULL;
  }

  void garbage_collect_array( void *v )
  {
    delete [] v;
    v = NULL;
  }

  class Vector
  {
  public:
    Point<> origin;
    float direction;
    float magnitude;

    Vector()
    {
      this->direction = this->magnitude = 0;
    }

    Vector( Point<> origin )
    {
      this->origin    = origin;
      this->direction = this->magnitude = 0;
    }

    Vector( Point<> origin, float direction, float magnitude )
    {
      this->origin    = origin;
      this->direction = direction;
      this->magnitude = magnitude;
    }

    Point<> end_point()
    {
      float x = this->magnitude * cosf( direction ) + this->origin.x;
      float y = this->magnitude * sinf( direction ) + this->origin.y;

      return( Point<>( x, y ) );
    }
    
    Point<> start_point()
    {
      return( this->origin );
    }

    Vector add( Vector v )
    {
      v.origin = this->end_point();

      Point<> end = v.end_point();

      Vector new_vector;
      new_vector.origin    = this->origin;
      new_vector.magnitude = this->origin.distance_from( end );
      new_vector.direction = atanf( ( end.y - new_vector.origin.y ) / ( end.x - new_vector.origin.x ) );

      return( new_vector );
    }
  };

  
  template<typename T>
  class Vector2
  {
  public:
	  //T x, y;
    
    Point<T> p;
	
    Vector2()
    {
      this->p.x = this->p.y = 0;
    }

	  Vector2( T x, T y )
    {
      this->p.x = x;
      this->p.y = y;
    }

    static Vector2 from_magnitude_and_direction( T magnitude, T direction )
    {
      T x = magnitude * cos( direction );
      T y = magnitude * sin( direction );

      return( Vector2( x, y ) );
    }

	  Vector2 operator+( Vector2 v )
	  {
      return( Vector2( this->p.x + v.p.x, this->p.y + v.p.y ) );
    }

	  Vector2 operator-( Vector2 v )
	  {
      return( Vector2( this->p.x - v.p.x, this->p.y - v.p.y ) );
    }

	  Vector2& operator+=( Vector2 v )
	  { 
      this->p.x += v.p.x;
      this->p.y += v.p.y;
      
      return( *this );
    }

	  Vector2& operator-=( Vector2 v )
	  { 
      this->p.x -= v.p.x;
      this->p.y -= v.p.y;
      
      return( *this );
    }

	  T dot( Vector2 v )
	  {
      return( ( this->p.x * v.p.x ) + ( this->p.y * v.p.y ) );
    }

	  T length()
    {
      return( sqrt( ( this->p.x * this->p.x ) + ( this->p.y * this->p.y ) ) );
    }

    T angle()
    {
      T quadrant_adjust;

      if( this->p.x > 0 && this->p.y > 0 ) // First Quadrant
        quadrant_adjust = 0;
      else if( this->p.x < 0 && this->p.y > 0 ) // Second Quadrant
        quadrant_adjust = 180;
      else if( this->p.x < 0 && this->p.y < 0 ) // Third Quadrant
        quadrant_adjust = 180;
      else if( this->p.x > 0 && this->p.y < 0 ) // Fourth Quadrant
        quadrant_adjust = 360;

      return( atan( ( this->p.y ) / ( this->p.x ) ) + ( quadrant_adjust * PI_OVER_180 ) );
    }

	  bool operator==( Vector2 v )
	  {
      return( ( this->p.x == v.p.x ) && ( this->p.y == v.p.y ) );
    }

	  bool operator!=( Vector2 v )
	  {
      return( !( *this == v ) );
    }
  };

  class ParticleSystem
  {
  public:
    Point<>   location;
    int     count;
    Vector *particles;
    Random<>  r;
    Range<>   velocity_range;
    int     display_count, display_max;

    float   chaos;

    float   color[3];
    float   opacity;

    bool is_clean;
    bool is_paused;

    void init( Point<> origin )
    {
      this->location = origin; 
      this->count  = 1500;
      this->velocity_range = Range<>( 0.0f, 0.010f );
      this->display_count = 0;
      this->display_max = 100;

      this->color[0] = 1.0f;
      this->color[1] = 1.0f;
      this->color[2] = 1.0f;

      this->chaos = .5f;


      this->is_clean  = false;
      this->is_paused = false;

      this->generate_points();
    }

    ParticleSystem( Point<> origin = Point<>() )
    {
      this->init( origin );
    }

    ~ParticleSystem()
    {
      this->cleanup();
    }

    void generate_points()
    {
      float velocity;
      float tick;
      this->particles = new Vector[count];

      for( int i = 0; i < this->count; i++ )
      {
        tick               = ( float(this->count) / 360 ) * i * PI_OVER_180;
        velocity           = r.next( this->velocity_range );
        this->particles[i] = Vector( this->location, tick, velocity );
      }
    }

    void cleanup()
    {
      if( this->is_clean ) return;

      garbage_collect_array( this->particles );

      this->is_clean = true;
    }

    bool draw( int blur = 0 )
    {
      if( this->display_count >= this->display_max )
      {
        this->cleanup();
        return(false);
      }
      else
      {
        this->set_opacity();

        glColor4f( this->color[0], this->color[1], this->color[2], this->opacity );
        glBegin( GL_POINTS  );
          for( int i = 0; i < blur+1; i++ )
          {
            for( int j = 0; j < count; j++ )
            {
              this->draw_particle(j);
              this->move(j);
            }
          }
        glEnd();

        if( !this->is_paused )
          this->display_count++;

        return(true);
      }
    }

    void set_opacity()
    {
      this->opacity = 1 - float(this->display_count) / this->display_max;
    }

    void draw_particle( int i )
    {
      glVertex2fv( this->particles[i].origin.to_array() );
    }

    void move( int i )
    {
      if( this->is_paused ) return;

      Point<> p = this->particles[i].end_point();
      p.x += r.next( -this->chaos, this->chaos ) / 100;
      p.y += r.next( -this->chaos, this->chaos ) / 100;

      this->particles[i].origin = p;
    }
  };

  /* class GarbageCollecter
  {
  public:
    enum GarbageType
    {
      NORMAL,
      ARRAY
    };
    
    struct Garbage
    {
      void *item;
      GarbageType type;

      Garbage(){}

      Garbage( void *item, GarbageType type )
      {
        this->item = item;
        this->type = type;
      }
    };

    Garbage *trash;
    int      count;

    GarbageCollecter()
    {
      trash = NULL;
      count = 0;
    }

    ~GarbageCollecter()
    {
      
    }

    void add( void *item, GarbageType type = GarbageType::NORMAL )
    {
      this->add( Garbage( item, type ) );
    }

    void add( Garbage g )
    {
      Garbage temp = new Garbage[count];

      for( int i = 0; i < this->count; i++ )
        temp[i] = this->trash[i];

      temp[this->count] = g;

      this->count++;

      delete [] this->trash;
      this->trash = temp;
    }
  };*/

}

#endif
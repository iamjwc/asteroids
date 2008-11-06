#ifndef ASTEROID_H
#define ASTEROID_H

#include "Graphics.h"
#include <iostream>

#include <stdlib.h>
#include <time.h>

using namespace Graphics;
using namespace std;

class Asteroid
{
private:
public:
  
  Random<> r;

  int    sides;
  Point<> *points;
  Point<>  location;

  Range<>  radius_range;
  Range<>  rotation_range;
  Range<>  velocity_range;

  Vector2<float> velocity;

  float  rotation_inc;
  float  rotation;

  int    fragment_count;
  int    explode_count;

  bool   is_paused;

public:
  Asteroid( Range<> size = Range<>( 0.05f, 0.1f ) )
  {
    this->fragment_count = 3;
    this->explode_count  = 2;

    this->radius_range   = size;
    this->velocity_range = Range<>( 0.01f, 0.03f );
    this->rotation_range = Range<>( -2.5f, 2.5f );

    this->sides          = 12;

    this->rotation       = 0;
    this->rotation_inc   = r.next( this->rotation_range );

    this->velocity      = Vector2<float>::from_magnitude_and_direction( r.next( this->velocity_range.min, this->velocity_range.max ), r.next( 0, 360 * PI_OVER_180 ) );

    this->is_paused      = false;
    
    this->generate_shape();
  }

  ~Asteroid()
  {
    delete [] this->points;
    this->points = NULL;
  }

  void draw()
  {
    if( !this->is_paused )
    {
      this->spin();
      this->move();
    }

    glTranslatef( this->location.x, this->location.y, 0 );
    glRotatef( this->rotation, 0, 0, 1);

    glColor3f( 0, 0, 0 );
    this->draw_points( GL_POLYGON );

    glColor3f( 1, 1, 1 );
    this->draw_points( GL_LINE_LOOP );

    glLoadIdentity();
  }

  void draw_points( GLenum mode = GL_LINE_LOOP )
  {
    glBegin( mode );
      for( int i = 0; i < this->sides; i++ )
        glVertex2f( points[i].x, points[i].y );
    glEnd();
  }

  void move()
  {
    this->location.x += this->velocity.p.x;
    this->location.y += this->velocity.p.y;
  }

  void move_to(Point<> p)
  {
    this->location = p;
  }

  void spin()
  {
    this->rotation += this->rotation_inc;
  }

  void check_boundaries(float h, float w)
  {
    float half_of_width  = w / 2;
    float half_of_height = h / 2;

    if( this->location.x < -half_of_width - this->radius_range.max )
      this->move_to( Point<>( half_of_width, this->location.y+this->radius_range.max ) );

    else if( this->location.x > half_of_width + this->radius_range.max )
      this->move_to( Point<>( -half_of_width, this->location.y-this->radius_range.max ) );

    else if( this->location.y < -half_of_height - this->radius_range.max )
      this->move_to( Point<>( this->location.x, half_of_height+this->radius_range.max) );

    else if( this->location.y > half_of_height + this->radius_range.max )
      this->move_to( Point<>( this->location.x, -half_of_height-this->radius_range.max) );
  }

  bool hit_test( Point<> coordinates )
  {
    float d = this->location.distance_from( coordinates );
    
    return( d <= ( this->radius_range.max * 1.2 ) );
  }

  Asteroid **get_fragments()
  {
    Asteroid **fragments = new Asteroid*[ this->fragment_count ];

    Range<> new_size( this->radius_range.min/(this->fragment_count * .75f), this->radius_range.max/(this->fragment_count * .75f) );

    float direction = ( 360 / fragment_count ) * PI_OVER_180;
    for( int i = 0; i < fragment_count; i++ )
    {
      fragments[i] = new Asteroid( new_size );

      Vector2<float> v = Vector2<float>::from_magnitude_and_direction( fragments[i]->radius_range.max, direction*i + this->velocity.angle() );
      
      fragments[i]->move_to( this->location + v.p );
      fragments[i]->velocity = this->velocity + Vector2<float>::from_magnitude_and_direction( fragments[i]->velocity.length(), direction*i + this->velocity.angle() );

      fragments[i]->explode_count = this->explode_count-1;
    }

    return(fragments);
  }

  ParticleSystem *get_particle_system()
  {
    return( new ParticleSystem( this->location ) );
  }

  bool can_explode()
  {
    return( this->explode_count > 0 );
  }

private:
  void generate_shape()
  {
    float radius;
    float tick;
    this->points = new Point<>[sides];
    Vector2<float> v;

    for( int i = 0; i < sides; i++ )
    {
      tick            = (360 / sides) * i * PI_OVER_180;
      radius          = r.next( this->radius_range.min, this->radius_range.max );
      v               = Vector2<float>::from_magnitude_and_direction(radius, tick);
      this->points[i] = v.p;
    }
  }
};

#endif
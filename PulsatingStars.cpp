/************************************************************/
/* Filename: PulsatingStars.cpp                             */
/* Generates a dozen star-shaped polygons that float and    */
/* pulsate in a 2D environment, bouncing off of any window  */
/* borders with they collide. Mouse operations are used to  */
/* "freeze" stars and keyboard and menu operations are used */
/* to resize or recolor those stars that are frozen.        */
/************************************************************/

#include <GLUT/glut.h>
#include <math.h>			// Header File For Math Library
#include <time.h>			// Header File For Accessing System Time
#include "LinkedList.h"		// Header File For Linked List Class
#include <iostream>

#include <fstream>

#include "Graphics.h"
#include "Asteroid.h"

//////////////////////
// Global Constants //
//////////////////////
const int INIT_WINDOW_POSITION[2] = { 100, 100 }; // Window Offset (in pixels).

using namespace Graphics;
using namespace std;




/////////////////////////
// Function Prototypes //
/////////////////////////
void key_press(unsigned char pressedKey, int mouseXPosition, int mouseYPosition);
void mouse_click(int mouseButton, int mouseState, int mouseXPosition, int mouseYPosition);
void menu(int menuID);
void tick(int value);
void draw();
void resize_window(GLsizei w, GLsizei h);
void init_gl( void (*f)() );
void init_main();

void draw_particle_system( ParticleSystem *particles );
void draw_asteroid( Asteroid *asteroid );
bool asteroid_click( Asteroid *asteroid );
void pause_asteroid( Asteroid *asteroid );
void unpause_asteroid( Asteroid *asteroid );
void pause_particles( ParticleSystem *particles );
void unpause_particles( ParticleSystem *particles );
void garbage_collect_asteroids( Asteroid *asteroid );
void garbage_collect_particles( ParticleSystem *particles );


//////////////////////
// Global Variables //
//////////////////////
int   g_current_window_size[] = { 1000, 750 };  // Window size in pixels { w, h }
float g_window_ratio[]        = { 4.0f, 3.0f }; // Window ratio { w, h }

LinkedList<Asteroid*>       g_asteroids; // Linked list of pointers to all of the Asteroids
LinkedList<ParticleSystem*> g_particles; // Linked list of pointers to all of the ParticleSystems

Point<> g_click_coordinates; // Coordinates of the last click on the screen

bool  g_is_paused;

Random<> g_random;

/* The main function: uses the OpenGL Utility Toolkit to set */
/* the window up to display the window and its contents.     */
int main(int argc, char **argv)
{
	glutInit (&argc, argv);

  init_gl( init_main );

  // Collects the garbage for both of the linked lists
  g_asteroids.each( garbage_collect_asteroids );
  g_particles.each( garbage_collect_particles );
}

void init_main()
{
  // Collects the garbage for both of the linked lists
  g_asteroids.each( garbage_collect_asteroids );
  g_particles.each( garbage_collect_particles );

  // Reinitialize the linked lists
  g_asteroids = LinkedList<Asteroid*>();
  g_particles = LinkedList<ParticleSystem*>();

  g_is_paused = false;

  // Generate X number of asteroids, places them at random
  // points on the screen, and inserts them into the list
  Asteroid *asteroid;
  Point<>     random_point;
  for( int i = 0; i < 12; i++ )
  {
    random_point.x = g_random.next( -g_window_ratio[0]/2.0, g_window_ratio[0]/2.0 );
    random_point.y = g_random.next( -g_window_ratio[1]/2.0, g_window_ratio[1]/2.0 );

    asteroid = new Asteroid( Range<>( .1f, .2f ) );
    asteroid->move_to( random_point );

    g_asteroids.insert( asteroid );
  }
}

void init_gl( void (*f)() )
{
  srand(time(0));
  
	/* Set up the display window. */
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
  glutInitWindowPosition( INIT_WINDOW_POSITION[0], INIT_WINDOW_POSITION[1] );
  glutInitWindowSize( g_current_window_size[0], g_current_window_size[1] );
  glutCreateWindow( "Astroids!!!" );

  // AntiAliasing
  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  // Creates the context menu
	glutCreateMenu( menu );
	glutAddMenuEntry( "Pause/Unpause", 1 );
	glutAddMenuEntry( "Reset", 2 );
	glutAttachMenu(GLUT_RIGHT_BUTTON);


  // Call program specific init code;
  f();


	/* Specify the resizing, refreshing, and interactive routines. */
	glutReshapeFunc( resize_window );
	glutDisplayFunc( draw );
  glutKeyboardFunc( key_press );
	glutMouseFunc( mouse_click );
	glutTimerFunc(50, tick, 1);
	glutMainLoop();
}

/* Function to react to the pressing of a mouse button by the user, */
/* by determining whether the mouse is positioned within a star's   */
/* boundaries and, if so, by freezing (or unfreezing) that star.    */
void mouse_click(int mouse_button, int mouse_state, int mouse_x, int mouse_y)
{
  // Exit function if mouse is not done, or if the game is paused
	if( mouse_state != GLUT_DOWN ) return;
  if( g_asteroids.getHeadValue()->is_paused ) return;

  // Converts the coordinates passed in, into the windows coordinate system
	g_click_coordinates.x = g_window_ratio[0] * mouse_x / g_current_window_size[0] - 0.5 * g_window_ratio[0];
	g_click_coordinates.y = 0.5 * g_window_ratio[1] - (g_window_ratio[1] * mouse_y / g_current_window_size[1]);

  // Call asteroid_click for each asteroid in the linked list
  g_asteroids.each( asteroid_click );
}

bool asteroid_click( Asteroid* asteroid )
{
  // If not hit, move on to the next asteroid in the list
  if( !asteroid->hit_test( g_click_coordinates ) )
    return( true );


  // Remove the asteroid clicked on from the linked list
  g_asteroids.removeHead();

  // If the asteroid clicked on can explode into smaller chunks,
  // expode, and add the fragments to the list of asteroids on
  // the screen;
  if( asteroid->can_explode() )
  {
    Asteroid **fragments = asteroid->get_fragments();

    for( int i = 0, n = asteroid->fragment_count; i < n; i++ )
      g_asteroids.insert( fragments[i] );

    garbage_collect_array( fragments );
  }

  // Get the asteroids particle system and add it to the global
  // particle system linked list
  g_particles.insert( asteroid->get_particle_system() );

  garbage_collect( asteroid );

  // End the loop
  return( false );
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLineWidth(2);

  // Draws each asteroid and particle system in its linked list
  g_asteroids.each( draw_asteroid, false );
  g_particles.each( draw_particle_system );

  glutSwapBuffers();
	glFlush();
}

void draw_particle_system( ParticleSystem *particles )
{
  // Draw particles with a 3 pixel blur until they fade out
  // when they have faded out, delete them
  if( !particles->draw( 3 ) )
  {
    garbage_collect( particles ); 
    g_particles.removeHead();
  }
}

// This function draws an asteroid
void draw_asteroid( Asteroid *asteroid )
{
  asteroid->draw();
  asteroid->check_boundaries( g_window_ratio[1], g_window_ratio[0] );
}

/* Function to react to the pressing of keyboard keys by  */
/* the user, by resetting or pausing the animated action. */
void key_press(unsigned char key, int mouse_x, int mouse_y)
{
  switch( tolower( key ) )
	{
    case 'r': 
      init_main();
      break;

		case 'p':
      // If one is paused, tehy should all be paused, so unpause all of them
      if( g_is_paused )//g_asteroids.getHeadValue()->is_paused )
      {
        g_asteroids.each( unpause_asteroid );
        g_particles.each( unpause_particles );
      }
      else
      {
        g_asteroids.each( pause_asteroid );
        g_particles.each( pause_particles );
      }

      g_is_paused = !g_is_paused;

      break; 
	}
}

// Functions to pause/unpause an asteroid or particle system
void pause_asteroid( Asteroid *asteroid )
{
  asteroid->is_paused = true;
}

void unpause_asteroid( Asteroid *asteroid )
{
  asteroid->is_paused = false;
}

void pause_particles( ParticleSystem *particles )
{
  particles->is_paused = true;
}

void unpause_particles( ParticleSystem *particles )
{
  particles->is_paused = false;
}

/* Function to react to selection from the pop-up    */
/* menu, by resizing or recoloring the frozen stars. */
void menu( int selection )
{
	switch( selection )
	{
		case 1: 
      key_press( 'p', 0, 0 );
      break;

		case 2: 
      key_press( 'r', 0, 0 );
      break;
	}

	// After processing the menu action, redraw the image without the menu.
	//glutPostRedisplay();
}

void tick( int value )
{
	glutPostRedisplay();
	glutTimerFunc(50, tick, 1);
}

/* Window-reshaping routine, to scale the rendered scene according */
/* to the window dimensions, setting the global variables so the   */
/* mouse operations will correspond to mouse pointer positions.    */
void resize_window( GLsizei w, GLsizei h )
{
	glViewport(0, 0, w, h);
	g_current_window_size[0] = w;
	g_current_window_size[1] = h;
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (w <= h)
	{
		g_window_ratio[0] = 2.0;
		g_window_ratio[1] = 2.0 * (GLfloat)h / (GLfloat)w;
    
    glOrtho(-1.0, 1.0, -1.0 * (GLfloat)h / (GLfloat)w, (GLfloat)h / (GLfloat)w, -10.0, 10.0);
	}
  else
	{
		g_window_ratio[0] = 2.0 * (GLfloat)w / (GLfloat)h;
		g_window_ratio[1] = 2.0;
    
    glOrtho(-1.0 * (GLfloat)w / (GLfloat)h, (GLfloat)w / (GLfloat)h, -1.0, 1.0, -10.0, 10.0);
	}
  
  glMatrixMode(GL_MODELVIEW);
}

void garbage_collect_asteroids( Asteroid *asteroid )
{
  garbage_collect( asteroid );
}

void garbage_collect_particles( ParticleSystem *particles )
{
  garbage_collect( particles );
}
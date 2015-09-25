/*
** wolf.c for main in /home/aubess_t/work/wolf3D
**
** Made by au
** Login   <aubess_t@epitech.net>
**
** Started on  Thu Dec 10 00:39:33 2014 au
** Last update Fri Feb 28 16:31:42 2014 au
*/

/*****************************************************************************
 * INCLUDES
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>


/*****************************************************************************
 * DEFINES
 ****************************************************************************/

#define		max(a,b)		((a)>(b)?(a):(b))
#define		min(a,b)		((a)<(b)?(a):(b))
#define		setbit(n,b)		((n) |= (b))
#define		clearbit(n,b)		((n) &= ~(b))

#define		DBL_MAX			(1.7976931348623158e+308)

#define		MRAND_MAX		(0x7fff)

#ifndef M_PI
#define		M_PI			(3.14159265358979323846)
#endif

#define		SCREEN_WIDTH		(1920)
#define		SCREEN_HEIGHT		(1024)
#define		SCREEN_DEPTH		(32)	// bits

#define		DEFAULT_FPS		(60)
#define		DEFAULT_MAZE_WIDTH	(32)	// min 3
#define		DEFAULT_MAZE_HEIGHT	(32)	// min 3
#define		DEFAULT_MAZE_DENSITY	(10)	// min 0, max 100
#define		MODE_CUBE		(0)
#define		MODE_FACE		(1)
#define		DEFAULT_MODE		(MODE_CUBE)
#define		TILE_SIZE		(64)
#define		WALL_HEIGHT		(128)
#define		cell(x,y)		(gGame.maze.map[((y) * (gGame.maze.size.width+2)) + (x)])
#define		COLLIDE_NO		(0)
#define		COLLIDE_WALL		(1)
#define		COLLIDE_EXIT		(2)

#define		WALL_LEFT_MSK		(1<<0)
#define		WALL_TOP_MSK		(1<<1)
#define		WALLS_MSK		(3<<0)
#define		ENEMY_MSK		(7<<2)
#define		OBJECT_MSK		(7<<5)

typedef struct	s_pos
{
  int		x;
  int		y;
}		pos;

typedef struct	s_dpos {
  double	x;
  double	y;
}		dpos;

typedef struct	s_size {
  int		width;
  int		height;
}		size;

typedef struct	s_dsize {
  double	width;
  double	height;
}		dsize;

typedef struct	s_rect {
  int		x;
  int		y;
  int		width;
  int		height;
}		rect;

typedef struct	s_mapView {
  int		marginLeft;
  int		marginRight;
  int		marginTop;
  int		marginBottom;
  pos		offset;
  size		size;
  dsize		cell;
}		mapView;

typedef struct	s_d3View {
  int		marginLeft;
  int		marginRight;
  int		marginTop;
  int		marginBottom;
  pos		offset;
  size		size;
  pos		center;
  int		rayWidth;
  int		focal;
}		d3View;

typedef struct	s_player {
  int		arc;
  pos		positionMap;
  pos		position3d;
  int		speed;
}		player;

typedef struct	s_maze {
  size		size;			// min 3x3
  int		density;		// min 0, max 100
  unsigned char	*map;
  pos		exit;
}		maze;

typedef struct	s_framerate {
  Uint32	framecount;
  float		rateticks;
  Uint32	baseticks;
  Uint32	lastticks;
  Uint32	rate;
  Uint32	time_passed;
}		framerate;

typedef struct	s_surfaces {
  SDL_Surface*	screen;
  SDL_Surface*	mapView;
  SDL_Surface*	d3View;
  SDL_Surface*	framerate;
  SDL_Surface*	density;
  SDL_Surface*	mapSize;
} surfaces;

typedef struct	s_game {
  int		mode;
  int		separator_x;
  int		FPS;
  framerate	framerate;
  surfaces	surfaces;
  TTF_Font	*font;
  mapView	mapView;
  d3View	d3View;
  maze		maze;
  player	player;
  int		moveSeparator;
  int		moveDensity;
  int		moveMapWidth;
  int		moveMapHeight;
  int		rotatePlayer;
  int		movePlayer;
  int		showMatrix;
  int		changeMode;
  // trigonometric tables
  double	*sin;
  double	*iSin;
  double	*cos;
  double	*iCos;
  double	*tan;
  double	*iTan;
  double	*fishEye;
  double	*stepX;
  double	*stepY;
}		game;


/*****************************************************************************
 * GLOBALS
 ****************************************************************************/

game		gGame;

int		ANGLE60;
int		ANGLE30;
int		ANGLE15;
int		ANGLE90;
int		ANGLE180;
int		ANGLE270;
int		ANGLE360;
int		ANGLE0;
int		ANGLE2;
int		ANGLE5;
int		ANGLE10;


/*****************************************************************************
 * FUNCTIONS DEBUG
 ****************************************************************************/

void consoleErr(char* str)
{
  fprintf(stderr, "%s: %s\n", str, SDL_GetError());
  fflush(stderr);
}

void consoleOut(char* str)
{
  fprintf(stdout, "%s: %s\n", str, SDL_GetError());
  fflush(stdout);
}


/*****************************************************************************
 * FUNCTIONS SDL
 ****************************************************************************/

void	setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  Uint8	*p = (Uint8*) surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
  *(Uint32 *)p = pixel;
}

Uint32	getPixel(SDL_Surface *surface, int x, int y)
{
  Uint8	*p = (Uint8*) surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
  return (*(Uint32 *)p);
}

void		drawCircle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel)
{
  double	error = (double)-radius;
  double	x = (double)radius - 0.5;
  double	y = (double)0.5;
  double	cx = n_cx - 0.5;
  double	cy = n_cy - 0.5;

  while (x >= y)
    {
      setPixel(surface, (int) (cx + x), (int) (cy + y), pixel);
      setPixel(surface, (int) (cx + y), (int) (cy + x), pixel);
      if (x != 0) {
	setPixel(surface, (int) (cx - x), (int) (cy + y), pixel);
	setPixel(surface, (int) (cx + y), (int) (cy - x), pixel);
      }
      if (y != 0) {
	setPixel(surface, (int) (cx + x), (int) (cy - y), pixel);
	setPixel(surface, (int) (cx - y), (int) (cy + x), pixel);
      }
      if (x != 0 && y != 0) {
	setPixel(surface, (int) (cx - x), (int) (cy - y), pixel);
	setPixel(surface, (int) (cx - y), (int) (cy - x), pixel);
      }
      error += y;
      ++y;
      error += y;
      if (error >= 0) {
	--x;
	error -= x;
	error -= x;
      }
    }
}

void fillCircle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel)
{
  int		BPP = surface->format->BytesPerPixel;
  double	r = (double)radius;
  double	dy;

  for (dy = 0; dy <= r; dy += 1.0) {
    double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
    int x = (int)(cx - dx);
    Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
    Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;
    for (; x <= cx + dx; x++) {
      *(Uint32 *)target_pixel_a = pixel;
      *(Uint32 *)target_pixel_b = pixel;
      target_pixel_a += BPP;
      target_pixel_b += BPP;
    }
  }
}

void	drawLine(SDL_Surface *surface, int x0, int y0, int x1, int y1, Uint32 pixel)
{
  int	dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int	dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int	err = (dx > dy ? dx : -dy) / 2, e2;

  for (;;) {
    setPixel(surface, x0, y0, pixel);
    if (x0 == x1 && y0 == y1) break;
    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }
}

void drawShadedLine(SDL_Surface *surface, int x0, int y0, int x1, int y1, int len)
{
  int	dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int	dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int	err = (dx > dy ? dx : -dy) / 2, e2;

  Uint32 pix;
  signed short a, r, v, b;
  double p = 10;
  double dp = (double)p/(double)len;

  for (;;) {
    pix = getPixel(surface, x0, y0);
    a = pix >> 24 & 0xff;
    r = pix >> 16 & 0xff;
    v = pix >> 8 & 0xff;
    b = pix >> 0 & 0xff;
    r=r-p;
    v=v-p;
    b=b-p;
    if (p>0) p-=dp;
    if (r<0) r=0;
    if (v<0) v=0;
    if (b<0) b=0;
    pix = ((a) << 24) | ((r) << 16) | ((v) << 8) | ((b) << 0);
    setPixel(surface, x0, y0, pix);
    if (x0 == x1 && y0 == y1) break;
    e2 = err;
    if (e2 > -dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
  }
}

void drawBox(SDL_Surface *surface, int x, int y, int w, int h, Uint32 pixel)
{
  drawLine(surface, x, y, x+w, y, pixel); 
  drawLine(surface, x, y, x, y+h, pixel);
  drawLine(surface, x+w, y, x+w, y+h, pixel);
  drawLine(surface, x, y+h, x+w, y+h, pixel);
}

int createRGBSurface(SDL_Surface** surface, int width, int height)
{
  int ret;

  if (*surface != NULL) SDL_FreeSurface(*surface);
  *surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
				  width, height, SCREEN_DEPTH,
				  gGame.surfaces.screen->format->Rmask,
				  gGame.surfaces.screen->format->Gmask,
				  gGame.surfaces.screen->format->Bmask,
				  gGame.surfaces.screen->format->Amask);
  if ((ret = (*surface == NULL ? -1 : 0)) != 0)
    consoleErr("Unable to create RGB surface");
  return (ret);
}

int apply_surface(int x, int y, SDL_Surface* source, SDL_Rect* clip)
{
  SDL_Rect offset;

  if (source == NULL)
    return (-1);
  offset.x = x;
  offset.y = y;
  return (SDL_BlitSurface(source, clip, gGame.surfaces.screen, &offset));
}

int fillRect(SDL_Surface* surface, SDL_Rect *rect, Uint8 r, Uint8 g, Uint8 b)
{
  int ret;

  if (surface == NULL)
    return (-1);
  if ((ret = SDL_FillRect(surface, rect, SDL_MapRGB(surface->format, r,g,b))) < 0 )
    consoleErr("SDL_FillRect failed");
  return (ret);
}

int initSDL()
{
  int ret;

  if ((ret = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE)) != 0) {
    consoleErr("Unable to init SDL");
    return (ret);
  }
  atexit(SDL_Quit);  
  if ((ret = TTF_Init()) != 0) {
    consoleErr("TTF_Init failed");
    return (ret);
  }
  if ((ret = ((gGame.font = TTF_OpenFont("assets/Roboto-Regular.ttf", 16)) == NULL?-1:0)) != 0) {
    consoleErr("TTF_OpenFont failed");
    return (ret);
  }
  return (ret);
}

int initVideo()
{
  int ret;

  ret = (gGame.surfaces.screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_HWSURFACE|SDL_DOUBLEBUF)) == NULL ? -1 : 0;
  if
    (ret < 0) consoleErr("Unable to set video mode");
  else
    SDL_WM_SetCaption("Wolf3D aubess_t", NULL);
  return (ret);
}

int flip()
{
  int ret;

  if ((ret = SDL_Flip(gGame.surfaces.screen)) < 0)
    consoleErr("SDL_Flip failed");
  return (ret);
}


/*****************************************************************************
 * FUNCTIONS GAME
 ****************************************************************************/

double arcToRad(double arcAngle)
{
  return ((double) (arcAngle * M_PI) / (double) ANGLE180);
}

void deleteTables()
{
  if (gGame.sin != NULL)	{ free(gGame.sin);	gGame.sin = NULL; }
  if (gGame.iSin != NULL)	{ free(gGame.iSin);	gGame.iSin = NULL; }
  if (gGame.cos != NULL)	{ free(gGame.cos);	gGame.cos = NULL; }
  if (gGame.iCos != NULL)	{ free(gGame.iCos);	gGame.iCos = NULL; }
  if (gGame.tan != NULL)	{ free(gGame.tan);	gGame.tan = NULL; }
  if (gGame.iTan != NULL)	{ free(gGame.iTan);	gGame.iTan = NULL; }
  if (gGame.fishEye != NULL)	{ free(gGame.fishEye);	gGame.fishEye = NULL; }
  if (gGame.stepX != NULL)	{ free(gGame.stepX);	gGame.stepX = NULL; }
  if (gGame.stepY != NULL)	{ free(gGame.stepY);	gGame.stepY = NULL; }
}

int createTables()
{
  int i;
  double radian;

  deleteTables();

  gGame.sin		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));
  gGame.iSin		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));
  gGame.cos		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));
  gGame.iCos		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));
  gGame.tan		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));
  gGame.iTan		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));
  gGame.fishEye		= (double*)malloc(sizeof(double) * ( ANGLE60 + 1));
  gGame.stepX		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));
  gGame.stepY		= (double*)malloc(sizeof(double) * (ANGLE360 + 1));

  if (gGame.sin==NULL || gGame.iSin==NULL || gGame.cos==NULL || gGame.iCos==NULL || gGame.tan==NULL || gGame.iTan==NULL || gGame.fishEye==NULL || gGame.stepX==NULL || gGame.stepY==NULL) {
    consoleErr("createTables() malloc failed");
    return (-1);
  }

  for (i = 0; i <= ANGLE360; i++)
    {
      radian		= arcToRad(i) + (double)(0.0001);	//avoid division by 0
      gGame.sin[i]	= sin(radian);
      gGame.iSin[i]	= (double)1 / (gGame.sin[i]);
      gGame.cos[i]	= cos(radian);
      gGame.iCos[i]	= (double)1 / (gGame.cos[i]);
      gGame.tan[i]	= tan(radian);
      gGame.iTan[i]	= (double)1 / gGame.tan[i];

      // you can see that the distance between xi is the same
      // if we know the angle
      //  _____|_/next xi______________
      //       |
      //  ____/|next xi_________ slope = tan = height / dist between xi's
      //     / |
      //  __/__|_________ dist between xi = height/tan where height=tile size
      // old xi|
      //                distance between xi = x_step[view_angle];
      //

      // facing left
      if (i >= ANGLE90 && i < ANGLE270) {
	gGame.stepX[i] = TILE_SIZE / gGame.tan[i];
	if (gGame.stepX[i] > 0)
	  gGame.stepX[i] = -gGame.stepX[i];
      }

      // facing right
      else {
	gGame.stepX[i] = TILE_SIZE / gGame.tan[i];
	if (gGame.stepX[i] < 0)
	  gGame.stepX[i] = -gGame.stepX[i];
      }
      
      // FACING DOWN
      if (i >= ANGLE0 && i < ANGLE180) {
	gGame.stepY[i] = TILE_SIZE * gGame.tan[i];
	if (gGame.stepY[i] < 0)
	  gGame.stepY[i] = -gGame.stepY[i];
      }

      // FACING UP
      else {
	gGame.stepY[i] = TILE_SIZE * gGame.tan[i];
	if (gGame.stepY[i] > 0)
	  gGame.stepY[i] = -gGame.stepY[i];
      }
    }

  for (i = -ANGLE30; i <= ANGLE30; i++) {
    radian = arcToRad(i);
    // we don't have negative angle, so make it start at 0
    // this will give range 0 to 320
    gGame.fishEye[i + ANGLE30] = 1.0 / cos(radian);
  }
  return (0);
}

void deleteMaze()
{
  if (gGame.maze.map != NULL)
    {
      free(gGame.maze.map);
      gGame.maze.map = NULL;
    }
}

int createMaze()
{
  deleteMaze();
  gGame.maze.map = (unsigned char*)malloc((gGame.maze.size.width+2) * (gGame.maze.size.height+2) * sizeof(unsigned char));
  if (gGame.maze.map==NULL)
    {
      consoleErr("createMaze() malloc failed");
      return (-1);
    }
  memset(gGame.maze.map, 0, (gGame.maze.size.width+2) * (gGame.maze.size.height+2) * sizeof(unsigned char));
  return (0);
}

/*
 * cell :   7  6  5   4  3  2   1   0
 *        ииииииииииииииииииииииииииииииииииииииииииии
 *        | objects | enemies | walls |
 *
 * walls encoding :       .     .
 *                        .     .
 *                   ......_____.____
 *                        |  2  |  2
 *                        |1    |1
 *                   .....|_____|____
 *                        |  2  |  2
 *                        |1    |1
 */
int fillMaze()
{
  int		i, x, y;
  int		density;
  unsigned char	cell8;
  
  maze		*maze	= &gGame.maze;
  int		w	= maze->size.width;
  int		h	= maze->size.height;
  
  // close all walls in viewable area
  for (x = 1; x < w+1; x++)
    for (y = 1; y < h+1; y++)
      cell(x,y) = WALLS_MSK; // bit 0 and 1 (left + top)

  // randomly open walls in viewable area
  if (gGame.mode == MODE_FACE)
    {
      density = (w * h * 2) * (100-maze->density) / 100;
      while (density)
	{
	  x = (rand() & MRAND_MAX) * w / MRAND_MAX; // rand_max 0x7fff
	  y = (rand() & MRAND_MAX) * h / MRAND_MAX;
	  cell8 = cell(x+1,y+1);
	  if (rand() & 1)
	    { // left or top
	      if (cell8 & WALL_LEFT_MSK)
		{
		  clearbit(cell8, WALL_LEFT_MSK);
		  density--;
		}
	    }
	  else
	    if (cell8 & WALL_TOP_MSK)
	      {
		clearbit(cell8, WALL_TOP_MSK);
		density--;
	      }
	  cell(x+1,y+1) = cell8;
	}

      // Force close maze borders (close the viewable area)
      for (x=1; x<maze->size.width+1; x++)	setbit(cell(x,1), WALL_TOP_MSK);
      for (y=1; y<maze->size.height+1; y++)	setbit(cell(1,y), WALL_LEFT_MSK);
      for (x=1; x<maze->size.width+1; x++)	setbit(cell(x,maze->size.height+1), WALL_TOP_MSK);
      for (y=1; y<maze->size.height+1; y++)	setbit(cell(maze->size.width+1,y), WALL_LEFT_MSK);

      // Exit wall
      switch (rand() & 3) { // random side
      case 0 : //left
	maze->exit.x = 1;
	maze->exit.y = ((rand() & MRAND_MAX) * (h-4) / MRAND_MAX) +2;
	clearbit(cell(maze->exit.x, maze->exit.y) ,WALL_TOP_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y), WALL_LEFT_MSK);
	clearbit(cell(maze->exit.x, maze->exit.y+1), WALL_TOP_MSK);	break;
      case 1 : // top
	maze->exit.x = ((rand() & MRAND_MAX) * (w-4)  / MRAND_MAX) +2;
	maze->exit.y = 1;
	clearbit(cell(maze->exit.x, maze->exit.y), WALL_LEFT_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y), WALL_LEFT_MSK);
	clearbit(cell(maze->exit.x, maze->exit.y+1), WALL_TOP_MSK);	break;
      case 2 : // right
	maze->exit.x = w;
	maze->exit.y = ((rand() & MRAND_MAX) * (h-4) / MRAND_MAX) +2;
	clearbit(cell(maze->exit.x, maze->exit.y), WALLS_MSK);
	clearbit(cell(maze->exit.x, maze->exit.y+1), WALL_TOP_MSK);	break;
      case 3 : // bottom
	maze->exit.x = ((rand() & MRAND_MAX) * (w-4)  / MRAND_MAX) +2;
	maze->exit.y = h;
	clearbit(cell(maze->exit.x, maze->exit.y), WALLS_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y), WALL_LEFT_MSK);	break;
      }

    // enemies
      for (i=1; i<=7;) {
	x = ((rand() & MRAND_MAX) * w  / MRAND_MAX);
	y = ((rand() & MRAND_MAX) * h / MRAND_MAX);
	if ((cell(x+1, y+1) & (63<<2)) == 0) {
	  setbit(cell(x+1, y+1), i<<2);
	  i++;
	}
      }

    // objects
      for (i=1; i<=7;) {
	x = ((rand() & MRAND_MAX) * w  / MRAND_MAX);
	y = ((rand() & MRAND_MAX) * h / MRAND_MAX);
	if ((cell(x+1, y+1) & (63<<2)) == 0) {
	  setbit(cell(x+1, y+1), i<<5);
	  i++;
	}
      }
  }

  if ( gGame.mode == MODE_CUBE)
    {
      density = (w * h) * (100-maze->density) / 100;
      while (density)
	{
	  x = (rand() & MRAND_MAX) * w / MRAND_MAX; // rand_max 0x7fff
	  y = (rand() & MRAND_MAX) * h / MRAND_MAX;
	  cell8 = cell(x+1,y+1);
	  if (cell8 & WALLS_MSK) {
	    clearbit(cell8, WALLS_MSK);
	    density--;
	  }
	  cell(x+1,y+1) = cell8;
	}

      // Force close maze borders (close the viewable area)
      for (x=1; x<maze->size.width+1; x++)	setbit(cell(x,1), WALLS_MSK);
      for (y=1; y<maze->size.height+1; y++)	setbit(cell(1,y), WALLS_MSK);
      for (x=1; x<maze->size.width+1; x++)	setbit(cell(x,maze->size.height), WALLS_MSK);
      for (y=1; y<maze->size.height+1; y++)	setbit(cell(maze->size.width,y), WALLS_MSK);

      // Exit wall
      switch ( rand() & 3) { // random side
      case 0 : //left
	maze->exit.x = 1;
	maze->exit.y = ( (rand() & MRAND_MAX) * (h-6) / MRAND_MAX) +3;
	setbit(cell(maze->exit.x, maze->exit.y) ,WALLS_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y-1), WALLS_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y  ), WALLS_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y+1), WALLS_MSK);	break;
      case 1 : // top
	maze->exit.x = ( (rand() & MRAND_MAX) * (w-6)  / MRAND_MAX) +3;
	maze->exit.y = 1;
	setbit(cell(maze->exit.x, maze->exit.y), WALLS_MSK);
	clearbit(cell(maze->exit.x-1, maze->exit.y+1), WALLS_MSK);
	clearbit(cell(maze->exit.x  , maze->exit.y+1), WALLS_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y+1), WALLS_MSK);	break;
      case 2 : // right
	maze->exit.x = w;
	maze->exit.y = ( (rand() & MRAND_MAX) * (h-6) / MRAND_MAX) +3;
	setbit(cell(maze->exit.x, maze->exit.y), WALLS_MSK);
	clearbit(cell(maze->exit.x-1, maze->exit.y-1), WALLS_MSK);
	clearbit(cell(maze->exit.x-1, maze->exit.y  ), WALLS_MSK);
	clearbit(cell(maze->exit.x-1, maze->exit.y+1), WALLS_MSK);	break;
      case 3 : // bottom
	maze->exit.x = ( (rand() & MRAND_MAX) * (w-6)  / MRAND_MAX) +3;
	maze->exit.y = h;
	setbit(cell(maze->exit.x, maze->exit.y), WALLS_MSK);
	clearbit(cell(maze->exit.x-1, maze->exit.y-1), WALL_LEFT_MSK);
	clearbit(cell(maze->exit.x  , maze->exit.y-1), WALL_LEFT_MSK);
	clearbit(cell(maze->exit.x+1, maze->exit.y-1), WALL_LEFT_MSK);	break;
      }

      // enemies
      for (i=1; i<=7;) {
	x = ( (rand() & MRAND_MAX) * w  / MRAND_MAX);
	y = ( (rand() & MRAND_MAX) * h / MRAND_MAX);
	if (cell(x+1, y+1) == 0) {
	  setbit(cell(x+1, y+1), i<<2);
	  i++;
	}
      }

      // objects
      for (i=1; i<=7;) {
	x = ( (rand() & MRAND_MAX) * w  / MRAND_MAX);
	y = ( (rand() & MRAND_MAX) * h / MRAND_MAX);
	if (cell(x+1, y+1) == 0) {
	  setbit(cell(x+1, y+1), i<<5);
	  i++;
	}
      }
    }
  return (0);
}


int resizeMaze()
{
  if (createMaze() != 0)
    return (-1);
  return (fillMaze());
}

void setPlayerPositionMap()
{
  player	*player	 = &gGame.player;
  mapView	*mapview = &gGame.mapView;

  player->positionMap.x	= (int) (((double) player->position3d.x / (double) TILE_SIZE) * mapview->cell.width);
  player->positionMap.y	= (int) (((double) player->position3d.y / (double) TILE_SIZE) * mapview->cell.height);
}

int resetGame()
{
  maze		*maze		= &gGame.maze;
  player	*player		= &gGame.player;
  framerate	*framerate 	= &gGame.framerate;

  int		cellx, celly, x, y;

  if (createMaze() != 0)
    return (-1);

  fillMaze();

  if (gGame.mode == MODE_FACE)
    {
      player->position3d.x	= (int)((rand() & MRAND_MAX) * ((maze->size.width-4)*TILE_SIZE) / MRAND_MAX) + (TILE_SIZE*2);
      player->position3d.y	= (int)((rand() & MRAND_MAX) * ((maze->size.height-4)*TILE_SIZE) / MRAND_MAX) + (TILE_SIZE*2);
    }
  if (gGame.mode == MODE_CUBE)
    {
      while (1)
	{
	  player->position3d.x	= (int)((rand() & MRAND_MAX) * ((maze->size.width-4)*TILE_SIZE) / MRAND_MAX) + (TILE_SIZE*2);
	  player->position3d.y	= (int)((rand() & MRAND_MAX) * ((maze->size.height-4)*TILE_SIZE) / MRAND_MAX) + (TILE_SIZE*2);
	  cellx = player->position3d.x / TILE_SIZE;
	  celly = player->position3d.y / TILE_SIZE;
	  if (cell(cellx, celly) == 0)
	    break;
	}
    }

  setPlayerPositionMap();

  // clear walls around player
  cellx = player->position3d.x / TILE_SIZE;
  celly = player->position3d.y / TILE_SIZE;
  if (gGame.mode == MODE_FACE)
    {
      clearbit(cell(cellx, celly), WALLS_MSK);
      clearbit(cell(cellx+1, celly), WALL_LEFT_MSK);
      clearbit(cell(cellx, celly+1), WALL_TOP_MSK);
    }

  if  (gGame.mode == MODE_CUBE)
    for (x=-1; x<=1; x++)
      for (y=-1; y<=1; y++)
	clearbit(cell(cellx+x, celly+y), WALLS_MSK);

  framerate->framecount = 0;
  framerate->rate = gGame.FPS;
  framerate->rateticks = ((float)1000 / (float)gGame.FPS);
  framerate->baseticks = SDL_GetTicks();
  framerate->lastticks = framerate->baseticks;

  return (0);
}

int resizeViews()
{
  mapView	*mapview	= &gGame.mapView;
  d3View	*d3view		= &gGame.d3View;
  maze		*maze		= &gGame.maze;
  player	*player		= &gGame.player;
  int		oldANGLE360	= ANGLE360==0?1:ANGLE360;

  mapview->offset.x		= mapview->marginLeft;
  mapview->offset.y		= mapview->marginTop;
  mapview->size.width		= max(64,gGame.separator_x - mapview->marginLeft - mapview->marginRight);
  mapview->size.height		= min(mapview->size.width, SCREEN_HEIGHT - mapview->marginTop - mapview->marginBottom);
  mapview->cell.width		= (double)(mapview->size.width) / (double)(maze->size.width);
  mapview->cell.height		= (double)(mapview->size.height) / (double)(maze->size.height);

  d3view->offset.x		= gGame.separator_x+d3view->marginLeft;
  d3view->offset.y		= d3view->marginTop;
  d3view->size.width		= max(64,SCREEN_WIDTH - gGame.separator_x - d3view->marginLeft - d3view->marginRight);
  d3view->size.height		= min(d3view->size.width, SCREEN_HEIGHT - d3view->marginTop - d3view->marginBottom);
  d3view->center.x		= d3view->size.width / 2;
  d3view->center.y		= d3view->size.height / 2;

  ANGLE60			= (d3view->size.width);
  ANGLE30			= (ANGLE60 / 2);
  ANGLE15			= (ANGLE30 / 2);
  ANGLE90			= (ANGLE30 * 3);
  ANGLE180			= (ANGLE90 * 2);
  ANGLE270			= (ANGLE90 * 3);
  ANGLE360			= (ANGLE60 * 6);
  ANGLE0			= (0);
  ANGLE2			= (ANGLE30 / 15);
  ANGLE5			= (ANGLE30 / 6);
  ANGLE10			= (ANGLE5 * 2);

  if (createTables() != 0)
    return (-1);

  d3view->focal			= (int)((double)d3view->center.x / gGame.tan[ANGLE30]);
  player->arc			= player->arc * ANGLE360/oldANGLE360;

  setPlayerPositionMap();

  return (0);
}

int initGame()
{
  surfaces	*surfaces	= &gGame.surfaces;
  mapView	*mapview	= &gGame.mapView;
  d3View	*d3view		= &gGame.d3View;
  maze		*maze		= &gGame.maze;
  player	*player		= &gGame.player;

  gGame.mode			= DEFAULT_MODE;
  gGame.separator_x		= SCREEN_WIDTH/2;
  gGame.FPS			= DEFAULT_FPS;
  gGame.moveSeparator		= 0;
  gGame.moveDensity		= 0;
  gGame.moveMapWidth		= 0;
  gGame.moveMapHeight		= 0;
  gGame.rotatePlayer		= 0;
  gGame.movePlayer		= 0;
  gGame.sin			= NULL;
  gGame.iSin			= NULL;
  gGame.cos			= NULL;
  gGame.iCos			= NULL;
  gGame.tan			= NULL;
  gGame.iTan			= NULL;
  gGame.fishEye			= NULL;
  gGame.stepX			= NULL;
  gGame.stepY			= NULL;
  gGame.showMatrix		= 0;
  gGame.changeMode		= 0;

  surfaces->framerate		= NULL;
  surfaces->mapView		= NULL;
  surfaces->d3View		= NULL;
  surfaces->density		= NULL;
  surfaces->mapSize		= NULL;

  mapview->marginLeft		= 16;
  mapview->marginRight		= 16;
  mapview->marginTop		= 16;
  mapview->marginBottom		= 16;

  d3view->marginLeft		= 16;
  d3view->marginRight		= 16;
  d3view->marginTop		= 16;
  d3view->marginBottom		= 16;
  d3view->rayWidth		= 1;

  maze->map			= NULL;
  maze->size.width		= DEFAULT_MAZE_WIDTH;	// min 3, max MAPVIEW_WIDTH/4
  maze->size.height		= DEFAULT_MAZE_HEIGHT;	// min 3, max MAPVIEW_HEIGHT/4
  maze->density			= DEFAULT_MAZE_DENSITY;	// min 0, max 100

  player->speed			= 4;

  srand((unsigned int)time(NULL));

  if (resizeViews() != 0)	return (-1);
  if (resetGame() != 0)		return (-1);

  return (0);
}

void drawWalls()
{
  int		mx, my;
  int		x, y;
  unsigned char	cell;

  maze		*maze		= &gGame.maze;
  SDL_Surface	*surface	= gGame.surfaces.mapView;
  mapView	*mapview	= &gGame.mapView;
  double	cw		= mapview->cell.width;
  double	ch		= mapview->cell.height;

  if (gGame.showMatrix) {
    for (mx=0; mx<maze->size.width+2; mx++) {
      for (my=0; my<maze->size.height+2; my++) {
	x = (float)mx * cw;
	y = (float)my * ch;
	cell = cell(mx, my);
	if 	(gGame.mode == MODE_FACE) {
	  drawLine(surface, x, y, x, y+ch-1, cell & WALL_LEFT_MSK ? 0xffffff : 0x404040);
	  drawLine(surface, x, y, x+cw-1, y, cell & WALL_TOP_MSK ? 0xffffff : 0x404040);
	}
	if (gGame.mode == MODE_CUBE)	drawBox(surface, x, y, cw-1, cw-1, cell & WALLS_MSK ? 0xffffff : 0x404040);
      }
    }
  } else {
    for (mx=0; mx<maze->size.width+2; mx++) {
      for (my=0; my<maze->size.height+2; my++) {
	x = (float)mx * cw;
	y = (float)my * ch;
	cell = cell(mx, my);
	if (gGame.mode == MODE_FACE) {
	  if (cell & WALL_LEFT_MSK)	drawLine(surface, x, y, x, y+ch-1, 0xffffff);
	  if (cell & WALL_TOP_MSK)	drawLine(surface, x, y, x+cw-1, y, 0xffffff);
	}
	if (gGame.mode == MODE_CUBE)
	  if (cell & WALLS_MSK)		drawBox(surface, x, y, cw-1, ch-1, 0xffffff);
      }
    }
  }
}

void drawExit()
{
  maze		*maze		= &gGame.maze;
  SDL_Surface	*surface	= gGame.surfaces.mapView;
  mapView	*mapview	= &gGame.mapView;
  double	cw		= mapview->cell.width;
  double	ch		= mapview->cell.height;

  if (gGame.mode == MODE_FACE) {
    if (maze->exit.x == 1)			drawLine(surface, cw, (float)(maze->exit.y*ch), cw, (float)(maze->exit.y*ch)+ch-1, 0xff0000);
    if (maze->exit.y == 1)			drawLine(surface, (float)(maze->exit.x*cw), ch, (float)(maze->exit.x*cw)+cw-1, ch, 0xff0000);
    if (maze->exit.x == maze->size.width)	drawLine(surface, (maze->size.width+1)*cw, (float)(maze->exit.y*ch), (maze->size.width+1)*cw, (float)(maze->exit.y*ch)+ch-1, 0xff0000);
    if (maze->exit.y == maze->size.height)	drawLine(surface, (float)(maze->exit.x*cw), (maze->size.height+1)*ch, (float)(maze->exit.x*cw)+cw-1, (maze->size.height+1)*ch, 0xff0000);
  }
  if (gGame.mode == MODE_CUBE) {
    if (maze->exit.x == 1)			drawBox(surface, cw, (float)(maze->exit.y*ch), cw-1, ch-1, 0xff0000);
    if (maze->exit.y == 1)			drawBox(surface, (float)(maze->exit.x*cw), ch, cw-1, ch-1, 0xff0000);
    if (maze->exit.x == maze->size.width)	drawBox(surface, (float)(maze->exit.x*cw), (float)(maze->exit.y*ch), cw-1, ch-1, 0xff0000);
    if (maze->exit.y == maze->size.height)	drawBox(surface, (float)(maze->exit.x*cw), (float)(maze->exit.y*ch), cw-1, ch-1, 0xff0000);
  }
}

void drawPlayer()
{
  SDL_Surface	*surface	= gGame.surfaces.mapView;
  mapView	*mapview	= &gGame.mapView;
  player	*player		= &gGame.player;
  double	cw		= mapview->cell.width;
  double	ch		= mapview->cell.height;

  fillCircle(surface, player->positionMap.x, player->positionMap.y, min(8,min(cw,ch)/2), 0xffff00);
}

void drawObjects()
{
  int		mx, my;
  int		x, y;
  unsigned char	cell;
  maze		*maze		= &gGame.maze;
  SDL_Surface	*surface	= gGame.surfaces.mapView;
  mapView	*mapview	= &gGame.mapView;
  double	cw		= mapview->cell.width;
  double	ch		= mapview->cell.height;

  for (mx=0; mx<maze->size.width; mx++)
    {
      for (my=0; my<maze->size.height; my++)
	{
	  cell = cell(mx, my);
	  x = mx * cw;
	  y = my * ch;
	  if (cell & ENEMY_MSK) {
	    fillCircle(surface, x+(cw/2), y+(ch/2), min(8,min(cw,ch)/2), (cell & ENEMY_MSK) << 20);
	    drawCircle(surface, x+(cw/2), y+(ch/2), min(8,min(cw,ch)/2), 0xffffff);
	  }
	  if (cell & OBJECT_MSK) {
	    fillCircle(surface, x+(cw/2), y+(ch/2), min(8,min(cw,ch)/2), (cell & OBJECT_MSK) << 8);
	    drawCircle(surface, x+(cw/2), y+(ch/2), min(8,min(cw,ch)/2), 0xffffff);
	  }
	}
    }
}

int processMap()
{
  int		ret;
  SDL_Rect	r;
  SDL_Surface	*surface = gGame.surfaces.mapView;
  mapView	*mapview = &gGame.mapView;

  if (gGame.separator_x == 0)
    return (0);

  ret = createRGBSurface(&gGame.surfaces.mapView, mapview->size.width + (mapview->cell.width*2), mapview->size.height + (mapview->cell.height*2));

  if (ret != 0)
    return (ret);

  surface = gGame.surfaces.mapView;

  // clear
  if (gGame.showMatrix) {
      r.x = 0;
      r.y = 0;
      r.w = surface->w;
      r.h = surface->h;
      fillRect(surface, &r, 0x0, 0x30, 0x0);
      r.x = mapview->cell.width;
      r.y = mapview->cell.height;
      r.w = mapview->size.width;
      r.h = mapview->size.height;
      fillRect(surface, &r, 0x0, 0x20, 0x0);
    } else {
    r.x = mapview->cell.width;
    r.y = mapview->cell.height;
    r.w = mapview->size.width;
    r.h = mapview->size.height;
    fillRect(surface, &r, 0x0, 0x0, 0x0);
  }
  
  drawWalls();
  drawExit();
  drawPlayer();
  drawObjects();
  
  return (ret);
}

void drawBackground()
{
  d3View	*d3view	 = &gGame.d3View;
  SDL_Surface	*surface = gGame.surfaces.d3View;
  double	c;
  int		i;
  SDL_Rect	r;

  // sky
  c = 250;
  r.x = 0;
  r.w = d3view->size.width;
  r.h = 3;
  for (i = 0; i < d3view->center.y; i += r.h, c-=1.5) {
    r.y = i;
    fillRect(surface, &r, (int)c, (int)c, (int)c);
  }

  // ground
  c = 25;
  for (; i < d3view->size.height; i += r.h, c++) {
    r.y = i;
    fillRect(surface, &r, (int)c, 20, 20);
  }
}

void drawMapviewRay(double x, double y)
{
  pos		*playerPositionMap	= &gGame.player.positionMap;
  dsize		*mapviewCell		= &gGame.mapView.cell;
  mapView	*mapview		= &gGame.mapView;

  int x2 = (int) (((double) (x * mapviewCell->width) / (double) TILE_SIZE));
  int y2 = (int) (((double) (y * mapviewCell->height) / (double) TILE_SIZE));

  if (x2<0)
    x2 = 0;
  if (x2>mapview->size.width + (mapview->cell.width*2) - 1)
    x2 = mapview->size.width + (mapview->cell.width*2) - 1;
  if (y2<0)
    y2 = 0;
  if (y2>mapview->size.height + (mapview->cell.height*2) - 1 )
    y2=mapview->size.height + (mapview->cell.height*2) - 1;

  // draw line from the player position to the position where the ray intersect with wall
  drawLine(gGame.surfaces.mapView,
	   playerPositionMap->x,
	   playerPositionMap->y,
	   x2,
	   y2,
	   0x333300);
}

int isExit(int distx, int disty, int cellx, int celly)
{
  maze	*maze	= &gGame.maze;

  if (distx<0) cellx++;
  if (disty<0) celly++;

  return ((cellx==maze->exit.x && celly==maze->exit.y) ? 1 : 0);
}

void draw3dviewSlice(int arc, int ray, double dist, int wallColor)
{
  SDL_Surface	*surface	= gGame.surfaces.d3View;
  maze		*maze		= &gGame.maze;
  d3View	*d3view		= &gGame.d3View;
  int		projectedWallHeight;
  int		topOfWall;		// used to compute the top and bottom of the sliver that
  int		bottomOfWall;		// will be the starting point of floor and ceiling
					// determine which ray strikes a closer wall.
					// if yray distance to the wall is closer, the yDistance will be shorter than the xDistance
  double	maxdist;
  double	ratio;
  SDL_Rect	r;
  SDL_Color	color;

  dist /= gGame.fishEye[ray];		// correct distance (compensate for the fisheye effect)
  projectedWallHeight = (int) (WALL_HEIGHT * (double)d3view->focal / dist);
  bottomOfWall = min(d3view->size.height - 1, d3view->center.y + (int) (projectedWallHeight /2));
  topOfWall = max(0, d3view->center.y - (int) (projectedWallHeight /2));

  maxdist = abs((double)(maze->size.width * TILE_SIZE) * gGame.iCos[arc]);
  ratio = dist*128/maxdist;
  if (wallColor == 0) {
    color.r = (int)((double)160 - ratio);
    color.g = 0; color.b = 0;
  } else {
    color.r = (int)((double)wallColor - ratio);
    color.g = (color.b = (color.r)>>1)<<1;
  }

  r.x = ray;
  r.y = topOfWall;
  r.w = d3view->rayWidth;
  r.h = bottomOfWall - topOfWall;
  fillRect(surface, &r, color.r, color.g, color.b);
}

int rayCollideHorizontal(int disty, int cellx, int celly)
{
  maze	*maze	= &gGame.maze;
  int	cell	= cell(cellx, celly);

  if (gGame.mode == MODE_FACE)
    {
      if (disty<0)					celly++;
      if (cellx==maze->exit.x && celly==maze->exit.y)	return (COLLIDE_EXIT);
      if ((cell & WALL_TOP_MSK) != 0)			return (COLLIDE_WALL);
  } 
  else // if (gGame.mode == MODE_CUBE) {
    {
      if (cellx==maze->exit.x && celly==maze->exit.y)	return (COLLIDE_EXIT);
      if ((cell & WALLS_MSK) != 0)			return (COLLIDE_WALL);
    }
  return (COLLIDE_NO);
}

int rayCollideVertical(int distx, int cellx, int celly)
{
  maze *maze	= &gGame.maze;
  int  cell	= cell(cellx, celly);

  if (gGame.mode == MODE_FACE)
    {
      if (distx<0)					cellx++;
      if (cellx==maze->exit.x && celly==maze->exit.y)	return (COLLIDE_EXIT);
      if ((cell & WALL_LEFT_MSK) != 0)			return (COLLIDE_WALL);
  }
  else // if (gGame.mode == MODE_CUBE) {
    {
      if (cellx==maze->exit.x && celly==maze->exit.y)	return (COLLIDE_EXIT);
      if ((cell & WALLS_MSK) != 0)			return (COLLIDE_WALL);
    }
  return (COLLIDE_NO);
}

void DDAHoriz(int arc, pos *positionMap, dpos *distToWallHit, dpos *intersection, pos *collide)
{
  pos	position3d	= gGame.player.position3d;
  size	size		= gGame.maze.size;
  pos	cell;		// the current cell that the ray is in
  pos	distToNextCell;	// how far to the next bound (this is multiple of tile size) x = horizontal wall, y = vertical wall

  if (arc > ANGLE0 && arc < ANGLE180) // ray is between 0 to 180 degree (1st and 2nd quadrant) -> ray is facing down
    {
      // Truncate then add to get the coordinate of the FIRST grid (horizontal wall) that is in front of the player (this is in pixel unit) ROUND DOWN
      (*positionMap).x	= (position3d.y / TILE_SIZE) * TILE_SIZE + TILE_SIZE;
      // compute distance to the next horizontal wall
      distToNextCell.x	= TILE_SIZE;
      // we can get the vertical distance to that wall by (horizontalGrid-GLplayerY)
      // we can get the horizontal distance to that wall by 1/tan(arc)*verticalDistance
      // find the x interception to that wall
      (*intersection).x = (gGame.iTan[arc] * ((*positionMap).x - position3d.y)) + position3d.x;
    }
  else // else, the ray is facing up
    {
      (*positionMap).x	= (position3d.y / TILE_SIZE) * TILE_SIZE;
      distToNextCell.x	= -TILE_SIZE;
      (*intersection).x	= (gGame.iTan[arc] * ((*positionMap).x - position3d.y)) + position3d.x;
      (*positionMap).x--;
  }

  while (1)
    {
      cell.x = (int) ((*intersection).x / TILE_SIZE);
      cell.y = ((*positionMap).x / TILE_SIZE);

      if ((cell.x >= size.width+2) || (cell.y >= size.height+2) || cell.x < 0 || cell.y < 0) {
	(*distToWallHit).x = DBL_MAX;	// MAX_VALUE;
	break;
      }

      (*collide).x = rayCollideHorizontal(distToNextCell.x, cell.x, cell.y);

      if ((*collide).x != COLLIDE_NO) {
	(*distToWallHit).x = ((*intersection).x - position3d.x) * gGame.iCos[arc];
	break;
      }

      // else, the ray is not blocked, extend to the next block
      (*intersection).x += gGame.stepX[arc];	//distToNextIntersection horiz
      (*positionMap).x += distToNextCell.x;
    }
}

void DDAVert(int arc, pos *positionMap, dpos *distToWallHit, dpos *intersection, pos *collide)
{
  pos	position3d	= gGame.player.position3d;
  size	size		= gGame.maze.size;
  pos	cell;
  pos	distToNextCell;

  if (arc < ANGLE90 || arc > ANGLE270) 	// RAY FACING RIGHT
    {
      (*positionMap).y	= (position3d.x / TILE_SIZE) * TILE_SIZE + TILE_SIZE;
      distToNextCell.y	= TILE_SIZE;
      (*intersection).y = (gGame.tan[arc] * ((*positionMap).y - position3d.x)) + position3d.y;
    }
  else 	// RAY FACING LEFT
    {
      (*positionMap).y	= (position3d.x / TILE_SIZE) * TILE_SIZE;
      distToNextCell.y	= -TILE_SIZE;
      (*intersection).y = (gGame.tan[arc] * ((*positionMap).y - position3d.x)) + position3d.y;
      (*positionMap).y--;
    }

  while (1)
    {
      cell.x = ((*positionMap).y / TILE_SIZE);
      cell.y = (int) ((*intersection).y / TILE_SIZE);
      
      if ((cell.x >= size.width+2) || (cell.y >= size.height+2) || cell.x < 0 || cell.y < 0) {
	(*distToWallHit).y = DBL_MAX;	// MAX_VALUE;
	break;
      }
      
      (*collide).y = rayCollideVertical(distToNextCell.y, cell.x, cell.y);
      
      if ((*collide).y != COLLIDE_NO) {
	(*distToWallHit).y = ((*intersection).y - position3d.y)	* gGame.iSin[arc];
	break;
      }
      
      (*intersection).y += gGame.stepY[arc];	//distToNextIntersection vert
      (*positionMap).y += distToNextCell.y;
    }
}

void rayCast(int is3dView)
{
  d3View	*d3view		= &gGame.d3View;
  player	*player		= &gGame.player;
  pos		positionMap;	// Horizontal or vertical coordinate of intersection
				// Theoretically, this will be multiple of TILE_SIZE, but some trick did here might cause the values off by 1   x = horizontal , y = vertical

  dpos		intersection;	// x and y intersections
  dpos		distToWallHit;	// the distance of the x and y ray intersections from the viewpoint.  x = horizontal , y = vertical
  int		arc;
  int		ray;
  pos		collide;
  
  // field of view is 60 degree with the point of view (player's direction in the middle)
  // 30   30
  //    ^
  //  \ | /
  //   \|/
  //    v
  // we will trace the rays starting from the leftmost ray

  arc = player->arc - ANGLE30;
  if (arc < 0)
    arc += ANGLE360;

  for (ray = 0; ray < d3view->size.width; ray += d3view->rayWidth, arc += d3view->rayWidth)
    {
      if (arc >= ANGLE360)
	arc -= ANGLE360;

      DDAHoriz(arc, &positionMap, &distToWallHit, &intersection, &collide);
      DDAVert(arc, &positionMap, &distToWallHit, &intersection, &collide);

      // DRAW THE WALL SLICE
      if (distToWallHit.x < distToWallHit.y)
	{
	  drawMapviewRay(intersection.x, positionMap.x);
	  if (is3dView)
	    draw3dviewSlice(arc, ray, distToWallHit.x, collide.x == COLLIDE_EXIT ? 0 : 192);
	}
      else
	{
	  drawMapviewRay(positionMap.y, intersection.y);
	  if (is3dView)
	    draw3dviewSlice(arc, ray, distToWallHit.y, collide.y == COLLIDE_EXIT ? 0 : 160);
	}
    }

  drawWalls();
  drawExit();
}

int process3DView()
{
  SDL_Rect	r;
  int		ret		= 0;
  SDL_Surface	*surface	= gGame.surfaces.d3View;
  d3View	*d3view		= &gGame.d3View;

  if (gGame.separator_x == SCREEN_WIDTH-1)
    rayCast(0);
  else {
    ret = createRGBSurface(&gGame.surfaces.d3View, d3view->size.width, d3view->size.height);

    if (ret != 0)
      return (ret);

    surface	= gGame.surfaces.d3View;
    r.x		= 0;
    r.y		= 0;
    r.w		= d3view->size.width;
    r.h		= d3view->size.height;

    fillRect(surface, &r, 0x0, 0x0, 0x0);    
    drawBackground();
    rayCast(1);
    drawBox(surface, 0, 0, d3view->size.width-1, d3view->size.height-1, 0xffffff);
  }
  return (ret);
}


int processInputs()
{
  player	*player		= &gGame.player;
  framerate	*framerate 	= &gGame.framerate;
  int		ret;
  SDL_Color	foreColor;
  char		fpsString[64];
  dpos		playerDir;
  double	ratioFPS	= (double)60 / (double)framerate->rate;
  pos		newPos;

  playerDir.x	= gGame.cos[player->arc];
  playerDir.y = gGame.sin[player->arc];

  if (gGame.moveSeparator != 0)
    {
      if (gGame.separator_x == 0)			gGame.separator_x += 64 + (gGame.moveSeparator*ratioFPS);
      else if (gGame.separator_x == SCREEN_WIDTH-1)	gGame.separator_x += -64 + (gGame.moveSeparator*ratioFPS);
      else						gGame.separator_x += (gGame.moveSeparator*ratioFPS);
      if (gGame.separator_x < 64)			gGame.separator_x = 0;
      if (gGame.separator_x > SCREEN_WIDTH-1-64)	gGame.separator_x = SCREEN_WIDTH-1;
      ret = resizeViews();
      if (ret != 0)
	consoleErr("Unable to resizeMaze()\n");
    }

  if (gGame.moveDensity != 0)
    {
      gGame.maze.density += gGame.moveDensity;
      if (gGame.maze.density < 0)  gGame.maze.density = 0;
      if (gGame.maze.density > 25) gGame.maze.density = 25;
      gGame.moveDensity = 0;
      fillMaze();
    }

  if (gGame.moveMapWidth != 0)
    {
      gGame.maze.size.width += gGame.moveMapWidth;
      if (gGame.maze.size.width < 3) gGame.maze.size.width = 3;
      gGame.moveMapWidth = 0;
      ret = resizeMaze();
      if (ret != 0)
	consoleErr("Unable to resizeMaze()\n");
    }

  if (gGame.moveMapHeight != 0)
    {
      gGame.maze.size.height += gGame.moveMapHeight;
      if (gGame.maze.size.height < 3) gGame.maze.size.height = 3;
      gGame.moveMapHeight = 0;
      ret = resizeMaze();
      if (ret != 0)
	consoleErr("Unable to resizeMaze()\n");
    }

  if (gGame.rotatePlayer < 0)
    if ((player->arc += (gGame.rotatePlayer * ratioFPS)) < ANGLE0)
      player->arc += ANGLE360;

  if (gGame.rotatePlayer > 0)
    if ((player->arc += (gGame.rotatePlayer * ratioFPS)) > ANGLE360)
      player->arc -= ANGLE360;

  if (gGame.movePlayer == 1)
    {
      newPos.x = player->position3d.x + ((int) (playerDir.x *3 * player->speed * ratioFPS));
      newPos.y = player->position3d.y + ((int) (playerDir.y *3 * player->speed * ratioFPS));
      if ((cell(newPos.x/TILE_SIZE, newPos.y/TILE_SIZE) & WALLS_MSK) == 0) {
	player->position3d.x += ((int) (playerDir.x * player->speed * ratioFPS));
	player->position3d.y += ((int) (playerDir.y * player->speed * ratioFPS));
	setPlayerPositionMap();
      }
    }

  if (gGame.movePlayer == -1)
    {
      newPos.x = player->position3d.x - ((int) (playerDir.x *3 * player->speed * ratioFPS));
      newPos.y = player->position3d.y - ((int) (playerDir.y *3 * player->speed * ratioFPS));
      if ((cell(newPos.x/TILE_SIZE, newPos.y/TILE_SIZE) & WALLS_MSK) == 0) {
	player->position3d.x -= ((int) (playerDir.x * player->speed * ratioFPS));
	player->position3d.y -= ((int) (playerDir.y * player->speed * ratioFPS));
	setPlayerPositionMap();
      }
    }

  if (gGame.changeMode == 1)
    {
      gGame.mode = (gGame.mode==MODE_FACE) ? MODE_CUBE : MODE_FACE;
      gGame.changeMode = 0;
      if (resetGame() != 0)
	ret = -1;
  }
  
  foreColor.r = 255;
  foreColor.g = 255;
  foreColor.g = 255;

  sprintf(fpsString, "Walls density: %d%%", gGame.maze.density );
  if (gGame.surfaces.density != NULL)
    SDL_FreeSurface(gGame.surfaces.density);
  gGame.surfaces.density = TTF_RenderText_Solid(gGame.font, fpsString, foreColor);  
  ret = gGame.surfaces.density == NULL ? -1 : 0;
  if (ret != 0)
    consoleErr("TTF_RenderText density failed\n");
  
  sprintf(fpsString, "Map size: %d x %d cells", gGame.maze.size.width, gGame.maze.size.height );
  if (gGame.surfaces.mapSize != NULL)
    SDL_FreeSurface(gGame.surfaces.mapSize);
  gGame.surfaces.mapSize = TTF_RenderText_Solid(gGame.font, fpsString, foreColor);
  ret = gGame.surfaces.mapSize == NULL ? -1 : 0;
  if (ret != 0)
    consoleErr("TTF_RenderText mapSize failed\n");

  return (ret);
}


int processFramerate()
{
  int		ret;
  SDL_Color	foreColor;
  char		fpsString[32];
  framerate	*framerate = &gGame.framerate;

  foreColor.r = 255;
  foreColor.g = 255;
  foreColor.g = 255;
  sprintf(fpsString, "%d fps", 1000 / (framerate->time_passed == 0 ? 1 : framerate->time_passed));

  if (gGame.surfaces.framerate != NULL)
    SDL_FreeSurface(gGame.surfaces.framerate);
  gGame.surfaces.framerate = TTF_RenderText_Solid(gGame.font, fpsString, foreColor);
  ret = gGame.surfaces.framerate == NULL ? -1 : 0;
  if (ret != 0)
    consoleErr("TTF_RenderText framerate failed\n");
  return (ret);
}

int pollEvent()
{
  SDL_Event	event;
  SDLKey	sym;

  while (SDL_PollEvent(&event) != 0)
    {
      sym = event.key.keysym.sym;
      switch (event.type) {
      case SDL_QUIT :
	return (1);
      case SDL_KEYUP :
	if ( sym == SDLK_LEFT )				gGame.moveSeparator = 0;
	if ( sym == SDLK_RIGHT )			gGame.moveSeparator = 0;
	if ( sym == SDLK_a && gGame.rotatePlayer < 0)	gGame.rotatePlayer = 0;
	if ( sym == SDLK_d && gGame.rotatePlayer > 0)	gGame.rotatePlayer = 0;
	if ( sym == SDLK_w && gGame.movePlayer > 0)	gGame.movePlayer = 0;
	if ( sym == SDLK_s && gGame.movePlayer < 0)	gGame.movePlayer = 0;
	if ( sym == SDLK_m )				gGame.showMatrix = 0;
	break;
      case SDL_KEYDOWN :
	if ( sym == SDLK_ESCAPE )	return (1);
	if ( sym == SDLK_LEFT )		gGame.moveSeparator = -3;
	if ( sym == SDLK_RIGHT )	gGame.moveSeparator = 3;
	if ( sym == SDLK_DOWN )		gGame.moveDensity = -1;
	if ( sym == SDLK_UP )		gGame.moveDensity = 1;
	if ( sym == SDLK_KP4 )		gGame.moveMapWidth = -1;
	if ( sym == SDLK_KP6 )		gGame.moveMapWidth = 1;
	if ( sym == SDLK_KP2 )		gGame.moveMapHeight = -1;
	if ( sym == SDLK_KP8 )		gGame.moveMapHeight = 1;

	if ( sym == SDLK_a )	gGame.rotatePlayer = -ANGLE2; // left
	if ( sym == SDLK_d )	gGame.rotatePlayer = ANGLE2; // right
	if ( sym == SDLK_w )	gGame.movePlayer = 1;
	if ( sym == SDLK_s )	gGame.movePlayer = -1;

	if ( sym == SDLK_m )	gGame.showMatrix = 1;
	if ( sym == SDLK_t )	gGame.changeMode = 1;
	break;
      }
    }
  return (0);
}

int process()
{
  int	ret;

  ret = processInputs();
  if (ret != 0)
    return (ret);

  ret = processMap();
  if (ret != 0)
    return (ret);

  ret = process3DView();
  if (ret != 0)
    return (ret);

  ret = processFramerate();
  if (ret != 0)
    return (ret);

  return (ret);
}

int render()
{
  int		ret;
  mapView	*mapview	= &gGame.mapView;
  d3View	*d3view		= &gGame.d3View;
  surfaces	*surfaces	= &gGame.surfaces;

  // clear screen
  fillRect(surfaces->screen, NULL, 0,0,0 );

  // render map
  if (gGame.separator_x > 0)
    if ((ret = apply_surface(mapview->offset.x - mapview->cell.width, mapview->offset.y - mapview->cell.height, surfaces->mapView, NULL)) < 0)
      consoleErr("render(): Apply map failed");

  // render view
  if (gGame.separator_x < SCREEN_WIDTH-1)
    if ((ret = apply_surface(d3view->offset.x, d3view->offset.y, surfaces->d3View, NULL)) < 0)
      consoleErr("render(): Apply view failed");

  // render inputs
  if ((ret = apply_surface(16, mapview->marginTop + mapview->size.height + mapview->marginBottom, surfaces->mapSize, NULL)) < 0)
    consoleErr("render(): Apply mapSize failed");
  if ((ret = apply_surface(16, mapview->marginTop + mapview->size.height + mapview->marginBottom + surfaces->mapSize->h, surfaces->density, NULL)) < 0)
    consoleErr("render(): Apply density failed");

  // render framerate
  if ((ret = apply_surface(SCREEN_WIDTH - surfaces->framerate->w, SCREEN_HEIGHT - surfaces->framerate->h, surfaces->framerate, NULL)) < 0)
    consoleErr("render(): Apply framerate failed");

  return (ret);
}


int framerateDelay()
{
  Uint32	current_ticks;
  Uint32	target_ticks;
  Uint32	delay;
  framerate	*framerate	= &gGame.framerate;

  framerate->framecount++;
  current_ticks			= SDL_GetTicks();
  framerate->time_passed	= current_ticks - framerate->lastticks;
  framerate->lastticks		= current_ticks;
  target_ticks			= framerate->baseticks + (Uint32) ((float) framerate->framecount * framerate->rateticks);

  if (current_ticks <= target_ticks)
    {
      delay = target_ticks - current_ticks;
      if (delay > 1000) {
	consoleErr("delay > 1000\n");
	return -1;
      }
      SDL_Delay(delay);
    }
  else
    {
      framerate->framecount = 0;
      framerate->baseticks = SDL_GetTicks();
    }
  return (0);
}

int clearAll() {
  deleteMaze();
  deleteTables();

  if (gGame.surfaces.screen != NULL)	SDL_FreeSurface(gGame.surfaces.screen);
  if (gGame.surfaces.framerate != NULL) SDL_FreeSurface(gGame.surfaces.framerate);
  if (gGame.surfaces.mapView != NULL)	SDL_FreeSurface(gGame.surfaces.mapView);
  if (gGame.surfaces.d3View != NULL)	SDL_FreeSurface(gGame.surfaces.d3View);
  if (gGame.surfaces.density != NULL)	SDL_FreeSurface(gGame.surfaces.density);
  if (gGame.surfaces.mapSize != NULL)	SDL_FreeSurface(gGame.surfaces.mapSize);

  TTF_CloseFont(gGame.font);
  TTF_Quit();

  return (0);
}


/*****************************************************************************
 * FUNCTION MAIN
 ****************************************************************************/

int main(int argc, char *argv[])
{
  int ret = 0;

  if ((ret = initSDL()) == 0 ) {
    if ((ret = initVideo()) == 0) {
      if ((ret = initGame()) == 0 ) {	      
	while (pollEvent() == 0) {
	  if ((ret = process()) < 0 ) break;
	  if ((ret = render()) < 0 ) break;
	  if ((ret = flip()) < 0 ) break;
	  if ((ret = framerateDelay()) < 0 ) break;
	}
      }
    }
  }
  clearAll();
  SDL_Quit();
  return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

// EOF

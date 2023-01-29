// 3D Filled Vector Graphics
// (c) 2019 Pawel A. Hernik

/*
 Implemented features:
 - optimized rendering without local framebuffer, in STM32 case 1 to 32 lines buffer can be used
 - pattern based background
 - 3D starfield
 - no floating point arithmetic
 - no slow trigonometric functions
 - rotations around X and Y axes
 - simple outside screen culling
 - rasterizer working for all convex polygons
 - backface culling
 - visible faces sorting by Z axis
*/
#include "math.h"
#include "stdlib.h"
#define RGBto565(r,g,b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define pgm_read_byte *
#include "stdint.h"
#define BLACK 0

int random(int min, int max) {
    return rand() % (max - min + 1) + min;
}


#define swap(a, b) { int t = a; a = b; b = t; }
// 一次渲染整个屏幕
#define NLINES SCREEN_HEIGHT
uint16_t frBuf[SCR_WD*NLINES];
int yFr=0;

// ------------------------------------------------
#define MAXSIN 255
const uint8_t sinTab[91] PROGMEM = {
0,4,8,13,17,22,26,31,35,39,44,48,53,57,61,65,70,74,78,83,87,91,95,99,103,107,111,115,119,123,
127,131,135,138,142,146,149,153,156,160,163,167,170,173,177,180,183,186,189,192,195,198,200,203,206,208,211,213,216,218,
220,223,225,227,229,231,232,234,236,238,239,241,242,243,245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
255
};

int fastSin(int i)
{
  while(i<0) i+=360;
  while(i>=360) i-=360;
  if(i<90)  return(pgm_read_byte(&sinTab[i])); else
  if(i<180) return(pgm_read_byte(&sinTab[180-i])); else
  if(i<270) return(-pgm_read_byte(&sinTab[i-180])); else
            return(-pgm_read_byte(&sinTab[360-i]));
}

int fastCos(int i)
{
  return fastSin(i+90);
}

// ------------------------------------------------

#define COL11 RGBto565(0,250,250)
#define COL12 RGBto565(0,180,180)
#define COL13 RGBto565(0,210,210)

#define COL21 RGBto565(250,0,250)
#define COL22 RGBto565(180,0,180)
#define COL23 RGBto565(210,0,210)

#define COL31 RGBto565(250,250,0)
#define COL32 RGBto565(180,180,0)
#define COL33 RGBto565(210,210,0)

#define COL41 RGBto565(250,150,0)
#define COL42 RGBto565(180,100,0)
#define COL43 RGBto565(210,140,0)

#define COL51 RGBto565(0,250,0)
#define COL52 RGBto565(0,180,0)
#define COL53 RGBto565(0,210,0)

// ------------------------------------------------ 
/*
 *        ^Y 
 *        |     / Z
 *        |    /
 *    7------6
 *   /|     /|
 *  3------2 | 
 *  | |    | |    ----> X
 *  | 4----|-5
 *  |/     |/
 *  0------1
 *
 */

const int16_t numVerts1 = 8;
const int16_t verts1[] = {
  -100, -100, -100,  // 0
   100, -100, -100,  // 1
   100,  100, -100,  // 2
  -100,  100, -100,  // 3
  -100, -100,  100,  // 4
   100, -100,  100,  // 5
   100,  100,  100,  // 6
  -100,  100,  100,  // 7
};

const int16_t numQuads1 = 6;
const int16_t quads1[] = {
  0, 1, 2, 3,  // front
  7, 6, 5, 4,  // back
  4, 5, 1, 0,  // bottom
  3, 2, 6, 7,  // top
  7, 4, 0, 3,  // left
  2, 1, 5, 6,  // right
};

const uint16_t quadColor1[] = {
        RGBto565(255,0,0),RGBto565(255,255,0),RGBto565(255,255,255),RGBto565(0,255,255),RGBto565(0,0,255),RGBto565(255,0,255),
  //RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN
};

// ----------------------------------------------- 
/*
            ^Y
            |
            |
         8----9
         |    |
    4----3----2----6
    |    |    |    |   --->X
    5----0----1----7
         |    |
         10--11
*/

const int16_t numVerts2 = 12*2;
const int16_t verts2[] = {
  -50, -50, -50,  // 0
   50, -50, -50,  // 1
   50,  50, -50,  // 2
  -50,  50, -50,  // 3
  -150,   50, -50,  // 4
  -150,  -50, -50,  // 5
   150,   50, -50,  // 6
   150,  -50, -50,  // 7
   -50,  150, -50,  // 8
    50,  150, -50,  // 9
   -50, -150, -50,  // 10
    50, -150, -50,  // 11

  -50, -50, 50,  // 0
   50, -50, 50,  // 1
   50,  50, 50,  // 2
  -50,  50, 50,  // 3
  -150,   50, 50,  // 4
  -150,  -50, 50,  // 5
   150,   50, 50,  // 6
   150,  -50, 50,  // 7
   -50,  150, 50,  // 8
    50,  150, 50,  // 9
   -50, -150, 50,  // 10
    50, -150, 50,  // 11

};

const int16_t numQuads2 = 5+5+3*4;
const int16_t quads2[] = {
  0, 1, 2, 3,  // front mid
  7, 6, 2, 1,  // front right
  4, 5, 0, 3,  // front left
  3, 2, 9, 8,  // front top
  10,11,1, 0,  // front bottom

  12+3, 12+2, 12+1, 12+0,  // rear mid
  12+1, 12+2, 12+6, 12+7,  // rear right
  12+3, 12+0, 12+5, 12+4,  // rear left
  12+8, 12+9, 12+2, 12+3,  // rear top
  12+0, 12+1,12+11,12+10,  // rear bottom

  0+12,10+12,10,0,
  10+12,11+12,11,10,
  11+12,1+12,1,11,

  1+12,7+12,7,1,
  7+12,6+12,6,7,
  6+12,2+12,2,6,

  2+12,9+12,9,2,
  9+12,8+12,8,9,
  8+12,3+12,3,8,

  3+12,4+12,4,3,
  4+12,5+12,5,4,
  5+12,0+12,0,5,

};

const uint16_t quadColor2[] = {
  COL31,COL51,COL51,COL51,COL51,
  COL31,COL51,COL51,COL51,COL51,
  COL52,COL53,COL52,COL53,COL52,COL53,
  COL52,COL53,COL52,COL53,COL52,COL53,
};

// ----------------------------------------------- 

const int16_t numVerts3 = 8*4;
const int16_t verts3[] = {
  -100, -100, -200,  // 0 front
   100, -100, -200,  // 1
   100,  100, -200,  // 2
  -100,  100, -200,  // 3
   -50,  -50,  -50,  // 4
    50,  -50,  -50,  // 5
    50,   50,  -50,  // 6
   -50,   50,  -50,  // 7

   -50,  -50,   50,  // 0 rear
    50,  -50,   50,  // 1
    50,   50,   50,  // 2
   -50,   50,   50,  // 3
  -100, -100,  200,  // 4
   100, -100,  200,  // 5
   100,  100,  200,  // 6
  -100,  100,  200,  // 7

  -200, -100, -100,  // 0 left
   -50,  -50, -50,   // 1
   -50,   50, -50,   // 2
  -200,  100, -100,  // 3
  -200, -100,  100,  // 4
   -50,  -50,  50,   // 5
   -50,   50,  50,   // 6
  -200,  100,  100,  // 7

    50,  -50,  -50,  // 0 right
   200, -100, -100,  // 1
   200,  100, -100,  // 2
    50,   50,  -50,  // 3
    50,  -50,   50,  // 4
   200, -100,  100,  // 5
   200,  100,  100,  // 6
    50,   50,   50,  // 7
};

const int16_t numQuads3 = 6*4;
const int16_t quads3[] = {
  0, 1, 2, 3,  // front
  7, 6, 5, 4,  // back
  4, 5, 1, 0,  // bottom
  3, 2, 6, 7,  // top
  7, 4, 0, 3,  // left
  2, 1, 5, 6,  // right

  8+0, 8+1, 8+2, 8+3,  // front
  8+7, 8+6, 8+5, 8+4,  // back
  8+4, 8+5, 8+1, 8+0,  // bottom
  8+3, 8+2, 8+6, 8+7,  // top
  8+7, 8+4, 8+0, 8+3,  // left
  8+2, 8+1, 8+5, 8+6,  // right

  16+0, 16+1, 16+2, 16+3,  // front
  16+7, 16+6, 16+5, 16+4,  // back
  16+4, 16+5, 16+1, 16+0,  // bottom
  16+3, 16+2, 16+6, 16+7,  // top
  16+7, 16+4, 16+0, 16+3,  // left
  16+2, 16+1, 16+5, 16+6,  // right

  24+0, 24+1, 24+2, 24+3,  // front
  24+7, 24+6, 24+5, 24+4,  // back
  24+4, 24+5, 24+1, 24+0,  // bottom
  24+3, 24+2, 24+6, 24+7,  // top
  24+7, 24+4, 24+0, 24+3,  // left
  24+2, 24+1, 24+5, 24+6,  // right
};

const uint16_t quadColor3[] = {
  COL13,COL13,COL11,COL11,COL12,COL12,
  COL23,COL23,COL21,COL21,COL22,COL22,
  COL33,COL33,COL31,COL31,COL32,COL32,
  COL43,COL43,COL41,COL41,COL42,COL42,
};

// ----------------------------------------------- 

const int16_t numVerts4 = 8*9;
const int16_t verts4[] = {

  -50, -50, -50,  // 0
   50, -50, -50,  // 1
   50,  50, -50,  // 2
  -50,  50, -50,  // 3
  -50, -50,  50,  // 4
   50, -50,  50,  // 5
   50,  50,  50,  // 6
  -50,  50,  50,  // 7

  100-50, 100-50, -50+100,  // 0
  100+50, 100-50, -50+100,  // 1
  100+50, 100+50, -50+100,  // 2
  100-50, 100+50, -50+100,  // 3
  100-50, 100-50,  50+100,  // 4
  100+50, 100-50,  50+100,  // 5
  100+50, 100+50,  50+100,  // 6
  100-50, 100+50,  50+100,  // 7

  -100-50, 100-50, -50+100,  // 0
  -100+50, 100-50, -50+100,  // 1
  -100+50, 100+50, -50+100,  // 2
  -100-50, 100+50, -50+100,  // 3
  -100-50, 100-50,  50+100,  // 4
  -100+50, 100-50,  50+100,  // 5
  -100+50, 100+50,  50+100,  // 6
  -100-50, 100+50,  50+100,  // 7

  -100-50, -100-50, -50+100,  // 0
  -100+50, -100-50, -50+100,  // 1
  -100+50, -100+50, -50+100,  // 2
  -100-50, -100+50, -50+100,  // 3
  -100-50, -100-50,  50+100,  // 4
  -100+50, -100-50,  50+100,  // 5
  -100+50, -100+50,  50+100,  // 6
  -100-50, -100+50,  50+100,  // 7

  100-50, -100-50, -50+100,  // 0
  100+50, -100-50, -50+100,  // 1
  100+50, -100+50, -50+100,  // 2
  100-50, -100+50, -50+100,  // 3
  100-50, -100-50,  50+100,  // 4
  100+50, -100-50,  50+100,  // 5
  100+50, -100+50,  50+100,  // 6
  100-50, -100+50,  50+100,  // 7

  100-50, 100-50, -50-100,  // 0
  100+50, 100-50, -50-100,  // 1
  100+50, 100+50, -50-100,  // 2
  100-50, 100+50, -50-100,  // 3
  100-50, 100-50,  50-100,  // 4
  100+50, 100-50,  50-100,  // 5
  100+50, 100+50,  50-100,  // 6
  100-50, 100+50,  50-100,  // 7

  -100-50, 100-50, -50-100,  // 0
  -100+50, 100-50, -50-100,  // 1
  -100+50, 100+50, -50-100,  // 2
  -100-50, 100+50, -50-100,  // 3
  -100-50, 100-50,  50-100,  // 4
  -100+50, 100-50,  50-100,  // 5
  -100+50, 100+50,  50-100,  // 6
  -100-50, 100+50,  50-100,  // 7

  -100-50, -100-50, -50-100,  // 0
  -100+50, -100-50, -50-100,  // 1
  -100+50, -100+50, -50-100,  // 2
  -100-50, -100+50, -50-100,  // 3
  -100-50, -100-50,  50-100,  // 4
  -100+50, -100-50,  50-100,  // 5
  -100+50, -100+50,  50-100,  // 6
  -100-50, -100+50,  50-100,  // 7

  100-50, -100-50, -50-100,  // 0
  100+50, -100-50, -50-100,  // 1
  100+50, -100+50, -50-100,  // 2
  100-50, -100+50, -50-100,  // 3
  100-50, -100-50,  50-100,  // 4
  100+50, -100-50,  50-100,  // 5
  100+50, -100+50,  50-100,  // 6
  100-50, -100+50,  50-100,  // 7
};

const int16_t numQuads4 = 6*9;
const int16_t quads4[] = {
  0, 1, 2, 3,  // front
  7, 6, 5, 4,  // back
  4, 5, 1, 0,  // bottom
  3, 2, 6, 7,  // top
  7, 4, 0, 3,  // left
  2, 1, 5, 6,  // right

  8+0, 8+1, 8+2, 8+3,  // front
  8+7, 8+6, 8+5, 8+4,  // back
  8+4, 8+5, 8+1, 8+0,  // bottom
  8+3, 8+2, 8+6, 8+7,  // top
  8+7, 8+4, 8+0, 8+3,  // left
  8+2, 8+1, 8+5, 8+6,  // right

  16+0, 16+1, 16+2, 16+3,  // front
  16+7, 16+6, 16+5, 16+4,  // back
  16+4, 16+5, 16+1, 16+0,  // bottom
  16+3, 16+2, 16+6, 16+7,  // top
  16+7, 16+4, 16+0, 16+3,  // left
  16+2, 16+1, 16+5, 16+6,  // right

  24+0, 24+1, 24+2, 24+3,  // front
  24+7, 24+6, 24+5, 24+4,  // back
  24+4, 24+5, 24+1, 24+0,  // bottom
  24+3, 24+2, 24+6, 24+7,  // top
  24+7, 24+4, 24+0, 24+3,  // left
  24+2, 24+1, 24+5, 24+6,  // right

  32+0, 32+1, 32+2, 32+3,  // front
  32+7, 32+6, 32+5, 32+4,  // back
  32+4, 32+5, 32+1, 32+0,  // bottom
  32+3, 32+2, 32+6, 32+7,  // top
  32+7, 32+4, 32+0, 32+3,  // left
  32+2, 32+1, 32+5, 32+6,  // right

  40+0, 40+1, 40+2, 40+3,  // front
  40+7, 40+6, 40+5, 40+4,  // back
  40+4, 40+5, 40+1, 40+0,  // bottom
  40+3, 40+2, 40+6, 40+7,  // top
  40+7, 40+4, 40+0, 40+3,  // left
  40+2, 40+1, 40+5, 40+6,  // right

  48+0, 48+1, 48+2, 48+3,  // front
  48+7, 48+6, 48+5, 48+4,  // back
  48+4, 48+5, 48+1, 48+0,  // bottom
  48+3, 48+2, 48+6, 48+7,  // top
  48+7, 48+4, 48+0, 48+3,  // left
  48+2, 48+1, 48+5, 48+6,  // right

  56+0, 56+1, 56+2, 56+3,  // front
  56+7, 56+6, 56+5, 56+4,  // back
  56+4, 56+5, 56+1, 56+0,  // bottom
  56+3, 56+2, 56+6, 56+7,  // top
  56+7, 56+4, 56+0, 56+3,  // left
  56+2, 56+1, 56+5, 56+6,  // right

  64+0, 64+1, 64+2, 64+3,  // front
  64+7, 64+6, 64+5, 64+4,  // back
  64+4, 64+5, 64+1, 64+0,  // bottom
  64+3, 64+2, 64+6, 64+7,  // top
  64+7, 64+4, 64+0, 64+3,  // left
  64+2, 64+1, 64+5, 64+6,  // right
};

const uint16_t quadColor4[] = {
  COL13,COL13,COL11,COL11,COL12,COL12,
  COL23,COL23,COL21,COL21,COL22,COL22,
  COL33,COL33,COL31,COL31,COL32,COL32,
  COL43,COL43,COL41,COL41,COL42,COL42,
  COL53,COL53,COL51,COL51,COL52,COL52,
  COL33,COL33,COL31,COL31,COL32,COL32,
  COL43,COL43,COL41,COL41,COL42,COL42,
  COL53,COL53,COL51,COL51,COL52,COL52,
  COL23,COL23,COL21,COL21,COL22,COL22,
};

// ----------------------------------------------- 
int16_t numVerts = numVerts1;
int16_t *verts   = (int16_t*)verts1;
int16_t numQuads = numQuads1;
int16_t *quads   = (int16_t*)quads1;
uint16_t *quadColor = (uint16_t*)quadColor1;

#define MAXQUADS 6*20
#define MAXVERTS 8*20

int transVerts[MAXVERTS*3];
int projVerts[MAXVERTS*2];
int sortedQuads[MAXQUADS];
int rot0 = 0, rot1 = 0;
int numVisible = 0;

// simple Amiga like blitter implementation
void rasterize(int x0, int y0, int x1, int y1, int *line) 
{
  if((y0<yFr && y1<yFr) || (y0>=yFr+NLINES && y1>=yFr+NLINES)) return; // exit if line outside rasterized area
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int err2,err = dx-dy;
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  
  while(1)  {
    if(y0>=yFr && y0<yFr+NLINES) {
      if(x0<line[2*(y0-yFr)+0]) line[2*(y0-yFr)+0] = x0>0 ? x0 : 0;
      if(x0>line[2*(y0-yFr)+1]) line[2*(y0-yFr)+1] = x0<WD_3D ? x0 : WD_3D-1;
    }

    if(x0==x1 && y0==y1)  return;
    err2 = err+err;
    if(err2 > -dy) { err -= dy; x0 += sx; }
    if(err2 < dx)  { err += dx; y0 += sy; }
  }
}

void drawQuad( int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint16_t c) 
{
  int x,y;
  int line[NLINES*2];
  for(y=0;y<NLINES;y++) { line[2*y+0] = WD_3D+1; line[2*y+1] = -1; }

  rasterize( x0, y0, x1, y1, line );
  rasterize( x1, y1, x2, y2, line );
  rasterize( x2, y2, x3, y3, line );
  rasterize( x3, y3, x0, y0, line );

  for(y=0;y<NLINES;y++)
    if(line[2*y+1]>line[2*y+0]) for(x=line[2*y+0]; x<=line[2*y+1]; x++) frBuf[SCR_WD*y+x]=c;
    //if(line[2*y+1]>line[2*y]) for(x=line[2*y]; x<=line[2*y+1]; x++) if(c==COL13) frBuf[SCR_WD*y+x]=pat7[((y+yFr)&0x1f)*32 + ((x-line[2*y])&0x1f)]; else frBuf[SCR_WD*y+x]=c;
}

void cullQuads(int *v) 
{
  // backface culling
  numVisible=0;
  int x1,y1,x2,y2,z;
  for(int i=0;i<numQuads;i++) {
    if(bfCull) {
      int x1 = v[3*quads[4*i+0]+0]-v[3*quads[4*i+1]+0];
      int y1 = v[3*quads[4*i+0]+1]-v[3*quads[4*i+1]+1];
      int x2 = v[3*quads[4*i+2]+0]-v[3*quads[4*i+1]+0];
      int y2 = v[3*quads[4*i+2]+1]-v[3*quads[4*i+1]+1];
      int z = x1*y2-y1*x2;
      if(z<0) sortedQuads[numVisible++] = i;
    } else sortedQuads[numVisible++] = i;
    //char txt[30];
    //snprintf(txt,30,"%d z=%6d  dr=%2d r0=%d",i,z,sortedQuads[i],rot[0]);
    //Serial.println(txt);
  }
  
  int i,j,zPoly[numVisible];
  // average Z of the polygon
  for(i=0;i<numVisible;++i) {
    zPoly[i] = 0.0;
    for(j=0;j<4;++j) zPoly[i] += v[3*quads[4*sortedQuads[i]+j]+2];
  }

  // sort by Z
  for(i=0;i<numVisible-1;++i) {
    for(j=i;j<numVisible;++j) {
      if(zPoly[i]<zPoly[j]) {
        swap(zPoly[j],zPoly[i]);
        swap(sortedQuads[j],sortedQuads[i]);
      }
    }
  }
}

void drawQuads(int *v2d) 
{
  for(int i=0;i<numVisible;i++) {
    int q = sortedQuads[i];
    //if(f<0) continue;
    int v0 = quads[4*q+0];
    int v1 = quads[4*q+1];
    int v2 = quads[4*q+2];
    int v3 = quads[4*q+3];
    drawQuad( v2d[2*v0+0],v2d[2*v0+1],  v2d[2*v1+0],v2d[2*v1+1], v2d[2*v2+0],v2d[2*v2+1], v2d[2*v3+0],v2d[2*v3+1], quadColor[q]);
  }
} 

// --------------------------------------------------

int t=0;

void render3D(void)
{
  int cos0,sin0,cos1,sin1;
  int i,x0,y0,z0,fac,distToObj;
  int camZ = 200;
  int scaleFactor = HT_3D/4;
  int near = 300;

  if(t++>360) t-=360;
  distToObj = 150 + 300*fastSin(3*t)/MAXSIN;
  cos0 = fastCos(rot0);
  sin0 = fastSin(rot0);
  cos1 = fastCos(rot1);
  sin1 = fastSin(rot1);

  for(i=0;i<numVerts;i++) {
    x0 = verts[3*i+0];
    y0 = verts[3*i+1];
    z0 = verts[3*i+2];
    transVerts[3*i+0] = (cos0*x0 + sin0*z0)/MAXSIN;
    transVerts[3*i+1] = (cos1*y0 + (cos0*sin1*z0-sin0*sin1*x0)/MAXSIN)/MAXSIN;
    transVerts[3*i+2] = camZ + ((cos0*cos1*z0-sin0*cos1*x0)/MAXSIN - sin1*y0)/MAXSIN;

    fac = 60;
//    fac = scaleFactor * near / (transVerts[3*i+2]+near+distToObj);

    projVerts[2*i+0] = (100*WD_3D/2 + fac*transVerts[3*i+0] + 100/2)/100;
    projVerts[2*i+1] = (100*HT_3D/2 + fac*transVerts[3*i+1] + 100/2)/100;
  }

  cullQuads(transVerts);

  for(i=0;i<HT_3D;i+=NLINES) {
    yFr = i;
    // 填充背景成黑色
    memset(frBuf,0,sizeof frBuf);
    drawQuads(projVerts);
      void drawImage(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *img16);
      drawImage(0,yFr,SCR_WD,NLINES,frBuf);
  }

  //旋转
  rot0 += 2;
  rot1 += 2;
  if(rot0>360) rot0-=360;
  if(rot1>360) rot1-=360;
} 


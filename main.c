#include <string.h>
#include "stdbool.h"
#include "baseSDL.h"
#include "config.h"


#define SCR_WD  SCREEN_WIDTH
#define SCR_HT  SCREEN_HEIGHT

#define WD_3D   SCREEN_WIDTH
#define HT_3D   SCREEN_HEIGHT

#define PROGMEM

int bgMode=0;
int object=0;
int bfCull=1;

#include "pat2.h"
#include "pat7.h"
#include "pat8.h"
#include "gfx3d.h"

static uint16_t workFrame[SCREEN_WIDTH * SCREEN_HEIGHT];


void setup() {
    initStars();
}


unsigned int ms,msMin=1000,msMax=0;
char txt[30];

void loop(uint8_t *buffer) {
//    handleButton();
//    if(buttonState<0 && prevButtonState>=0 && ++bgMode>4) bgMode=0;
//    if(buttonState>0) {
//        if(++object>3) object=0;
//        msMin=1000;
//        msMax=0;
//    }
    switch(object) {
        case 0:
            numVerts  = numVerts1;
            verts     = (int16_t*)verts1;
            numQuads  = numQuads1;
            quads     = (int16_t*)quads1;
            quadColor = (uint16_t*)quadColor1;
            bfCull    = 1;
            break;
        case 1:
            numVerts  = numVerts2;
            verts     = (int16_t*)verts2;
            numQuads  = numQuads2;
            quads     = (int16_t*)quads2;
            quadColor = (uint16_t*)quadColor2;
            bfCull    = 1;
            break;
        case 2:
        default:
            numVerts  = numVerts3;
            verts     = (int16_t*)verts3;
            numQuads  = numQuads3;
            quads     = (int16_t*)quads3;
            quadColor = (uint16_t*)quadColor3;
            bfCull    = 1;
            break;
        case 3:
            numVerts  = numVerts4;
            verts     = (int16_t*)verts4;
            numQuads  = numQuads4;
            quads     = (int16_t*)quads4;
            quadColor = (uint16_t*)quadColor4;
            bfCull    = 0;
            break;
    }
//    ms=millis();
    render3D();
//    ms=millis()-ms;
//    if(ms<msMin) msMin=ms;
//    if(ms>msMax) msMax=ms;
//    snprintf(txt,30,"%d ms     %d fps ",ms,1000/ms);
//    lcd.setTextColor(YELLOW,BLACK); lcd.setCursor(0,SCR_HT-28); lcd.print(txt);
//    snprintf(txt,30,"%d-%d ms  %d-%d fps   ",msMin,msMax,1000/msMax,1000/msMin);
//    lcd.setTextColor(GREEN,BLACK); lcd.setCursor(0,SCR_HT-18); lcd.print(txt);
//    snprintf(txt,30,"total/vis %d / %d   ",numQuads,numVisible);
//    lcd.setTextColor(MAGENTA,BLACK); lcd.setCursor(0,SCR_HT-8); lcd.print(txt);
    memcpy(buffer, workFrame, SCREEN_WIDTH * SCREEN_HEIGHT * 2);


}

// draws image from RAM
void drawImage(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *img16)
{
    uint16_t * p = &workFrame[y*SCR_WD+x];

    memcpy(p,img16,w*h*sizeof(uint16_t));


}

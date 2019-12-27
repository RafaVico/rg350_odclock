////////////////////////////////////////////////
/*  OpenDingux Clock                          */
/*                                            */
/*  Created by Rafa Vico                      */
/*  December 2019                             */
/*                                            */
/*  License: GPL v.2                          */
////////////////////////////////////////////////

///////////////////////////////////
/*  Libraries                    */
///////////////////////////////////
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <string>
#include <fstream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>

#include "../inc/font_pixelberry.h"       // font are embedded in executable
#include "../inc/font_atomicclockradio.h"
#include "../inc/bmp_icons.h"

///////////////////////////////////
/*  Joystick codes               */
///////////////////////////////////

#define GCW_BUTTON_UP           SDLK_UP
#define GCW_BUTTON_DOWN         SDLK_DOWN
#define GCW_BUTTON_LEFT         SDLK_LEFT
#define GCW_BUTTON_RIGHT        SDLK_RIGHT
#define GCW_BUTTON_A            SDLK_LCTRL
#define GCW_BUTTON_B            SDLK_LALT
#define GCW_BUTTON_X            SDLK_SPACE
#define GCW_BUTTON_Y            SDLK_LSHIFT
#define GCW_BUTTON_L1           SDLK_TAB
#define GCW_BUTTON_R1           SDLK_BACKSPACE
#define GCW_BUTTON_L2           SDLK_PAGEUP
#define GCW_BUTTON_R2           SDLK_PAGEDOWN
#define GCW_BUTTON_SELECT       SDLK_ESCAPE
#define GCW_BUTTON_START        SDLK_RETURN
#define GCW_BUTTON_L3           SDLK_KP_DIVIDE
#define GCW_BUTTON_R3           SDLK_KP_PERIOD
#define GCW_BUTTON_POWER        SDLK_HOME
#define GCW_BUTTON_VOLUP        0 //SDLK_PAUSE
//#define GCW_BUTTON_VOLDOWN      0
#define GCW_JOYSTICK_DEADZONE   1000

///////////////////////////////////
/*  Other defines                */
///////////////////////////////////
#define TRUE   1
#define FALSE  0

#define BORDER_NO       0
#define BORDER_SINGLE   1
#define BORDER_ROUNDED  2

#define MODE_CLOCK  1
#define MODE_CAL    2
#define MODE_ALARM  3
#define MODE_TIMER  4

///////////////////////////////////
/*  Structs                      */
///////////////////////////////////
struct joystick_state
{
  int j1_left;
  int j1_right;
  int j1_up;
  int j1_down;
  int button_l3;
  int j2_left;
  int j2_right;
  int j2_up;
  int j2_down;
  int button_r3;
  int pad_left;
  int pad_right;
  int pad_up;
  int pad_down;
  int button_a;
  int button_b;
  int button_x;
  int button_y;
  int button_l1;
  int button_l2;
  int button_r1;
  int button_r2;
  int button_select;
  int button_start;
  int button_power;
  int button_voldown;
  int button_volup;
  int escape;
  int any;
};

struct settings
{
  int format_24;
  int date_ord1;      // 0=day, 1=month, 2=year, order of date
  int date_ord2;
  int date_ord3;
  int mon_first;      // is monday first day of the week?
};

///////////////////////////////////
/*  Globals                      */
///////////////////////////////////
SDL_Surface* screen;   		    // screen to work
int done=FALSE;
TTF_Font* font;                 // used font
TTF_Font* font2;                // used font
SDL_Joystick* joystick;         // used joystick
joystick_state mainjoystick;
Uint8* keys=SDL_GetKeyState(NULL);
tm actual_time;
tm edit_time;
int mode_app=MODE_CLOCK;
int edit_mode=FALSE;
settings clock_settings;

// graphics
SDL_Surface *img_icons[10];
//sonidos
Mix_Chunk *sound_tone;

///////////////////////////////////
/*  Messages                     */
///////////////////////////////////
const char* msg[4]=
{
  "[START] exit",
  "[SELECT] set time",
  "[A] accept",
  "[B] cancel"
};

///////////////////////////////////
/*  Function declarations        */
///////////////////////////////////
void process_events();
void process_joystick();

///////////////////////////////////
/*  Debug Functions              */
///////////////////////////////////
/*std::string getCurrentDateTime( std::string s )
{
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if(s=="now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s=="date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
}

void Logger( std::string logMsg )
{

    std::string filePath = "/usr/local/home/log_"+getCurrentDateTime("date")+".txt";
    std::string now = getCurrentDateTime("now");
    std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app );
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}*/

///////////////////////////////////
/*  Load clock settings          */
///////////////////////////////////
void load_config()
{
  FILE* config_file;
  config_file=fopen("/usr/local/home/.odclock/settings.ini","r");
  if(config_file!=NULL)
  {
    char str[50];
    char var[50];
    int val;
    while(fgets(str,50,config_file)!=NULL)
    {
      sscanf(str,"%s %d",var,&val);
      if(strcmp(var,"FormatFull")==0)
        clock_settings.format_24=val;
      if(strcmp(var,"MondayFirst")==0)
        clock_settings.mon_first=val;
      if(strcmp(var,"DateOrderA")==0)
        clock_settings.date_ord1=val;
      if(strcmp(var,"DateOrderB")==0)
        clock_settings.date_ord2=val;
      if(strcmp(var,"DateOrderC")==0)
        clock_settings.date_ord3=val;
    }
    fclose(config_file);
  }
}

///////////////////////////////////
/*  Save clock settings          */
///////////////////////////////////
void save_config()
{
  mkdir("/usr/local/home/.odclock",0);
  FILE* config_file;
  config_file=fopen("/usr/local/home/.odclock/settings.ini","wb");
  if(config_file!=NULL)
  {
    char line[20];
    sprintf(line,"FormatFull %d\n",clock_settings.format_24);
    fputs(line,config_file);
    sprintf(line,"MondayFirst %d\n",clock_settings.mon_first);
    fputs(line,config_file);
    sprintf(line,"DateOrderA %d\n",clock_settings.date_ord1);
    fputs(line,config_file);
    sprintf(line,"DateOrderB %d\n",clock_settings.date_ord2);
    fputs(line,config_file);
    sprintf(line,"DateOrderC %d\n",clock_settings.date_ord3);
    fputs(line,config_file);

    fclose(config_file);
  }
}

///////////////////////////////////
/*  Draw a pixel in surface      */
///////////////////////////////////
void putpixel(SDL_Surface *dst, int x, int y, Uint32 pixel)
{
    int byteperpixel = dst->format->BytesPerPixel;
    Uint8 *p = (Uint8*)dst->pixels + y * dst->pitch + x * byteperpixel;
    // Adress to pixel
    *(Uint32 *)p = pixel;
}

///////////////////////////////////
/*  Draw a line in surface       */
///////////////////////////////////
void drawLine(SDL_Surface* dst, int x0, int y0, int x1, int y1, Uint32 pixel)
{
    int i;
    double x = x1 - x0;
    double y = y1 - y0;
    double length = sqrt( x*x + y*y );
    double addx = x / length;
    double addy = y / length;
    x = x0;
    y = y0;

    for ( i = 0; i < length; i += 1) {
        putpixel(dst, x, y, pixel);
        x += addx;
        y += addy;
    }
}

///////////////////////////////////
/*  Print text in surface        */
///////////////////////////////////
void draw_text(SDL_Surface* dst, TTF_Font* f, char* string, int x, int y, int fR, int fG, int fB)
{
  if(dst && string && f)
  {
    SDL_Color foregroundColor={fR,fG,fB};
    SDL_Surface *textSurface=TTF_RenderText_Blended(f,string,foregroundColor);
    if(textSurface)
    {
      SDL_Rect textLocation={x,y,0,0};
      SDL_BlitSurface(textSurface,NULL,dst,&textLocation);
      SDL_FreeSurface(textSurface);
    }
  }
}

///////////////////////////////////
/*  Return text width             */
///////////////////////////////////
int text_width(char* string)
{
  int nx=0,ny=0;
  TTF_SizeText(font,string,&nx,&ny);

  return nx;
}

///////////////////////////////////
/*  Draw a rectangle             */
///////////////////////////////////
void draw_rectangle(int x, int y, int w, int h, SDL_Color* c, int border=BORDER_NO, SDL_Color* bc=NULL)
{
  SDL_Rect dest;

  // if size<=2pixels, don't draw border
  if(w<=2 || h<=2)
  {
    border=BORDER_NO;
    bc=c;
  }
  else if(border==BORDER_ROUNDED && (w<=4 || h<=4))
  {
    border=BORDER_NO;
    bc=c;
  }

  switch(border)
  {
    case BORDER_NO:
      dest.x=x;
      dest.y=y;
      dest.w=w;
      dest.h=h;
      if(c)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,c->r,c->g,c->b));
      break;
    case BORDER_SINGLE:
      dest.x=x;
      dest.y=y;
      dest.w=w;
      dest.h=h;
      if(bc)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,bc->r,bc->g,bc->b));
      dest.x=x+1;
      dest.y=y+1;
      dest.w=w-2;
      dest.h=h-2;
      if(c)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,c->r,c->g,c->b));
      break;
    case BORDER_ROUNDED:
      dest.x=x+1;
      dest.y=y+1;
      dest.w=w-2;
      dest.h=h-2;
      if(bc)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,bc->r,bc->g,bc->b));
      dest.x=x+2;
      dest.y=y;
      dest.w=w-4;
      dest.h=h;
      if(bc)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,bc->r,bc->g,bc->b));
      dest.x=x;
      dest.y=y+2;
      dest.w=w;
      dest.h=h-4;
      if(bc)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,bc->r,bc->g,bc->b));
      dest.x=x+2;
      dest.y=y+1;
      dest.w=w-4;
      dest.h=h-2;
      if(c)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,c->r,c->g,c->b));
      dest.x=x+1;
      dest.y=y+2;
      dest.w=w-2;
      dest.h=h-4;
      if(c)
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format,c->r,c->g,c->b));
      break;
  }
}

void draw_actualtime(int x, int y)
{
  time_t now=time(0);
  tm* timeinfo;
  timeinfo=localtime(&now);
  actual_time=*timeinfo;

  // time
  char timetext[20];
  if(clock_settings.format_24)
  {
    strftime(timetext,20,"%H:%M",timeinfo);
    draw_text(screen,font2,timetext,x+17,y+40,255,255,0);
  }
  else
  {
    strftime(timetext,20,"%I:%M",timeinfo);
    draw_text(screen,font2,timetext,x+17,y+40,255,255,0);
  }

  // date
  char datetext[80];
  char formatdate[20];
  formatdate[0]=0;
  if(clock_settings.date_ord1==0)
    strcat(formatdate,"%d ");
  else if(clock_settings.date_ord1==1)
    strcat(formatdate,"%B ");
  else if(clock_settings.date_ord1==2)
    strcat(formatdate,"%Y ");
  if(clock_settings.date_ord2==0)
    strcat(formatdate,"%d ");
  else if(clock_settings.date_ord2==1)
    strcat(formatdate,"%B ");
  else if(clock_settings.date_ord2==2)
    strcat(formatdate,"%Y ");
  if(clock_settings.date_ord3==0)
    strcat(formatdate,"%d");
  else if(clock_settings.date_ord3==1)
    strcat(formatdate,"%B");
  else if(clock_settings.date_ord3==2)
    strcat(formatdate,"%Y");
//strftime(datetext,80,"%d %B %Y",timeinfo);
  strftime(datetext,80,formatdate,timeinfo);
  char* text=datetext;
  while(*text)
  {
    *text=toupper((unsigned char)*text);
    text++;
  }
  int tw=text_width(datetext);
  draw_text(screen,font,datetext,x+(150-tw)/2,y+80,255,255,0);

  // AM/PM
  char amtext[20];
  if(!clock_settings.format_24)
  {
    strftime(amtext,20,"%p",timeinfo);
    draw_text(screen,font,amtext,x+120,y+40,128,128,0);
  }
  else
    draw_text(screen,font,(char*)"24h",x+120,y+40,30,30,30);

  // seconds
  char sectext[20];
  strftime(sectext,20,":%S",timeinfo);
  draw_text(screen,font,sectext,x+120,y+56,128,128,0);
}

void draw_edittime(int x, int y)
{
  // time
  char timetext[20];
  if(clock_settings.format_24)
  {
    strftime(timetext,20,"%H:%M",&edit_time);
    draw_text(screen,font2,timetext,x+17,y+40,255,255,0);
  }
  else
  {
    strftime(timetext,20,"%I:%M",&edit_time);
    draw_text(screen,font2,timetext,x+17,y+40,255,255,0);
  }

  // date
  char datetext[80];
  char formatdate[20];
  formatdate[0]=0;
  if(clock_settings.date_ord1==0)
    strcat(formatdate,"%d ");
  else if(clock_settings.date_ord1==1)
    strcat(formatdate,"%B ");
  else if(clock_settings.date_ord1==2)
    strcat(formatdate,"%Y ");
  if(clock_settings.date_ord2==0)
    strcat(formatdate,"%d ");
  else if(clock_settings.date_ord2==1)
    strcat(formatdate,"%B ");
  else if(clock_settings.date_ord2==2)
    strcat(formatdate,"%Y ");
  if(clock_settings.date_ord3==0)
    strcat(formatdate,"%d");
  else if(clock_settings.date_ord3==1)
    strcat(formatdate,"%B");
  else if(clock_settings.date_ord3==2)
    strcat(formatdate,"%Y");
//strftime(datetext,80,"%d %B %Y",timeinfo);
  strftime(datetext,80,formatdate,&edit_time);
  char* text=datetext;
  while(*text)
  {
    *text=toupper((unsigned char)*text);
    text++;
  }
  int tw=text_width(datetext);
  draw_text(screen,font,datetext,x+(150-tw)/2,y+80,255,255,0);

  // AM/PM
  char amtext[20];
  if(!clock_settings.format_24)
  {
    strftime(amtext,20,"%p",&edit_time);
    draw_text(screen,font,amtext,x+120,y+40,128,128,0);
  }
  else
    draw_text(screen,font,(char*)"24h",x+120,y+40,30,30,30);

  // seconds
  char sectext[20];
  strftime(sectext,20,":%S",&edit_time);
  draw_text(screen,font,sectext,x+120,y+56,128,128,0);
}

///////////////////////////////////
/*  Draw a clock at position x,y */
///////////////////////////////////
void draw_clock(int x, int y)
{
// 100,80
  SDL_Color color;
  SDL_Color border;

  // up
  border.r=225;
  border.g=65;
  border.b=65;
  color.r=120;
  color.g=132;
  color.b=171;
  draw_rectangle(x,y,150,22,&color,BORDER_ROUNDED,&color);
  // front
  color.r=62;
  color.g=55;
  color.b=92;
  draw_rectangle(x,y+20,150,90,&color,BORDER_SINGLE,&color);
  // inner
  color.r=23;
  color.g=17;
  color.b=26;
  draw_rectangle(x+10,y+30,130,70,&color);
  // inner shadows
  color.r=55;
  color.g=37;
  color.b=56;
  draw_rectangle(x+10,y+30,2,70,&color);
  draw_rectangle(x+10,y+30,130,2,&color);
  //button
  color.r=122;
  color.g=33;
  color.b=58;
  draw_rectangle(x+60,y+5,30,10,&color,BORDER_ROUNDED,&color);
  color.r=225;
  color.g=65;
  color.b=65;
  draw_rectangle(x+60,y+2,30,10,&color,BORDER_ROUNDED,&color);
}

///////////////////////////////////
/*  Clear values from joystick   */
/*  structure                    */
///////////////////////////////////
void clear_joystick_state()
{
  mainjoystick.j1_left=0;
  mainjoystick.j1_right=0;
  mainjoystick.j1_up=0;
  mainjoystick.j1_down=0;
  mainjoystick.button_l3=FALSE;
  mainjoystick.j2_left=0;
  mainjoystick.j2_right=0;
  mainjoystick.j2_up=0;
  mainjoystick.j2_down=0;
  mainjoystick.button_r3=FALSE;
  mainjoystick.pad_left=FALSE;
  mainjoystick.pad_right=FALSE;
  mainjoystick.pad_up=FALSE;
  mainjoystick.pad_down=FALSE;
  mainjoystick.button_a=FALSE;
  mainjoystick.button_b=FALSE;
  mainjoystick.button_x=FALSE;
  mainjoystick.button_y=FALSE;
  mainjoystick.button_l1=FALSE;
  mainjoystick.button_l2=FALSE;
  mainjoystick.button_r1=FALSE;
  mainjoystick.button_r2=FALSE;
  mainjoystick.button_select=FALSE;
  mainjoystick.button_start=FALSE;
  mainjoystick.button_power=FALSE;
  mainjoystick.button_voldown=FALSE;
  mainjoystick.button_volup=FALSE;
  mainjoystick.escape=FALSE;
  mainjoystick.any=FALSE;
}

///////////////////////////////////
/*  Load graphic with alpha      */
///////////////////////////////////
void load_imgalpha(const char* file, SDL_Surface *&dstsurface)
{
  SDL_Surface *tmpsurface;

  tmpsurface=IMG_Load(file);
  if(tmpsurface)
  {
    dstsurface=SDL_CreateRGBSurface(SDL_SRCCOLORKEY, tmpsurface->w, tmpsurface->h, 16, 0,0,0,0);
    SDL_BlitSurface(tmpsurface,NULL,dstsurface,NULL);
    SDL_SetColorKey(dstsurface,SDL_SRCCOLORKEY,SDL_MapRGB(screen->format,255,0,255));
    SDL_FreeSurface(tmpsurface);
  }
}

///////////////////////////////////
/*  Init the app                 */
///////////////////////////////////
void init_game()
{
  clock_settings.format_24=TRUE;
  clock_settings.mon_first=FALSE;
  clock_settings.date_ord1=0;
  clock_settings.date_ord2=1;
  clock_settings.date_ord3=2;

  mode_app=MODE_CLOCK;
  // Initalizations
  srand(time(NULL));
  SDL_JoystickEventState(SDL_ENABLE);
  joystick=SDL_JoystickOpen(0);
  SDL_ShowCursor(0);

  Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 1024);

  TTF_Init();
  font=TTF_OpenFontRW(SDL_RWFromMem(font_pixelberry,font_pixelberry_len),1, 8);
  font2=TTF_OpenFontRW(SDL_RWFromMem(font_atomicclockradio,font_atomicclockradio_len),1, 28);

  // Graphics
  SDL_Rect rect;
  SDL_Surface* tmpsurface;

  SDL_RWops *rw = SDL_RWFromMem(bmp_icons, bmp_icons_len);
  tmpsurface=SDL_LoadBMP_RW(rw,1);
  for(int f=0; f<10; f++)
  {
    rect.x=f*10;
    rect.y=0;
    rect.w=10;
    rect.h=10;
    img_icons[f]=SDL_CreateRGBSurface(SDL_SRCCOLORKEY, rect.w, rect.h, 16, 0,0,0,0);
    SDL_BlitSurface(tmpsurface,&rect,img_icons[f],NULL);
    SDL_SetColorKey(img_icons[f],SDL_SRCCOLORKEY,SDL_MapRGB(screen->format,255,0,255));
  }
  SDL_FreeSurface(tmpsurface);

  // Load sounds
  sound_tone=Mix_LoadWAV("media/tone.wav");
}

///////////////////////////////////
/*  Finish app, free memory      */
///////////////////////////////////
void end_game()
{
  SDL_FillRect(screen, NULL, 0x000000);

  if(SDL_JoystickOpened(0))
    SDL_JoystickClose(joystick);

  // Free graphics
  for(int f=0; f<10; f++)
    if(img_icons[f])
      SDL_FreeSurface(img_icons[f]);

  // Free sounds
  Mix_HaltChannel(-1);
  Mix_FreeChunk(sound_tone);
  Mix_CloseAudio();
}

///////////////////////////////////
/*  Process buttons events       */
///////////////////////////////////
void process_events()
{
  SDL_Event event;
  static int joy_pressed=FALSE;

  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case GCW_BUTTON_LEFT:
            mainjoystick.pad_left=TRUE;
            break;
          case GCW_BUTTON_RIGHT:
            mainjoystick.pad_right=TRUE;
            break;
          case GCW_BUTTON_UP:
            mainjoystick.pad_up=TRUE;
            break;
          case GCW_BUTTON_DOWN:
            mainjoystick.pad_down=TRUE;
            break;
          case GCW_BUTTON_Y:
            mainjoystick.button_y=TRUE;
            break;
          case GCW_BUTTON_X:
            mainjoystick.button_x=TRUE;
            break;
          case GCW_BUTTON_B:
            mainjoystick.button_b=TRUE;
            break;
          case GCW_BUTTON_A:
            mainjoystick.button_a=TRUE;
            break;
          case GCW_BUTTON_L1:
            mainjoystick.button_l1=TRUE;
            break;
          case GCW_BUTTON_L2:
            mainjoystick.button_l2=TRUE;
            break;
          case GCW_BUTTON_R1:
            mainjoystick.button_r1=TRUE;
            break;
          case GCW_BUTTON_R2:
            mainjoystick.button_r2=TRUE;
            break;
          case GCW_BUTTON_L3:
            mainjoystick.button_l3=TRUE;
            break;
          case GCW_BUTTON_R3:
            mainjoystick.button_r3=TRUE;
            break;
          case GCW_BUTTON_SELECT:
            mainjoystick.button_select=TRUE;
            break;
          case GCW_BUTTON_START:
            mainjoystick.button_start=TRUE;
            break;
          // Volume Up and Volume Down can't be detected individual
          case GCW_BUTTON_VOLUP:
            mainjoystick.button_volup=TRUE;
            mainjoystick.button_voldown=TRUE;
            break;
//          case GCW_BUTTON_VOLDOWN:
//            mainjoystick.button_voldown=1;
//            break;
          case GCW_BUTTON_POWER:
            mainjoystick.button_power=TRUE;
            break;
        }
        mainjoystick.any=TRUE;
        break;
      case SDL_JOYAXISMOTION:
        if(joy_pressed && SDL_JoystickGetAxis(joystick,0)>-GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,0)<GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,1)>-GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,1)<GCW_JOYSTICK_DEADZONE)
        {
          joy_pressed=FALSE;
        }

        if(!joy_pressed)
        {
            switch(event.jaxis.axis)
            {
              case 0:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j1_left=event.jaxis.value;
                  mainjoystick.j1_right=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j1_right=event.jaxis.value;
                  mainjoystick.j1_left=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                break;
              case 1:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j1_up=event.jaxis.value;
                  mainjoystick.j1_down=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j1_down=event.jaxis.value;
                  mainjoystick.j1_up=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                break;
              case 2:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j2_left=event.jaxis.value;
                  mainjoystick.j2_right=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j2_right=event.jaxis.value;
                  mainjoystick.j2_left=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                break;
              case 3:
                if(event.jaxis.value<0)
                {
                  mainjoystick.j2_up=event.jaxis.value;
                  mainjoystick.j2_down=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.j2_down=event.jaxis.value;
                  mainjoystick.j2_up=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.any=TRUE;
                    joy_pressed=TRUE;
                  }
                }
                break;
            }
        }
        break;
      }
  }
}

///////////////////////////////////
/*  Process keyboard and joystick*/
/*  (no events), and save in     */
/*  mainjoystick variable        */
///////////////////////////////////
void process_joystick()
{
  SDL_Event event;
  while(SDL_PollEvent(&event));

  if(keys[GCW_BUTTON_B])
    mainjoystick.button_b=TRUE;
  if(keys[GCW_BUTTON_A])
    mainjoystick.button_a=TRUE;
  if(keys[GCW_BUTTON_Y])
    mainjoystick.button_y=TRUE;
  if(keys[GCW_BUTTON_X])
    mainjoystick.button_x=TRUE;

  if(keys[GCW_BUTTON_LEFT])
    mainjoystick.pad_left=TRUE;
  if(keys[GCW_BUTTON_RIGHT])
    mainjoystick.pad_right=TRUE;
  if(keys[GCW_BUTTON_UP])
    mainjoystick.pad_up=TRUE;
  if(keys[GCW_BUTTON_DOWN])
    mainjoystick.pad_down=TRUE;

  if(SDL_JoystickGetAxis(joystick,0)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_left=SDL_JoystickGetAxis(joystick,0);
  if(SDL_JoystickGetAxis(joystick,0)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_right=SDL_JoystickGetAxis(joystick,0);
  if(SDL_JoystickGetAxis(joystick,1)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_up=SDL_JoystickGetAxis(joystick,1);
  if(SDL_JoystickGetAxis(joystick,1)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j1_down=SDL_JoystickGetAxis(joystick,1);
  if(SDL_JoystickGetAxis(joystick,2)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_left=SDL_JoystickGetAxis(joystick,2);
  if(SDL_JoystickGetAxis(joystick,2)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_right=SDL_JoystickGetAxis(joystick,2);
  if(SDL_JoystickGetAxis(joystick,3)<-GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_up=SDL_JoystickGetAxis(joystick,3);
  if(SDL_JoystickGetAxis(joystick,3)>GCW_JOYSTICK_DEADZONE)
    mainjoystick.j2_down=SDL_JoystickGetAxis(joystick,3);

  /*if(keys[GCW_BUTTON_POWER])
    mainjoystick.button_power=1;
  if(keys[GCW_BUTTON_VOLUP])
  {
    mainjoystick.button_volup=1;
    mainjoystick.button_voldown=1;
  }
  if(keys[GCW_BUTTON_VOLDOWN])
  {
    mainjoystick.button_volup=1;
    mainjoystick.button_voldown=1;
  }*/

  if(keys[GCW_BUTTON_START])
    mainjoystick.button_start=TRUE;
  if(keys[GCW_BUTTON_SELECT])
    mainjoystick.button_select=TRUE;

  if(keys[GCW_BUTTON_L1])
    mainjoystick.button_l1=TRUE;
  if(keys[GCW_BUTTON_R1])
    mainjoystick.button_r1=TRUE;
  if(keys[GCW_BUTTON_L2])
    mainjoystick.button_l2=TRUE;
  if(keys[GCW_BUTTON_R2])
    mainjoystick.button_r2=TRUE;

  if(keys[GCW_BUTTON_L3])
    mainjoystick.button_l3=TRUE;
  if(keys[GCW_BUTTON_R3])
    mainjoystick.button_r3=TRUE;
}

///////////////////////////////////
/*  Draw screen, console and     */
/*  buttons                      */
///////////////////////////////////
void draw_mode_clock()
{
  if(!edit_mode)
  {
    draw_clock(85,50);
    draw_actualtime(85,50);
    draw_text(screen,font,(char*)msg[1],100,230,255,255,255);
  }
  else
  {
    draw_clock(85,50);
    draw_edittime(85,50);
    draw_text(screen,font,(char*)"Edit-MODE",0,200,255,255,0);
    draw_text(screen,font,(char*)msg[2],100,230,255,255,255);
    draw_text(screen,font,(char*)msg[3],110+text_width((char*)msg[2]),230,255,255,255);
  }
}

///////////////////////////////////
/*  Check buttons, update actions*/
///////////////////////////////////
void update_mode_clock()
{
  if(mainjoystick.button_select)
  {
    edit_mode=TRUE;
    edit_time.tm_year=actual_time.tm_year;
    edit_time.tm_mon=actual_time.tm_mon;
    edit_time.tm_mday=actual_time.tm_mday;
    edit_time.tm_hour=actual_time.tm_hour;
    edit_time.tm_min=actual_time.tm_min;
    edit_time.tm_sec=actual_time.tm_sec;
  }
}

///////////////////////////////////
/*  Draw calendar                */
///////////////////////////////////
void draw_mode_cal()
{
}

///////////////////////////////////
/*  Update calendar              */
///////////////////////////////////
void update_mode_cal()
{
}

///////////////////////////////////
/*  Draw alarm                   */
///////////////////////////////////
void draw_mode_alarm()
{
}

///////////////////////////////////
/*  Update alarm                 */
///////////////////////////////////
void update_mode_alarm()
{
}

///////////////////////////////////
/*  Draw timer                   */
///////////////////////////////////
void draw_mode_timer()
{
}

///////////////////////////////////
/*  Update timer                 */
///////////////////////////////////
void update_mode_timer()
{
}

///////////////////////////////////
/*  Update menu icons            */
///////////////////////////////////
void update_menu()
{
  clear_joystick_state();
  //process_joystick();
  process_events();   // only process events 1 time for frame

  if(mainjoystick.button_start)
    done=TRUE;
  if(mainjoystick.button_r1)
  {
    edit_mode=FALSE;
    mode_app++;
    if(mode_app>MODE_TIMER)
      mode_app=MODE_CLOCK;
  }
  if(mainjoystick.button_l1)
  {
    edit_mode=FALSE;
    mode_app--;
    if(mode_app<MODE_CLOCK)
      mode_app=MODE_TIMER;
  }

  if(mainjoystick.button_a)
    clock_settings.format_24=TRUE;
  if(mainjoystick.button_b)
    clock_settings.format_24=FALSE;
}

///////////////////////////////////
/*  Draw menu icons              */
///////////////////////////////////
void draw_menu()
{
  // only clean screen in this process
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,6,6,6));

  // menu icons
  SDL_Rect dest;
  dest.x=0;
  dest.y=230;
  if(img_icons[0])
    SDL_BlitSurface(img_icons[0],NULL,screen,&dest);
  dest.x=50;
  if(img_icons[1])
    SDL_BlitSurface(img_icons[1],NULL,screen,&dest);

  dest.x=10;
  for(int f=0; f<4; f++)
  {
    if(f==(mode_app-1))
      SDL_BlitSurface(img_icons[f+2],NULL,screen,&dest);
    else
      SDL_BlitSurface(img_icons[f+6],NULL,screen,&dest);
    dest.x+=10;
  }

  // menu message
  draw_text(screen,font,(char*)msg[0],320-text_width((char*)msg[0]),230,255,255,255);
}

///////////////////////////////////
/*  Init                         */
///////////////////////////////////
int main(int argc, char *argv[])
{
  if(SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_AUDIO)<0)
		return 0;

  screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
  if (screen==NULL)
    return 0;

  init_game();
  load_config();

  const int GAME_FPS=60;
  Uint32 start_time;

  while(!done)
	{
    start_time=SDL_GetTicks();

    update_menu();
    draw_menu();

    switch(mode_app)
    {
      case MODE_CLOCK:
        update_mode_clock();
        draw_mode_clock();
        break;
      case MODE_CAL:
        update_mode_cal();
        draw_mode_cal();
        break;
      case MODE_ALARM:
        update_mode_alarm();
        draw_mode_alarm();
        break;
      case MODE_TIMER:
        update_mode_timer();
        draw_mode_timer();
        break;
    }

    SDL_Flip(screen);

    // set FPS 60
    if(1000/GAME_FPS>SDL_GetTicks()-start_time)
      SDL_Delay(1000/GAME_FPS-(SDL_GetTicks()-start_time));
	}

	save_config();
  end_game();

  return 1;
}
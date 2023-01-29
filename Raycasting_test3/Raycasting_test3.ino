#include "TFT_eSPI/TFT_eSPI.h"
#include "textures.c"



#define LCD_BUTTON_UP         5       //定义按钮针脚
#define LCD_BUTTON_L         13
#define LCD_BUTTON_R          8
#define LCD_BUTTON_CENTER     4
#define LCD_BUTTON_DOWN       9

#define screenWidth 160
#define screenHeight 80
#define mapWidth 24
#define mapHeight 24

#define blockup                1        // 按钮返回值定义
#define blockdown              2        //
#define blocklift              3        //
#define blockright             4        //
#define blockc                 5        //

int8_t worldMap[mapWidth][mapHeight]=
{
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
  {4,0,0,0,0,0,0,5,5,5,0,0,0,0,5,5,7,7,7,7,7,7,7,1},
  {6,6,6,6,6,6,6,6,6,0,0,0,0,0,0,6,6,6,6,6,6,6,6,6},
  {8,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,4},
  {6,6,6,6,6,6,0,0,6,6,0,0,0,0,6,6,6,6,6,6,6,6,6,6},
  {4,4,4,4,4,4,0,0,4,4,0,0,0,2,2,2,2,2,2,2,3,3,3,3},
  {4,0,0,0,0,0,0,0,0,4,6,0,0,2,0,0,0,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,0,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
  {4,0,0,5,0,0,0,0,0,4,6,0,0,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,0,2,0,0,0,0,0,2,0,0,0,2},
  {4,4,4,4,4,4,4,4,4,4,6,1,2,2,2,2,2,2,2,3,3,3,3,3}
};

void setup_buttons();
unsigned int down_up();

TFT_eSPI tft=TFT_eSPI(80,160); 


double posX=22,posY=12;
double dirX=-1,dirY=0;
double planeX=0,planeY=1;
long nowFrame=0,oldFrame=0,t1=0;
uint16_t Frame_image[screenWidth][screenHeight],ddd[screenWidth*screenHeight];
double fps;

void wblack(){
  uint16_t c;
  for(int x=0;x<screenWidth;x++){
    for(int y=0;y<screenHeight;y++){
      if(y>=screenHeight/2){
        c=TFT_OLIVE;
      }else{
        c=TFT_SKYBLUE;
      }
      Frame_image[x][y]=c;
    }
  }
}

void wddd(){
  int n=0;
  for(int x=0;x<screenWidth;x++){
    for(int y=0;y<screenHeight;y++){
      ddd[n]=Frame_image[x][y];
      n++;
    }
  }
}

void print_fps(){  //每隔1秒将帧率输出到串口 目前大概是20~27FPS 建议在需要查看FPS时再启用。
  if(millis()+t1>=1000){
    t1=millis();
    Serial.print("fps: ");
    Serial.println(fps);
    Serial.println("");
    Serial.println("");
  }
}

uint16_t change_sideTexColor(uint16_t color){
  uint16_t newColor;
  uint16_t color_Red,color_Green,color_Blue;
  color_Red=(color & 0xF800)>>11;
  color_Green=(color & 0x07E0)>>5;
  color_Blue=color & 0x001F;
 
  newColor=(int(color_Red/2)<<11) | (int(color_Green/2)<<5) | int(color_Blue/2);
  return newColor;
}

void reycasting(){
    wblack();
    for(int x=0;x<screenWidth;x++){
      double cameraX=2*x/double(screenWidth)-1;
      double rayDirX=dirX+planeX*cameraX;
      double rayDirY=dirY+planeY*cameraX;
      int mapX=int(posX);
      int mapY=int(posY);
      double sideDistX;
      double sideDistY;
      double deltaDistX=(rayDirX == 0) ? 1e30 : abs(1/rayDirX);
      double deltaDistY=(rayDirY == 0) ? 1e30 : abs(1/rayDirY);
      double perpWallDist;
      int stepX,stepY;
      int hit=0;
      int side;
      if(rayDirX<0){
        stepX=-1;
        sideDistX=(posX-mapX)*deltaDistX;
      }else{
        stepX=1;
        sideDistX=(mapX+1.0-posX)*deltaDistX;
      }
      if(rayDirY<0){
        stepY=-1;
        sideDistY=(posY-mapY)*deltaDistY;
      }else{
        stepY=1;
        sideDistY=(mapY+1.0-posY)*deltaDistY;
      }
      while(hit==0)
      {
        if(sideDistX<sideDistY){
          sideDistX+=deltaDistX;
          mapX+=stepX;
          side=0;
        }else{
          sideDistY+=deltaDistY;
          mapY+=stepY;
          side=1;
        }
        if(worldMap[mapX][mapY]>0){
          hit=1;
        }
      }
      if(side==0){
        perpWallDist=(sideDistX-deltaDistX);
      }else{
        perpWallDist=(sideDistY-deltaDistY);
      }
      int lineHeight=(int)(screenHeight/perpWallDist);
      int pitch=0;
      int drawStart=-lineHeight/2+screenHeight/2+pitch;
      if(drawStart<0){
        drawStart=0;
      }
      int drawEnd=lineHeight/2+screenHeight/2+pitch;
      if(drawEnd>=screenHeight){
        drawEnd=screenHeight-1;
      }
      uint16_t usedTexture[textureData/2],color;
      int n=0,c=0;
      int8_t texNum;
      if(worldMap[mapX][mapY]>0){
        texNum=worldMap[mapX][mapY]-1; 
        for(uint8_t a=0;a<texWidth;a++){
          for(uint8_t b=0;b<texHeight;b++){
            usedTexture[n]=texture[texNum][c]<<8|texture[texNum][c+1];
            c+=2;
            n++;
          }
        }
      }
      double wallX;
      if(side==0){
        wallX=posY+perpWallDist*rayDirY;  
      }else{
        wallX=posX+perpWallDist*rayDirX;
      }
      wallX-=floor(wallX);
      int texX=int(wallX*double(texWidth));
      if((side==0 && rayDirX>0) || (side==1 && rayDirY<0)){
        texX=texWidth-texX-1;
      }
      double oneStep=1.0*texHeight/lineHeight;
      double texPos=(drawStart-pitch-screenHeight/2+lineHeight/2)*oneStep;
      for(int y=drawStart;y<drawEnd;y++){
        int texY=(int)texPos & (texHeight-1);
        texPos+=oneStep;
        color=usedTexture[texHeight*texX+texY];
        if(side==1){
          color=change_sideTexColor(color);
        }
        Frame_image[x][y]=color;
      }
  }
  oldFrame=nowFrame;
  nowFrame=millis();
  double frameTime=(nowFrame-oldFrame)/1000.0;
  fps=(1.0/frameTime);
  double moveSpeed=frameTime*4.0;
  double rotSpeed=frameTime*2.0;
  int a=down_up();
  switch(a){
      case 0:{  
        break;
      }
      case blocklift:{
        if(worldMap[int(posX-dirX*moveSpeed)][int(posY)]==false){
          posX-=dirX*moveSpeed;
        }
        if(worldMap[int(posX)][int(posY-dirY*moveSpeed)]==false){
          posY-=dirY*moveSpeed;
        }
        break;
      }
      case blockright:{
        if(worldMap[int(posX+dirX*moveSpeed)][int(posY)]==false){
          posX+=dirX*moveSpeed;
        }
        if(worldMap[int(posX)][int(posY+dirY*moveSpeed)]==false){
          posY+=dirY*moveSpeed;
        }
        break;
      }
      case blockup:{
        double oldDirX = dirX;
        dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
        dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        break;
      }
      case blockdown:{
        double oldDirX=dirX;
        dirX=dirX*cos(rotSpeed)-dirY*sin(rotSpeed);
        dirY=oldDirX*sin(rotSpeed)+dirY*cos(rotSpeed);
        double oldPlaneX=planeX;
        planeX=planeX*cos(rotSpeed)-planeY*sin(rotSpeed);
        planeY=oldPlaneX*sin(rotSpeed)+planeY*cos(rotSpeed);
        break;
      }
      case blockc:{
        
        break;        
      }
    }
}




void setup() {
  // put your setup code here, to run once:
  Serial.begin(15200);
  tft.init(); 
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);  
  setup_buttons();
}

void loop() {
  // put your main code here, to run repeatedly:
  reycasting();
  wddd();
  tft.pushImage(0,0,80,160,ddd);
  //print_fps();   //每隔1秒将帧率输出到串口 目前大概是20~27FPS 建议在需要查看FPS时再启用。
}

//りんごあつめ 3DS
//2019/02/16 Dai Matsmoto

#include <citro2d.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SPRITES 512
#define SPRITE_SHEETS 1

typedef struct
{
  C2D_Sprite spr;
  u8 use;
  u8 visible;
  u8 sheet;
  u8 v; 
  u8 t;
  int source;
  int AnimStart;
  int AnimMsec;
  u8  AnimLoop;
  int AnimTable[33];
} Sprite;

static C2D_SpriteSheet spriteSheet[SPRITE_SHEETS];
static Sprite sprites[2][MAX_SPRITES];

static void initSprites() {
  for (size_t d = 0; d < 2; d++){
    for (size_t i = 0; i < MAX_SPRITES; i++){
      sprites[d][i].use = 0;
      sprites[d][i].visible = 1;
      sprites[d][i].AnimStart = -1;
      sprites[d][i].AnimMsec  = -1;
      for (size_t f = 0; f<=32; f++){
        sprites[d][i].AnimTable[f] = -1;
      }
    }
  }
}

static void spset(u8 display, u8 number, u8 sheet, int source){
  sprites[display][number].use    = 1;
  sprites[display][number].visible= 1;
  sprites[display][number].sheet  = sheet;
  sprites[display][number].source = source;
  C2D_SpriteFromSheet(&sprites[display][number].spr, spriteSheet[sheet], source);
  C2D_SpriteSetCenter(&sprites[display][number].spr, 0.5f, 0.5f);
  C2D_SpriteSetPos(&sprites[display][number].spr, (400-display*80)/2, 120);
  printf("%d",1);
}

static void spchr(u8 display, u8 number, u8 sheet, int source){
  u8 ox, oy, ohx, ohy;
  float oang;
  Sprite sp = sprites[display][number];
  sp.sheet  = sheet;
  sp.source = source;
  ox = sp.spr.params.pos.x;
  oy = sp.spr.params.pos.y;
  ohx = sp.spr.params.center.x;
  ohy = sp.spr.params.center.y;

  C2D_SpriteFromSheet(&sprites[display][number].spr, spriteSheet[sheet], source);
  C2D_SpriteSetPos(&sprites[display][number].spr, ox, oy);
  C2D_SpriteSetCenter(&sprites[display][number].spr, ohx, ohy);
}

static void spofs(u8 display, u8 number, float positionX, float positionY){
  C2D_SpriteSetPos(&sprites[display][number].spr, positionX, positionY);
}

static void spvis(u8 display, u8 number, u8 param){
  sprites[display][number].visible = param;
}

int main(){

  //各種ライブラリ初期化
  romfsInit();
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();

  //レンダリングターゲット
  C3D_RenderTarget* target;
  target = C2D_CreateScreenTarget(GFX_TOP   , GFX_LEFT);
  C3D_RenderTarget* bottom;
  bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);


  u8 index = 0, life = 6, endflg = 0, point = 0;
  size_t i = 0, app_cnt = 2, zPosX = 200, zPosY = 208;
 
  //テキストバッファ
  C2D_TextBuf g_Buffer;
  C2D_Text g_Text;
  g_Buffer = C2D_TextBufNew(256);
  C2D_TextParse(&g_Text, g_Buffer, "画面をタッチして落ちてくるりんごを回収しよう");
  C2D_TextOptimize(&g_Text);

  //スプライトシート
  spriteSheet[0] = C2D_SpriteSheetLoad("romfs:/images.t3x");
  if (!spriteSheet[0]) svcBreak(USERBREAK_PANIC);
  C2D_Image Image = C2D_SpriteSheetGetImage(spriteSheet[0], 0);	

  //ざる
  spset(0,0,0,1);
  spofs(0,0,200,208);

  //ライフ
  for(i=0;i<6;i++){
    spset(0,30+i,0,5);
    spofs(0,30+i,8+16*i,224);
  }

  srand(time(NULL));

 while (aptMainLoop()){
    hidScanInput();
    u32 kDown = hidKeysDown();
    if (kDown & KEY_START) break;

    //タッチ座標を取得&ざる座標に反映
    touchPosition touch;
    hidTouchRead(&touch);
    if (touch.px+touch.py) zPosX = touch.px*1.25f;
    if (endflg == 0) spofs(0,0,zPosX,zPosY);

    //新しいりんごを生成
    if (app_cnt < 30 && !(rand() % 31) && endflg == 0){
      sprites[0][app_cnt].t = rand()%3 + 2;
      spset(0,app_cnt,0,sprites[0][app_cnt].t);
      spofs(0,app_cnt,rand()%400,-32);
      sprites[0][app_cnt].v = rand() % 5+2;
      app_cnt++;
    }

    //りんごを落とす
    for (i = 2; i < app_cnt;i++){
      Sprite sp = sprites[0][i];
      spofs(0,i,sp.spr.params.pos.x,sp.spr.params.pos.y+sp.v);

      //ざるに触れたら消す
      if (sqrt(pow(sp.spr.params.pos.x - zPosX,2) + pow(sp.spr.params.pos.y - zPosY, 2)) < 32){
        spvis(0,i,0);

        //毒りんごを回収したらダメージ
        //赤りんごは10点、青りんごは20点
        if (sp.t == 4) {
          life--;
          spofs(0,i,0,240);
          spvis(0,30+life,0);
        }else{
          point += (sp.t-2) * 10;
        }
      }
    }

    //27個目のりんごが落ちるか、ライフが0になれば終了
    if (life<=0 || sprites[0][29].spr.params.pos.y>240){
      spset(0,0,0,8);
      for(i=1;i<30;i++){
        spvis(0,i,0);
        endflg = 1;
      }
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_SceneBegin(target);
    C2D_TargetClear(target, C2D_Color32(0xA0, 0xA0, 0xFF, 0xFF));
    C2D_DrawImageAt(Image, 0.0f, 0.0f, 0.0f, NULL, 1.0f, 1.0f);
    for (i = 0; i < MAX_SPRITES; i ++){
      if ((sprites[0][i].use == 1)&&(sprites[0][i].visible == 1)){
        C2D_DrawSprite(&sprites[0][i].spr);
      }
    }
    C2D_SceneBegin(bottom);
    C2D_TargetClear(bottom, C2D_Color32(0xA0, 0xA0, 0xFF, 0xFF));
    C2D_DrawText(&g_Text, 0, 24.0f, 120.0f, 0.0f, 0.5f, 0.5f);
    C3D_FrameEnd(0);

    gspWaitForVBlank();
  }


 
  return 0;
}

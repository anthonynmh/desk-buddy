// Original Pip app, MIT license. Embedded font license is included separately.
#pragma once
#include <stdint.h>
#include <math.h>
#include "PipFonts.h"
namespace pip {
inline int imin(int a,int b){return a<b?a:b;} inline int imax(int a,int b){return a>b?a:b;}
inline float clampf(float v,float a,float b){return v<a?a:(v>b?b:v);}
constexpr uint16_t rgb(int r,int g,int b){return ((r&248)<<8)|((g&252)<<3)|(b>>3);}
constexpr uint16_t BG=rgb(245,243,237),INK=rgb(30,55,49),MUTED=rgb(102,119,109),STAGE=rgb(228,236,224),WHITE=rgb(255,255,250),MINT=rgb(153,221,178),DARK=rgb(29,64,61),PEACH=rgb(242,171,137);
constexpr int PX=166,PY=158,PW=320,PH=302;
inline uint16_t blend(uint16_t a,uint16_t b,int k){int r=(((a>>11)&31)*(255-k)+((b>>11)&31)*k+127)/255,g=(((a>>5)&63)*(255-k)+((b>>5)&63)*k+127)/255,v=((a&31)*(255-k)+(b&31)*k+127)/255;return (r<<11)|(g<<5)|v;}
struct Canvas{
 uint16_t *p;int w,h,ox,oy;
 Canvas(uint16_t *v,int a,int b,int x=0,int y=0):p(v),w(a),h(b),ox(x),oy(y){}
 void rect(int x,int y,int rw,int rh,uint16_t color){int l=imax(0,x-ox),r=imin(w,x+rw-ox),t=imax(0,y-oy),b=imin(h,y+rh-oy);for(int j=t;j<b;j++)for(int i=l;i<r;i++)p[j*w+i]=color;}
 void dot(int x,int y,uint16_t color,int a=255){x-=ox;y-=oy;if(x>=0&&x<w&&y>=0&&y<h)p[y*w+x]=blend(p[y*w+x],color,a);}
 void ellipse(float x,float y,float rx,float ry,uint16_t color){if(rx<=0||ry<=0)return;for(int j=imax(oy,(int)floorf(y-ry));j<=imin(oy+h-1,(int)ceilf(y+ry));j++){float k=(j+.5f-y)/ry;if(fabsf(k)>=1)continue;float d=rx*sqrtf(1-k*k);int l=ceilf(x-d),r=floorf(x+d);rect(l,j,r-l+1,1,color);dot(l-1,j,color,(int)(clampf(l-x+d,0,1)*180));dot(r+1,j,color,(int)(clampf(x+d-r,0,1)*180));}}
 void round(int x,int y,int rw,int rh,int rad,uint16_t color){rad=imin(rad,imin(rw,rh)/2);for(int j=imax(y,oy);j<imin(y+rh,oy+h);j++){float d=j<y+rad?y+rad-j-.5f:(j>=y+rh-rad?j+.5f-y-rh+rad:0);float inset=d>0?rad-sqrtf(fmaxf(0,rad*rad-d*d)):0;int a=ceilf(inset);rect(x+a,j,rw-a*2,1,color);if(a){dot(x+a-1,j,color,(int)((a-inset)*220));dot(x+rw-a,j,color,(int)((a-inset)*220));}}}
 void line(float x,float y,float u,float v,int size,uint16_t color){int n=imax(1,(int)(hypotf(u-x,v-y)/2));for(int i=0;i<=n;i++){float t=(float)i/n;ellipse(x+(u-x)*t,y+(v-y)*t,size*.5f,size*.5f,color);}}
 void triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint16_t color){int xs[]={x0,x1,x2},ys[]={y0,y1,y2};for(int y=imax(oy,imin(y0,imin(y1,y2)));y<=imin(oy+h-1,imax(y0,imax(y1,y2)));y++){float hits[3],py=y+.5f;int n=0;for(int i=0;i<3;i++){int j=(i+1)%3;if((ys[i]<=py&&ys[j]>py)||(ys[j]<=py&&ys[i]>py))hits[n++]=xs[i]+(py-ys[i])*(xs[j]-xs[i])/(ys[j]-ys[i]);}if(n>=2){int l=ceilf(fminf(hits[0],hits[1])),r=floorf(fmaxf(hits[0],hits[1]));rect(l,y,r-l+1,1,color);}}}
 int width(const char *s,const Font &f){int v=0;for(;*s;s++){unsigned ch=(unsigned char)*s;v+=f.glyphs[(ch>=32&&ch<=126?ch:'?')-32].advance;}return v;}
 void text(int x,int y,const char *s,const Font &f,uint16_t color){for(;*s;s++){unsigned ch=(unsigned char)*s;const Glyph &g=f.glyphs[(ch>=32&&ch<=126?ch:'?')-32];int gx=x+g.dx,gy=y+g.dy;if(gx+g.w>ox&&gx<ox+w&&gy+g.h>oy&&gy<oy+h)for(int j=0;j<g.h;j++)for(int i=0;i<g.w;i++){int a=f.pixels[g.offset+j*g.w+i];if(a)dot(gx+i,gy+j,color,a);}x+=g.advance;}}
 void centered(int x,int y,const char *s,const Font &f,uint16_t color){text(x-width(s,f)/2,y,s,f,color);}
};
enum Mode{HELLO,FOCUS,PLAY,NAP};
struct State{Mode mode=HELLO;bool waving=false,happy=false;uint32_t waveAt=0,happyAt=0;float gazeX=0,gazeY=0;unsigned pets=0;const char *message="Hey, I'm Pip. Nice to meet you.";};
struct Box{int x,y,w,h;bool contains(int a,int b)const{return a>=x&&a<x+w&&b>=y&&b<y+h;}};
static const Box buttons[]={{662,220,148,112},{826,220,148,112},{662,350,148,112},{826,350,148,112}};
static const char *labels[]={"Say hello","Focus","Play","Nap"};
static const char *modes[]={"HAPPY TO BE HERE","IN QUIET COMPANY","A LITTLE HAPPY DANCE","RESTING MY PIXELS"};
inline void sparkle(Canvas&c,int x,int y,int r,uint16_t col){c.triangle(x,y-r,x-r/3,y,x+r/3,y,col);c.triangle(x,y+r,x-r/3,y,x+r/3,y,col);c.triangle(x-r,y,x,y-r/3,x,y+r/3,col);c.triangle(x+r,y,x,y-r/3,x,y+r/3,col);}
inline void heart(Canvas&c,int x,int y,int r,uint16_t col){c.ellipse(x-r/2,y,r*.58f,r*.58f,col);c.ellipse(x+r/2,y,r*.58f,r*.58f,col);c.triangle(x-r,y,x+r,y,x,y+r*1.4f,col);}
inline void icon(Canvas&c,int type,int x,int y,uint16_t col,uint16_t bg){if(type==0){c.round(x-9,y-3,18,19,6,col);for(int i=0;i<4;i++)c.round(x-10+i*5,y-17+(i==0?4:0),4,20,2,col);c.line(x-8,y+10,x-18,y-1,5,col);}else if(type==1){c.round(x-15,y-12,30,24,5,col);c.rect(x-11,y-8,22,16,bg);c.rect(x-2,y-7,3,14,col);}else if(type==2)sparkle(c,x,y,19,col);else{c.ellipse(x,y,17,17,col);c.ellipse(x+8,y-6,14,14,bg);}}
inline void scene(Canvas&c,const State&s){
 c.rect(0,0,1024,600,BG);c.ellipse(49,46,13,13,INK);c.ellipse(45,44,2,3,MINT);c.ellipse(54,44,2,3,MINT);c.text(72,17,"Pip",fontTitle,INK);c.text(173,32,"YOUR DESK COMPANION",fontSmall,MUTED);
 c.round(816,30,176,34,17,STAGE);c.ellipse(835,47,4,4,rgb(73,146,107));c.text(849,35,"HERE WITH YOU",fontSmall,INK);
 c.round(32,108,584,424,26,STAGE);c.text(58,128,modes[s.mode],fontSmall,MUTED);sparkle(c,116,227,9,rgb(180,197,176));sparkle(c,533,364,12,rgb(180,197,176));c.ellipse(528,198,4,4,rgb(183,199,179));c.ellipse(104,397,3,3,rgb(183,199,179));
 c.centered(324,466,s.message,fontBody,INK);c.centered(324,504,"Tap to greet  /  brush to make me smile",fontSmall,MUTED);
 c.round(640,108,352,424,26,WHITE);c.text(666,132,"A little company.",fontHeading,INK);c.text(666,176,"Pick our next tiny moment.",fontSmall,MUTED);
 for(int i=0;i<4;i++){const Box&b=buttons[i];bool active=s.mode==i;uint16_t fill=active?rgb(220,235,207):rgb(244,244,237);c.round(b.x,b.y,b.w,b.h,18,active?rgb(126,158,111):rgb(216,222,211));c.round(b.x+1,b.y+1,b.w-2,b.h-2,17,fill);icon(c,i,b.x+b.w/2,b.y+34,active?INK:MUTED,fill);c.centered(b.x+b.w/2,b.y+73,labels[i],fontBody,INK);}
 c.centered(816,490,"Small moments. Good company.",fontSmall,MUTED);c.text(34,561,"A calm little corner of your workday.",fontSmall,MUTED);c.text(795,561,"Made for tiny breaks.",fontSmall,MUTED);
}
inline void arc(Canvas&c,float x,float y,float rx,float ry,float start,float end,int thick,uint16_t col){for(int i=0;i<16;i++){float a=start+(end-start)*i/16,b=start+(end-start)*(i+1)/16;c.line(x+cosf(a)*rx,y+sinf(a)*ry,x+cosf(b)*rx,y+sinf(b)*ry,thick,col);}}
inline void mascot(Canvas&c,const State&s,uint32_t now){
 c.rect(PX,PY,PW,PH,STAGE);float t=now/1000.f;bool sleep=s.mode==NAP,play=s.mode==PLAY,happy=s.happy&&uint32_t(now-s.happyAt)<3500,wave=s.waving&&uint32_t(now-s.waveAt)<3200;
 float bob=sleep?sinf(t*1.5f)*2:(play?-fabsf(sinf(t*4))*14:sinf(t*2.2f)*4);int cx=PX+160+(int)(play?sinf(t*4)*7:sinf(t*1.1f)*2),cy=PY+166+(int)bob;
 c.ellipse(PX+160,PY+277,79+(int)bob,11,rgb(199,211,192));c.ellipse(PX+160,PY+277,56+(int)bob/2,6,rgb(183,202,177));
 c.ellipse(cx-48,cy+90,28,12,rgb(93,157,129));c.ellipse(cx+48,cy+90,28,12,rgb(93,157,129));float ly=cy+37+(play?sinf(t*6)*13:0);c.line(cx-83,cy+11,cx-111,ly,22,rgb(99,178,143));c.ellipse(cx-112,ly,13,16,MINT);
 float hx=cx+112,hy=cy+38;if(wave){hx=cx+118+sinf((now-s.waveAt)*.015f)*8;hy=cy-40;}else if(play)hy=cy+10+sinf(t*6+2)*18;c.line(cx+82,cy+12,hx,hy,23,rgb(99,178,143));c.ellipse(hx,hy,14,16,MINT);if(wave){c.line(hx+2,hy-9,hx+5,hy-24,7,MINT);c.line(hx-6,hy-9,hx-9,hy-23,7,MINT);}
 c.line(cx,cy-72,cx+3,cy-108,8,rgb(82,137,111));c.ellipse(cx+3,cy-111,12,12,PEACH);c.ellipse(cx,cy-114,4,4,rgb(255,223,196));
 c.round(cx-95,cy-80,190,166,58,rgb(61,119,94));c.round(cx-92,cy-79,184,159,55,rgb(113,190,148));c.round(cx-88,cy-77,176,145,52,rgb(145,215,170));c.round(cx-81,cy-74,162,123,48,rgb(171,232,190));c.ellipse(cx-55,cy-60,13,5,rgb(216,250,215));
 c.round(cx-76,cy-41,152,89,31,rgb(103,179,141));c.round(cx-73,cy-43,146,85,28,DARK);c.round(cx-68,cy-39,136,26,21,rgb(35,75,69));
 bool blink=fmodf(t+.3f,4.3f)>4.13f;int ex=s.gazeX,ey=s.gazeY;
 if(sleep){c.line(cx-43,cy-2,cx-23,cy-2,4,MINT);c.line(cx+23,cy-2,cx+43,cy-2,4,MINT);c.ellipse(cx+3,cy+24,5,6,MINT);}
 else if(happy||play){arc(c,cx-33,cy+3,11,7,3.35f,6.05f,4,MINT);arc(c,cx+33,cy+3,11,7,3.35f,6.05f,4,MINT);arc(c,cx,cy+20,12,6,.2f,2.94f,3,MINT);}
 else if(s.mode==FOCUS){for(int side=-1;side<=1;side+=2){int x=cx+side*33;c.round(x-25,cy-22,50,38,12,rgb(232,202,131));c.round(x-22,cy-19,44,32,10,DARK);c.ellipse(x+ex,cy-2+ey,6,blink?2:8,MINT);}c.rect(cx-9,cy-9,18,3,rgb(232,202,131));c.round(cx-6,cy+25,12,3,1,MINT);}
 else{for(int side=-1;side<=1;side+=2){int x=cx+side*33+ex;c.ellipse(x,cy-4+ey,9,blink?2:14,rgb(197,250,194));if(!blink)c.ellipse(x-2,cy-10+ey,3,3,WHITE);}arc(c,cx,cy+19,10,5,.2f,2.94f,3,MINT);}
 c.ellipse(cx-52,cy+20,8,4,rgb(196,136,116));c.ellipse(cx+52,cy+20,8,4,rgb(196,136,116));c.ellipse(cx,cy+64,7,7,rgb(74,134,105));c.ellipse(cx-1,cy+62,3,3,rgb(204,246,193));
 if(happy){float age=(now-s.happyAt)/1000.f;for(int i=0;i<4;i++){float u=fmodf(age*.48f+i*.23f,1);heart(c,PX+44+i*77+(int)(sinf(u*5+i)*9),PY+113-(int)(u*95),5+(int)((1-u)*5),blend(STAGE,PEACH,(int)((1-u)*240)));}}
 else if(sleep){int y=PY+46+(int)(sinf(t)*4);c.text(PX+228,y,"z",fontHeading,MUTED);c.text(PX+252,y-24,"z",fontBody,MUTED);}else if(play){sparkle(c,PX+33,PY+80+(int)(sinf(t*2)*10),10,PEACH);sparkle(c,PX+283,PY+52+(int)(cosf(t*2)*9),8,rgb(139,175,130));}
}
}

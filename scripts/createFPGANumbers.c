#include <stdio.h>




int main()
{

int lbModule,ram,addr;



for (lbModule=0; lbModule < 8 ; lbModule++)
{
 for (ram=0; ram<3; ram++)
 { 
  for (addr=0; addr <128 ;addr++)
  {

     int value=0;
     
     // Delay register nanosecs
     if (ram==0) value=0;
     
     // Width register nanosecs
     if (ram==1) value=50;

     // Amplitude register mVolts
     if (ram==2)
     {  
      // if (addr>=0 && addr<20)     value=800;
      // if (addr>=20 && addr<40)    value=1000;
      // if (addr>=40 && addr<60)    value=1200;
      // if (addr>=60 && addr<80)    value=1400;
      // if (addr>=80 && addr<100)   value=1600;
      // if (addr>=100 && addr<128)   value=1800; 
     
 value=400; int step=150;

 if (addr>=0 && addr<5)       value=400;
 if (addr>=5 && addr<10)      value=550;
 if (addr>=10 && addr<15)     value=700;
 if (addr>=15 && addr<20)     value=850;
 if (addr>=20 && addr<25)     value=1000;
 
 if (addr>=25 && addr<30)     value=1150;
 if (addr>=30 && addr<35)     value=1300;
 if (addr>=35 && addr<40)     value=1450;
 if (addr>=40 && addr<45)     value=1600;
 if (addr>=45 && addr<50)     value=1750;

 if (addr>=50 && addr<55)     value=1900;
 if (addr>=55 && addr<60)     value=2050;
 if (addr>=60 && addr<65)     value=2200;
 if (addr>=65 && addr<70)     value=2350;
 if (addr>=70 && addr<75)     value=2500;
 
 if (addr>=75 && addr<80)     value=2650;
 if (addr>=80 && addr<85)     value=2800;
 if (addr>=85 && addr<90)     value=2950;
 if (addr>=90 && addr<95)     value=3100;

 if (addr>=95 && addr<98)     value=3250; 
 if (addr>=98 && addr<100)    value=3400;   
      //value=1000;

 
            
     }

      printf("%-5d  %-5d  %-5d  %-5d\n",lbModule,ram,addr,value);
     //printf("%-5d  %-5d  %-5d  %-5d\n",lbModule,ram,addr,addr);    

  }  //addr
 }  //ram
}  //lbModule




}

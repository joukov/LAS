#include <stdio.h>



#define LB_CLR_IL_FLAG       0x0
#define LB_MEM_WEN           0x1
#define LB_MEM               0x2
#define LB_START_LOAD_SEQ    0x3
#define LB_CLR_ALL_LM_REGS   0x4
#define LB_LM_REG            0x5
#define LB_CTRL_REG          0x6
#define LB_BOARD_ID          0x7
#define LB_RUN_MODE_OFF      0x8
#define LB_RUN_MODE_ON       0x9
#define LB_MAN_TRG_OFF       0xA
#define LB_MAN_TRG_ON        0xB
#define LB_TRG_GEN_OFF       0xC
#define LB_TRG_GEN_ON        0xD
#define LB_LM_TEST_MODE_OFF  0xE
#define LB_LM_TEST_MODE_ON   0xF

//
// FPGA Register Address
//

#define LB_LASER_ON_SEL      0x0
#define LB_FAULT_IGNORE_ON   0x1
#define LB_LASER_ON          0x2
#define LB_LASER_DIODE_TRIP  0x3
#define LB_SHOT_POINTER      0x4
#define LB_REPEAT_COUNTER    0x5

#define LB_TABS_LOADED_0_3   0x8
#define LB_TABS_LOADED_4_7   0x9
#define LB_WRITE_FLAGS_0_3   0xA
#define LB_WRITE_FLAGS_4_7   0xB
#define LB_WRITE_POINTER     0xC
#define LB_GENERAL_FLAGS     0xD

//
// LaserBoard General Flags
//

#define LB_GF_ILEN          0x01
#define LB_GF_ILON          0x02
#define LB_GF_RMEN          0x04
#define LB_GF_RMON          0x08
#define LB_GF_MTON          0x10
#define LB_GF_TGON          0x20
#define LB_GF_LMTON         0x40


//
// LaserModule Register Addresses
//

#define LM_DELAY             0x0
#define LM_WIDTH             0x1
#define LM_AMPLITUDE         0x2
#define LM_BIAS              0x3
#define LM_THRESHOLD         0x4
#define LM_DACD              0x5
#define LM_CLR_LDT           0xF









int main()
{
// printf("\n hello world ! \n");
 printf("*iitem                   AM      width   address         mask            read    write   description\n");
 printf("*\n");

int lbModule,ram,addr;
long function;



//////////////////////////////////////////////////////
// SetRunModeON/OFF 
//////////////////////////////////////////////////////

  function= LB_RUN_MODE_ON << 17;
  printf("lbSetRunModeON          0d      4       00%x        ffffffff        1       1       set RunMode ON\n",
                  function);

  function= LB_RUN_MODE_OFF << 17;
  printf("lbSetRunModeOFF         0d      4       00%x        ffffffff        1       1       set RunMode OFF\n",
                  function);
 


//////////////////////////////////////////////////////
// SetLMTestON/OFF
//////////////////////////////////////////////////////

 function= LB_LM_TEST_MODE_ON << 17;
 printf("lbSetLMTestON           0d      4       00%x        ffffffff        1       1       set LM Test ON\n",
                  function);

 function= LB_LM_TEST_MODE_OFF << 17;
 printf("lbSetLMTestOFF          0d      4       00%x        ffffffff        1       1       set LM Test OFF\n",
                  function);



//////////////////////////////////////////////////////
// Set/GetLaserONSel
//////////////////////////////////////////////////////

  function= (LB_CTRL_REG << 16 | LB_LASER_ON_SEL << 8) << 1;
  printf("lbSetLaserONSel         0d      4       000%x        ffffffff        1       1       set LaserONSel\n", 
                 function);


  function= (LB_CTRL_REG << 16 | LB_LASER_ON_SEL << 8) << 1;
  printf("lbGetLaserONSel         0d      4       000%x        ffffffff        1       1       get LaserONSel\n",
                 function);


//////////////////////////////////////////////////////
// Set/GetFaultIgnore
//////////////////////////////////////////////////////

  function= (LB_CTRL_REG << 16 | LB_FAULT_IGNORE_ON << 8) << 1;
  printf("lbSetFaultIgnore        0d      4       000%x        ffffffff        1       1       set FaultIgnore\n",
                 function);

  function= (LB_CTRL_REG << 16 | LB_FAULT_IGNORE_ON << 8) << 1;
  printf("lbGetFaultIgnore        0d      4       000%x        ffffffff        1       1       get FaultIgnore\n",
                 function);


//////////////////////////////////////////////////////
// Get Laser ON
//////////////////////////////////////////////////////

  function= (LB_CTRL_REG << 16 | LB_LASER_ON << 8) << 1; 
  printf("lbGetLaserON            0d      4       000%x        ffffffff        1       1       get Laser  ON\n",
                 function);


//////////////////////////////////////////////////////
// Get LaserDiodeTrip
//////////////////////////////////////////////////////

  function= (LB_CTRL_REG << 16 | LB_LASER_DIODE_TRIP << 8) << 1;
  printf("lbGetLaserDiodeTrip     0d      4       000%x        ffffffff        1       1       get LaserDiodeTrip\n",
                 function);



//////////////////////////////////////////////////////
// getTableLoaded 2 Entries
//////////////////////////////////////////////////////

  function= (LB_CTRL_REG << 16 | LB_TABS_LOADED_0_3 << 8) << 1;
  printf("GetTableLoaded03        0d      4       000%x        ffffffff        1       0       table load  LM 0-3\n",
                  function);

  function= (LB_CTRL_REG << 16 | LB_TABS_LOADED_4_7 << 8) << 1;
  printf("GetTableLoaded47        0d      4       000%x        ffffffff        1       0       table load  LM 4-7\n",
                  function);

//////////////////////////////////////////////////////
// Read Control Registers
//////////////////////////////////////////////////////


function= (LB_CTRL_REG << 16 | LB_GENERAL_FLAGS  << 8) << 1;
printf("lbGetGeneralFlags       0d      4       000%x        ffffffff        1       0       general flags \n",
 function);


//////////////////////////////////////////////////////
// Memory Write Enable 8LM x 3 Registers = 24 Entries
//////////////////////////////////////////////////////
 
for (lbModule=0; lbModule < 8 ; lbModule++)
{
  for (ram=0; ram<3; ram++)
  { function= (1 << 16 | lbModule << 12 | ram << 8) << 1;
    printf("MemoryWriteEnable%d%d     0d      4       000%x        ffffffff        1       1       Mem Wri En LM%d Reg%d\n",lbModule,ram,function,lbModule,ram); 

  }

}


/////////////////////////////////////////////////////
// Read Status of Register  2 Entries
//////////////////////////////////////////////////////


 printf("*\n");
 printf("MemoryWriteAll          0d      4       00040000        ffffffff        1       1       Mem Write\n");
 printf("*\n");
 printf("ReadStatusRegister1     0d      4       000C1400        ffffffff        1       0       Read Stat  LM 0-3\n");
 printf("ReadStatusRegister2     0d      4       000C1600        ffffffff        1       0       Read Stat  LM 4-7\n");


//////////////////////////////////////////////////////
// getTableLoaded 2 Entries
//////////////////////////////////////////////////////
/*
  function= (LB_CTRL_REG << 16 | LB_TABS_LOADED_0_3 << 8) << 1;
  printf("GetTableLoaded03        0d      4       000%x        ffffffff        1       0       table load  LM 0-3\n",
                  function);

  function= (LB_CTRL_REG << 16 | LB_TABS_LOADED_4_7 << 8) << 1;
  printf("GetTableLoaded47        0d      4       000%x        ffffffff        1       0       table load  LM 4-7\n",
                  function);
*/


//////////////////////////////////////////////////////
// Memory Read  8LM x 3 Registers x 128   Entries
//////////////////////////////////////////////////////

 printf("*\n");

for (lbModule=0; lbModule < 8 ; lbModule++)
{
 for (ram=0; ram<3; ram++)
 { 
  for (addr=0; addr <128 ;addr++)
  { function= (2 << 16 | lbModule << 12 | ram << 8 | addr) << 1;
   char str[50];
   if (addr<10)              sprintf(str,"MemoryRead%d%d00%d",lbModule,ram,addr); 
   if (addr>=10&&addr<100)   sprintf(str,"MemoryRead%d%d0%d",lbModule,ram,addr);
   if (addr>=100)            sprintf(str,"MemoryRead%d%d%d",lbModule,ram,addr);
  // printf("%s         0d      4       000%x        ffffffff        1       1       Mem Read\n", str,function);
  printf("%s         0d      4       000%x        00000fff        1       0       Mem Read\n", str,function);    

  }  //addr
 }  //ram
}  //lbModule

printf("*\n");

//printf("\nMemoryWriteEnable00     0d      4       00020000        ffffffff        1       1       Mem Wri En LM0 Reg0");

//////////////////////////////////////////////////////
// StartLoadSequence 
//////////////////////////////////////////////////////

  function= LB_START_LOAD_SEQ << 17;
  printf("lbStartLoadSeq          0d      4       000%x        ffffffff        1       0       Start Load Seq\n",
                  function);


//////////////////////////////////////////////////////
// lmCleanUp 
//////////////////////////////////////////////////////

 function= LB_CLR_ALL_LM_REGS << 17;
 printf("lmCleanUp               0d      4       000%x        ffffffff        1       1       lmCleanUp  ON\n",
                 function);
  


//////////////////////////////////////////////////////
// lmClrLDT
//////////////////////////////////////////////////////

for (lbModule=0; lbModule < 8 ; lbModule++)
{
 char str[50];
 function=(LB_LM_REG << 16 | lbModule << 12 | LM_CLR_LDT << 8) << 1;
 sprintf(str,"lmClrLDT%d",lbModule);
 printf("%s               0d      4       000%x        ffffffff        1       1       lmCrlLDT\n", str,function);
}


//////////////////////////////////////////////////////
// Set/GetRegValue THRESHOLD
////////////////////////////////////////////////////// 

for (lbModule=0; lbModule < 8 ; lbModule++)
{ 
 function= (LB_LM_REG << 16 | lbModule << 12 | LM_THRESHOLD << 8) << 1;
 char str[50];
 sprintf(str,"lmSetRegValueTHRES%d",lbModule);
 printf("%s     0d      4       000%x        ffffffff        0       1       lmSetRegValueTHRES\n", str,function);
}



//////////////////////////////////////////////////////
// Set/GetRegValue BIAS 
//////////////////////////////////////////////////////

for (lbModule=0; lbModule < 8 ; lbModule++)
{
 function= (LB_LM_REG << 16 | lbModule << 12 | LM_BIAS << 8) << 1;
 char str[50];
 sprintf(str,"lmSetRegValueBIAS%d",lbModule);
 printf("%s      0d      4       000%x        ffffffff        0       1       lmSetRegValueBIAS\n", str,function);
}



//////////////////////////////////////////////////////
// GetRegValue BIAS
//////////////////////////////////////////////////////

for (lbModule=0; lbModule < 8 ; lbModule++)
{
 function= (LB_LM_REG << 16 | lbModule << 12 | LM_BIAS << 8) << 1;
 char str[50];
 sprintf(str,"lmGetRegValueBIAS%d",lbModule);
 printf("%s      0d      4       000%x        00000fff        1       0       lmGetRegValueBIAS\n", str,function);
}




for (lbModule=0; lbModule < 8 ; lbModule++)
{
 for (ram=0; ram<3; ram++)
   { 
     function= (LB_LM_REG << 16 | lbModule << 12 | ram << 8) << 1;
     char str[50];
     sprintf(str,"lmGetRegValue%d%d",lbModule,ram);
     //printf("%s         0d      4       000%x        ffffffff        1       1       lmGetRegValue\n", str,function);
      printf("%s         0d      4       000%x        00000fff        1       0       lmGetRegValue\n", str,function);


   }
}


// Reading shoot pointer

 function= (LB_CTRL_REG << 16 | LB_SHOT_POINTER <<  8) << 1;
 char str[50];
 sprintf(str,"lbGetShootPointer");
 printf("%s       0d      4       000%x        00000fff        1       0       lbGetShootPointer\n", str,function);


// Get/Set repeat counter 
 
 function= (LB_CTRL_REG << 16 | LB_REPEAT_COUNTER <<  8) << 1;
 //char str[50];
 sprintf(str,"lbSetRepeatCounter");
 printf("%s      0d      4       000%x        00000fff        0       1       lbSetRepeatCounter\n", str,function);

 function= (LB_CTRL_REG << 16 | LB_REPEAT_COUNTER <<  8) << 1;
 //char str[50];
 sprintf(str,"lbGetRepeatCounter");
 printf("%s      0d      4       000%x        00000fff        1       0       lbGetRepeatCounter\n", str,function);



// Set Manual TRiger ON
 
 function= LB_MAN_TRG_ON << 17;
 sprintf(str,"lbSetManTrgON");
 printf("%s           0d      4       00%x        ffffffff        1       1       lbSetManTrgON\n", str,function);


 // Set Manual TRiger OFF

 function= LB_MAN_TRG_OFF << 17;
 sprintf(str,"lbSetManTrgOFF");
 printf("%s          0d      4       00%x        ffffffff        0       1       lbSetManTrgOFF\n", str,function);



 // Set TRiger Generator ON 

 function= LB_TRG_GEN_ON << 17;
 sprintf(str,"lbSetTrgGenON");
 printf("%s           0d      4       00%x        ffffffff        0       1       lbSetTrgGenON\n", str,function);

 


///// Trigger Board ////////////////////

char strTrg[50];

sprintf(strTrg,"tbSetTestModeON");
printf("%s         0d      4       00000004        ffffffff        0       1       tbSetTestModeON\n", strTrg);
sprintf(strTrg,"tbSetTestModeOFF");
printf("%s        0d      4       00000006        ffffffff        1       1       tbSetTestModeOFF\n", strTrg);

sprintf(strTrg,"tbTriggerTTA");
printf("%s            0d      4       00000008        ffffffff        1       1       tbTriggerTTA\n", strTrg);
sprintf(strTrg,"tbTriggerTTB");
printf("%s            0d      4       0000000A        ffffffff        1       1       tbTriggerTTB\n", strTrg);
sprintf(strTrg,"tbTriggerTTC");
printf("%s            0d      4       0000000C        ffffffff        1       1       tbTriggerTTC\n", strTrg);
sprintf(strTrg,"tbTriggerTTD");
printf("%s            0d      4       0000000E        ffffffff        1       1       tbTriggerTTD\n", strTrg);

/*
TestModeOn              0d      4       00000004        ffffffff        0       1       switches Test Mode On
TestModeOff             0d      4       00000006        ffffffff        1       1       switches Test Mode Off
SimulateTriggerTTCA     0d      4       00000008        ffffffff        1       1       Simulate Trigger on TTCA
SimulateTriggerTTCB     0d      4       0000000A        ffffffff        1       1       Simulate Trigger on TTCB
SimulateTriggerTTCC     0d      4       0000000C        ffffffff        1       1       Simulate Trigger on TTCC
SimulateTriggerTTCD     0d      4       0000000E        ffffffff        1       1       Simulate Trigger on TTCD
*/





}

/*************************************************************************
  KD8CEC's uBITX Display Routine for TFT320240 SPI
  Uses SPI interface  
-----------------------------------------------------------------------------
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************************/
#include "ubitx.h"

//======================================================================== 
//Begin of TFT320240 Library by EA4GZI
//========================================================================

void clearTFT()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

TFT_eSPI_Button btFreq[9];         // buttons frequency display
TFT_eSPI_Button btFreqTx[9];       // buttons frequency display TX at ritOn & SplitOn
TFT_eSPI_Button btMain[10];        // buttons main display
TFT_eSPI_Button btNav[5];          // buttons main display
TFT_eSPI_Button btCal[5];          // buttons Calibration display
TFT_eSPI_Button btSet[5];          // buttons Setting display
TFT_eSPI_Button btNet[5];          // buttons Setting Net display
TFT_eSPI_Button btFlot[5];         // buttons flotantes
TFT_eSPI_Button btSta[5];          // buttons Status
TFT_eSPI_Button btYN[3];           // buttons OK / Cancel / Backspace
TFT_eSPI_Button btKey[50];        // buttons Key 0..9, A..Z & symbols
byte btMainact[10]={0,0,0,0,0,0,0,0,0,0};
byte btCalact[15]={1,1,1,0,1};
byte btSetact[15]={1,1,1,1,1};
byte btNetact[15]={1,1,1,1,1};
byte btNavact[5]={1,0,0,0,1};
byte btFlotact[5]={1,0,0,0,0};
char btMaintext[15][8]={"RX","TX","A/B","Band","Band","LSB","USB","CW","RIT","SPL"};
char btCaltext[5][15]={"Calibration","BFO","SI5351 ADDR","xxx","Reset"};
char btSettext[5][15]={"Language","CallSign","Latitude","Longitude","xxx"};
char btNettext[5][15]={"Auto Connect","SSID","Password","Mode","Static IP"};
char btNavtext[5][8]={"<","xxx","xxx","xxx",">"};
char btFlottext[5][8]={"Ent","xxx","xxx","xxx","xxx"};
char btYNtext[3][8]={"OK","Cancel","<--"};
char btKeytext[50][2]={"0","1","2","3","4","5","6","7","8","9",
                       "A","B","C","D","E","F","G","H","I","J",
                       "K","L","M","N","Ñ","O","P","Q","R","S",
                       "T","U","V","W","X","Y","Z","x","x","x",
                       "-","=","_",".",",",";","/","(",")","x"};
char softBuffLines[2][20 + 1];
char softBuffSended[2][20 + 1];

char auxc[30], auxb[30];
char softBuff[20];
char softTemp[20];

void TFT_Init()
{
  Serial2.println("TFT_Init");
  tft.init(); tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Init...",0,0);
}

//===================================================================
//Begin of Nextion LCD Protocol
// v0~v9, va~vz : Numeric (Transceiver -> Nextion LCD)
// s0~s9  : String (Text) (Transceiver -> Nextion LCD)
// vlSendxxx, vloxxx: Reserve for Nextion (Nextion LCD -> Transceiver)
//===================================================================
#define CMD_NOW_DISP      '0' //c0
char L_nowdisp = -1;          //Sended nowdisp

#define CMD_VFO_TYPE      'v' //cv
char L_vfoActive;             //vfoActive

#define CMD_CURR_FREQ     'c' //vc
unsigned long L_vfoCurr;      //vfoA
#define CMD_CURR_MODE     'c' //cc
byte L_vfoCurr_mode;          //vfoA_mode

#define CMD_VFOA_FREQ     'a' //va
unsigned long L_vfoA;         //vfoA
#define CMD_VFOA_MODE     'a' //ca
byte L_vfoA_mode;             //vfoA_mode

#define CMD_VFOB_FREQ     'b' //vb
unsigned long L_vfoB;         //vfoB
#define CMD_VFOB_MODE     'b' //cb
byte L_vfoB_mode;             //vfoB_mode

#define CMD_IS_RIT        'r' //cr
char L_ritOn;
#define CMD_RIT_FREQ      'r' //vr
unsigned long L_ritTxFrequency; //ritTxFrequency

#define CMD_IS_TX         't' //ct
char L_inTx;

#define CMD_IS_DIALLOCK   'l' //cl
byte L_isDialLock;            //byte isDialLock

#define CMD_IS_SPLIT      's' //cs
byte  L_Split;            //isTxType
#define CMD_IS_TXSTOP     'x' //cx
byte  L_TXStop;           //isTxType

#define CMD_TUNEINDEX     'n' //cn
byte L_tuneStepIndex;     //byte tuneStepIndex

#define CMD_SMETER        'p' //cs
byte L_scaledSMeter;      //scaledSMeter

#define CMD_SIDE_TONE     't' //vt
unsigned long L_sideTone; //sideTone
#define CMD_KEY_TYPE      'k' //ck
byte L_cwKeyType = -1;          //L_cwKeyType 0: straight, 1 : iambica, 2: iambicb

#define CMD_CW_SPEED      's' //vs
unsigned int L_cwSpeed;   //cwSpeed

#define CMD_CW_DELAY      'y' //vy
byte L_cwDelayTime=-1;       //cwDelayTime

#define CMD_CW_STARTDELAY 'e' //ve
byte L_delayBeforeCWStartTime=-1;  //byte delayBeforeCWStartTime

#define CMD_ATT_LEVEL     'f' //vf
byte L_attLevel;

byte L_isIFShift;             //1 = ifShift, 2 extend
#define CMD_IS_IFSHIFT    'i' //ci

int L_ifShiftValue;
#define CMD_IFSHIFT_VALUE 'i' //vi

byte L_sdrModeOn;
#define CMD_SDR_MODE      'j' //cj

#define CMD_UBITX_INFO     'm' //cm  Complete Send uBITX Information

byte L_isShiftDisplayCWFreq;  //byte isShiftDisplayCWFreq
int L_shiftDisplayAdjustVal;        //int shiftDisplayAdjustVal

//0:CW Display Shift Confirm, 1 : IFshift save
byte L_commonOption0;         //byte commonOption0

//0:Line Toggle, 1 : Always display Callsign, 2 : scroll display, 3 : s.meter
byte L_displayOption1;            //byte displayOption1
byte L_displayOption2;            //byte displayOption2 (Reserve)

#define TS_CMD_MODE           1
#define TS_CMD_FREQ           2
#define TS_CMD_BAND           3
#define TS_CMD_VFO            4
#define TS_CMD_SPLIT          5
#define TS_CMD_RIT            6
#define TS_CMD_TXSTOP         7
#define TS_CMD_SDR            8
#define TS_CMD_LOCK           9 //Dial Lock
#define TS_CMD_ATT           10 //ATT
#define TS_CMD_IFS           11 //IFS Enabled
#define TS_CMD_IFSVALUE      12 //IFS VALUE
#define TS_CMD_STARTADC      13
#define TS_CMD_STOPADC       14
#define TS_CMD_SPECTRUMOPT   15 //Option for Spectrum
#define TS_CMD_SPECTRUM      16 //Get Spectrum Value
#define TS_CMD_TUNESTEP      17 //Get Spectrum Value
#define TS_CMD_WPM           18 //Set WPM
#define TS_CMD_KEYTYPE       19 //Set KeyType

#define TS_CMD_SWTRIG        21 //SW Action Trigger for WSPR and more
#define TS_CMD_READMEM       31 //Read EEProm
#define TS_CMD_WRITEMEM      32 //Write EEProm
#define TS_CMD_LOOPBACK0     74 //Loopback1 (Response to Loopback Channgel)
#define TS_CMD_LOOPBACK1     75 //Loopback2 (Response to Loopback Channgel)
#define TS_CMD_LOOPBACK2     76 //Loopback3 (Response to Loopback Channgel)
#define TS_CMD_LOOPBACK3     77 //Loopback4 (Response to Loopback Channgel)
#define TS_CMD_LOOPBACK4     78 //Loopback5 (Response to Loopback Channgel)
#define TS_CMD_LOOPBACK5     79 //Loopback6 (Response to Loopback Channgel)
#define TS_CMD_FACTORYRESET  85 //Factory Reset
#define TS_CMD_UBITX_REBOOT  95 //Reboot

char nowdisp = 0;

#define SWS_HEADER_INT_TYPE  'v'  //Numeric Protocol Prefex

//Control must have prefix 'v' or 's'
const byte ADCIndex[6] = {A0, A1, A2, A3, A6, A7};

void SendCommandUL(char varIndex, unsigned long sendValue)
{
  memset(softTemp, 0, 20);
  ultoa(sendValue, softTemp, DEC);
}

void SendCommandL(char varIndex, long sendValue)
{
  memset(softTemp, 0, 20);
  ltoa(sendValue, softTemp, DEC);
}

uint8_t softBuff1Num[14] = {'p', 'm', '.', 'c', '0', '.', 'v', 'a', 'l', '=', 0, 0xFF, 0xFF, 0xFF};
void SendCommand1Num(char varType, char sendValue) //0~9 : Mode, nowDisp, ActiveVFO, IsDialLock, IsTxtType, IsSplitType
{
  softBuff1Num[4] = varType;
  softBuff1Num[10] = sendValue + 0x30;

   for (int i = 0; i < 14; i++)
     {
     //EA4GZI     SWSerial_Write(softBuff1Num[i]);
     }
}

void SetSWActivePage(char newPageIndex)
{
    if (L_nowdisp != newPageIndex)
    {
      L_nowdisp = newPageIndex;
      SendCommand1Num(CMD_NOW_DISP, L_nowdisp);
    }
}
//===================================================================
//End of Nextion LCD Protocol
//===================================================================

// The generic routine to display one line on the LCD 
void printLine(unsigned char linenmbr, const char *c) {
  tft.drawString(c,0,20*linenmbr);
}

void printLineF(char linenmbr, const __FlashStringHelper *c)
{
  int i;
  char tmpBuff[21];
  PGM_P p = reinterpret_cast<PGM_P>(c);  
  for (i = 0; i < 21; i++){
    unsigned char fChar = pgm_read_byte(p++);
    tmpBuff[i] = fChar;
    if (fChar == 0)
      break;
  }
  printLine(linenmbr, tmpBuff);
}

void printLineFromEEPRom(char linenmbr, char lcdColumn, byte eepromStartIndex, byte eepromEndIndex, char offsetTtype) 
{
  int colIndex = lcdColumn;
  for (byte i = eepromStartIndex; i <= eepromEndIndex; i++)
    {
    if (++lcdColumn <= 20)
      softBuffLines[linenmbr][colIndex++] = EEPROM.read((offsetTtype == 0 ? USER_CALLSIGN_DAT : WSPR_MESSAGE1) + i);
    else
      break;
    }
}

void clearLine2()
{
  printLine(0,"");
  line2DisplayStatus = 0;
}

void printLine2ClearAndUpdate(){
  printLine(0, "");
  line2DisplayStatus = 0;  
  updateDisplay(true,false);
}
//End of Display Base Routines
//==================================================================================

//==================================================================================
//Begin of User Interface Routines
//Main Display for Nextion LCD
byte nowPageIndex = 0;

//sendType == 1 not check different 
void sendUIData(int sendType)
{
  char nowActiveVFO = conf.vfoActive == VFO_A ? 0 : 1;
  if (L_vfoActive != nowActiveVFO)
    {
    L_vfoActive = nowActiveVFO;
    SendCommand1Num(CMD_VFO_TYPE, L_vfoActive);
    }

  if (L_vfoCurr != conf.frequency)
  {
    L_vfoCurr = conf.frequency;
    SendCommandUL(CMD_CURR_FREQ, conf.frequency);
  }

  byte vfoCurr_mode = modeToByte();
  if (L_vfoCurr_mode != vfoCurr_mode)
  {
    L_vfoCurr_mode = vfoCurr_mode;
    SendCommand1Num(CMD_CURR_MODE, L_vfoCurr_mode);
  }

  //if auto cw key mode, exit
  //if (isCWAutoMode != 0 || menuOn != 0)
  if (isCWAutoMode != 0)
    return;

  //nowPageIndex = 0;
  if (menuOn==0)
    {
    if (sendType == 0)
      {
      SetSWActivePage(0);
      }
    else
      {
      SetSWActivePage(0);
      }
    }
  else
    {
    //Text Line Mode
      SetSWActivePage(1);
    }

  //VFOA
  if (L_vfoA != conf.vfoA)
    {
    L_vfoA = conf.vfoA;
    SendCommandUL(CMD_VFOA_FREQ, L_vfoA);
    }

  //#define CMD_VFOA_MODE     'a' //ca
  if (L_vfoA_mode != conf.vfoA_mode)
    {
    L_vfoA_mode = conf.vfoA_mode;
    SendCommand1Num(CMD_VFOA_MODE, L_vfoA_mode);
    }

  //VFOB
  if (L_vfoB != conf.vfoB)
    {
    L_vfoB = conf.vfoB;
    SendCommandUL(CMD_VFOB_FREQ, L_vfoB);
    }

  if (L_vfoB_mode != conf.vfoB_mode)
    {
    L_vfoB_mode = conf.vfoB_mode;
    SendCommand1Num(CMD_VFOB_MODE, L_vfoB_mode);  
    }

  if (L_isDialLock != isDialLock)
    {
    L_isDialLock = isDialLock;
    SendCommand1Num(CMD_IS_DIALLOCK, L_isDialLock);  
    }

  if (L_ritOn != conf.ritOn)
    {
    L_ritOn = conf.ritOn;
    SendCommand1Num(CMD_IS_RIT, L_ritOn);  
    }
  
  if (L_ritTxFrequency != conf.ritTxFrequency)
    {
    L_ritTxFrequency = conf.ritTxFrequency;
    SendCommandUL(CMD_RIT_FREQ, L_ritTxFrequency);  
    }

  if (L_inTx != inTx)
  {
    L_inTx = inTx;
    SendCommand1Num(CMD_IS_TX, L_inTx);  
  }

  if (L_isDialLock != isDialLock)
  {
    L_isDialLock = isDialLock;
    SendCommand1Num(CMD_IS_DIALLOCK, L_isDialLock);  
  }

  if (L_Split != conf.splitOn)
  {
    L_Split = conf.splitOn;
    SendCommand1Num(CMD_IS_SPLIT, L_Split);  
  }
  

  byte isTXStop = ((isTxType & 0x01) == 0x01);
  if (L_TXStop != isTXStop)
  {
    L_TXStop = isTXStop;
    SendCommand1Num(CMD_IS_TXSTOP, L_TXStop);
  }

  if (L_tuneStepIndex != conf.tuneStepIndex)
  {
    L_tuneStepIndex = conf.tuneStepIndex;
    SendCommand1Num(CMD_TUNEINDEX, L_tuneStepIndex);
  }

  if (L_scaledSMeter != conf.scaledSMeter)
  {
    L_scaledSMeter = conf.scaledSMeter;
    SendCommand1Num(CMD_SMETER, L_scaledSMeter);  
  }

  if (L_sideTone != conf.sideTone)
  {
    L_sideTone = conf.sideTone;
    SendCommandL(CMD_SIDE_TONE, L_sideTone);
  }

  if (L_cwKeyType != conf.cwKeyType) {
    L_cwKeyType = conf.cwKeyType;
    SendCommand1Num(CMD_KEY_TYPE, L_cwKeyType);  
  }

  if (L_cwSpeed != conf.cwSpeed)
  {
    L_cwSpeed = conf.cwSpeed;
    SendCommandL(CMD_CW_SPEED, L_cwSpeed);  
  }

  if (L_cwDelayTime != conf.cwDelayTime)
  {
    L_cwDelayTime = conf.cwDelayTime;
    SendCommandL(CMD_CW_DELAY, L_cwDelayTime);  
  }

  if (L_delayBeforeCWStartTime != conf.delayBeforeCWStartTime)
  {
    L_delayBeforeCWStartTime = conf.delayBeforeCWStartTime;
    SendCommandL(CMD_CW_STARTDELAY, L_delayBeforeCWStartTime);
  }

  if (L_attLevel != conf.attLevel)
  {
    L_attLevel = conf.attLevel;
    SendCommandL(CMD_ATT_LEVEL, L_attLevel);
  }

  if (L_isIFShift != isIFShift)
  {
    L_isIFShift = isIFShift;
    SendCommand1Num(CMD_IS_IFSHIFT, L_isIFShift);
  }

  if (L_ifShiftValue != conf.ifShiftValue)
  {
    L_ifShiftValue = conf.ifShiftValue;
    SendCommandL(CMD_IFSHIFT_VALUE, L_ifShiftValue);
  }

  if (L_sdrModeOn != conf.sdrModeOn)
  {
    L_sdrModeOn = conf.sdrModeOn;
    SendCommand1Num(CMD_SDR_MODE, L_sdrModeOn);
  }
}

void displayTime()
{
  if (!WiFi.isConnected()) return;
  btSta[1].initButtonUL(&tft,150,220,30,20,2,TFT_BLACK,TFT_WHITE,itoa(hour(),buff,10),2);
  btSta[1].drawButton();
  btSta[2].initButtonUL(&tft,186,220,30,20,2,TFT_BLACK,TFT_WHITE,itoa(minute(),buff,10),2);
  btSta[2].drawButton();
  btSta[3].initButtonUL(&tft,219,220,30,20,2,TFT_BLACK,TFT_WHITE,itoa(second(),buff,10),2);
  btSta[3].drawButton();
  tft.setTextSize(2);  tft.setTextColor(TFT_WHITE);
  tft.drawString(":",210,220); 
  tft.drawString(":",178,220); 
}

void displayStatus()
{
  if ((inEntN) || (inEntA)) return;   // entradas de teclado 
  btSta[0].initButtonUL(&tft,65,210,50,30,2,WiFi.isConnected()?TFT_GREEN:TFT_RED,TFT_BLACK,"WiFi",1);
  btSta[0].drawButton();
  displayTime();
}

void displayFreq()
{
  char freqpart[9][4]={"","","","","","","","",""};
  char freqpartSec[9][4]={"","","","","","","","",""};
  unsigned long f=conf.frequency;
  unsigned long fsec;
  if (conf.ritOn==1)
    fsec=conf.frequencyB;
  else if (conf.splitOn==1)
    fsec=conf.frequencyB;
  for (byte i=1;i<6;i++)    // no se escribe la cifra de centena de Mhz
    {
    long auxL=long(pow(10,8-i));
    strcat(freqpart[i],itoa((f/auxL)%10,buff,10));
    strcat(freqpartSec[i],itoa((fsec/auxL)%10,buff,10));
    btFreq[i].initButtonUL(&tft,90+26*i,70,26,40,2,TFT_BLACK,conf.tuneStepIndex==i?TFT_YELLOW:TFT_WHITE,freqpart[i],i>=6?3:4);
    btFreq[i].drawButton();
    }
  for (byte i=6;i<9;i++)
    {
    long auxL=long(pow(10,8-i));
    strcat(freqpart[i],itoa((f/auxL)%10,buff,10));
    strcat(freqpartSec[i],itoa((fsec/auxL)%10,buff,10));
    btFreq[i].initButtonUL(&tft,126+20*i,75,20,30,2,TFT_BLACK,conf.tuneStepIndex==i?TFT_YELLOW:TFT_WHITE,freqpart[i],i>=6?3:4);
    btFreq[i].drawButton();
    }
  tft.fillCircle(164,105,2,TFT_WHITE);
  if ((conf.ritOn==1) || (conf.splitOn==1))
    {
    tft.setTextSize(3); tft.setTextColor(TFT_LIGHTGREY);
    tft.drawString("TX",60,125);
    for (byte i=1;i<6;i++)    // no se escribe la cifra de centenas de Mhz
      {
      btFreqTx[i].initButtonUL(&tft,90+26*i,115,26,40,2,TFT_BLACK,TFT_LIGHTGREY,freqpartSec[i],i>=6?3:4);
      btFreqTx[i].drawButton();
      }
    for (byte i=6;i<9;i++)
      {
      btFreqTx[i].initButtonUL(&tft,126+20*i,122,20,30,2,TFT_BLACK,TFT_LIGHTGREY,freqpartSec[i],i>=6?3:4);
      btFreqTx[i].drawButton();
      }
    tft.fillCircle(164,145,2,TFT_LIGHTGREY);
    }
  tft.setTextSize(2); tft.setTextColor(TFT_RED, TFT_BLACK);
  byte auxf=getIndexHambanBbyFreq(f);
  tft.drawString(((f>=conf.hamBandRange[auxf][0]*1000) && (f<=conf.hamBandRange[auxf][1]*1000))?"   ":"OUT",65,90);
}

void displayNav()
{
  // botones navegación
  for (byte i=0;i<5;i++) if (btNavact[i]==1)
    {
    btNav[i].initButtonUL(&tft,62*i,210,60,30,2,TFT_WHITE,TFT_BLACK,btNavtext[i],2);
    btNav[i].drawButton();
    }
}

void displayYN()
{
  // botones Yes/Cancel
  for (byte i=0;i<3;i++)
    {
    btYN[i].initButtonUL(&tft,100*i,210,90,30,2,TFT_WHITE,TFT_BLACK,btYNtext[i],2);
    btYN[i].drawButton();
    }
}

void displayCal()
{
  // botones setting
  for (byte i=0;i<5;i++) if (btCalact[i]==1)
    {
    btCal[i].initButtonUL(&tft,0,35*i+30,160,30,2,TFT_WHITE,TFT_BLACK,btCaltext[i],2);
    btCal[i].drawButton();
    }
}

void displayKey()    
{
  clearTFT();
  for (byte i=0;i<50;i++)
    {
    btKey[i].initButtonUL(&tft,31*(i%10),35+(31*int(i/10)),30,30,2,TFT_WHITE,TFT_BLACK,btKeytext[i],2);
    btKey[i].drawButton();
    }
  displayYN();
}

void displaySet()
{
  // botones setting
  for (byte i=0;i<5;i++) if (btSetact[i]==1)
    {
    btSet[i].initButtonUL(&tft,0,35*i+30,160,30,2,TFT_WHITE,TFT_BLACK,btSettext[i],2);
    btSet[i].drawButton();
    }
}

void displayNet()
{
  // botones setting Net
  for (byte i=0;i<5;i++) if (btNetact[i]==1)
    {
    btNet[i].initButtonUL(&tft,0,35*i+30,160,30,2,TFT_WHITE,TFT_BLACK,btNettext[i],2);
    btNet[i].drawButton();
    }
}

void displayFlot()
{
  // botones flotantes
  btFlot[0].initButtonUL(&tft,0,70,60,30,2,TFT_WHITE,TFT_BLACK,btFlottext[0],2);
  btFlot[0].drawButton();
}

void displayMain()
{
  initButtons();
  // botones superiores
  for (byte i=0;i<5;i++)
    {
    btMain[i].initButtonUL(&tft,62*i,0,60,30,2,btMainact[i]==0?TFT_WHITE:i==1?TFT_RED:TFT_YELLOW,TFT_BLACK,btMaintext[i],2);
    btMain[i].drawButton();
    }
  for (byte i=5;i<10;i++)
    {
    btMain[i].initButtonUL(&tft,62*(i-5),35,60,30,2,btMainact[i]==0?TFT_WHITE:TFT_YELLOW,TFT_BLACK,btMaintext[i],2);
    btMain[i].drawButton();
    }
}

void updateDisplay(boolean alldata, boolean freqdata) {
    freqdata=true; 
    clearTFT();
    if (tftpage==0)   // Main page
      {
      if (alldata) 
        {
        displayMain();
        displayNav();
        displayFlot();
        displayStatus();    // time, status
        }
      displayFreq();   // frecuencia
      }
    else if (tftpage==1)    // Setup
      {
      // data
      displaySet();
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Setup",0,0);
      tft.drawString(conf.lang==0?"Español":"English",180,40);
      tft.drawString(conf.CallSign,180,75);
      tft.drawNumber(conf.latitud,180,110);
      tft.drawNumber(conf.longitud,180,145);
      displayNav();
      displayStatus();
      }
    else if (tftpage==2)    // WiFi
      {
      // data
      displayNet();
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Setup WiFi",0,0);
      tft.drawString(conf.autoWiFi==1?"YES":"NO",180,40);
      tft.drawString(conf.ssidSTA,180,75);
      tft.drawString(conf.passSTA,180,110);
      tft.drawString(conf.wifimode==0?"AP":conf.wifimode==1?"STA":"AP+STA",180,145);
      tft.drawString(conf.staticIP==1?"YES":"NO",180,180);
      displayNav();
      displayStatus();
      }
    else if (tftpage==3)    // Calibration
      {
      // data
      displayCal();
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Calibration",0,0);
      tft.drawNumber(conf.calibration,180,40);
      tft.drawNumber(conf.usbCarrier,180,75);
      tft.drawNumber(conf.SI5351BX_ADDR,180,110);
      displayNav();
      displayStatus();
      }
}

//****************************************************************
// Spectrum for Range scan and Band Scan
//****************************************************************
#define RESPONSE_SPECTRUM     0
#define RESPONSE_EEPROM       1
#define RESPONSE_EEPROM_HEX_R 72  //Nextion order (Reverse)
#define RESPONSE_EEPROM_STR   87  //String

const uint8_t ResponseHeader[11]={'p', 'm', '.', 's', 'h', '.', 't', 'x', 't', '=', '"'};
const char HexCodes[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', };

//void sendSpectrumData(unsigned long startFreq, unsigned long incStep, int scanCount, int delayTime, int sendCount)
//sendResponseData(RESPONSE_EEPROM, 0, eepromIndex, eepromReadLength, eepromDataType, 1);
//protocol Type : 0 - Spectrum, 1 : EEProm
//startFreq   : Spectrum - Frequency, EEProm - 0
//sendOption1 : Spectrum - 1 Step Frequency, EEProm - EEProm Start Address
//scanCount   : Spectrum - 1 Set Length, EEProm - Read Length
//sendOption2 : Spectrum - Value offset (because support various S-Meter), EEProm - EEProm Response DataType (0:HEX, 1:String)
//sendCount : Spectrum - All scan set count, EEProm - always 1

void sendResponseData(int protocolType, unsigned long startFreq, unsigned int sendOption1, int readCount, int sendOption2, int sendCount)  //Spectrum and EEProm Data
{
  unsigned long beforFreq = conf.frequency;
  unsigned long k;
  uint8_t adcBytes[200];    //Maximum 200 Step
  
  //Voltage drop
  //scanResult[0] = analogRead(ANALOG_SMETER);
  //adcBytes[0] = analogRead(ANALOG_SMETER);
  //delay(10);
  int readedValue = 0;

  for (int auxsi = 0; auxsi < sendCount; auxsi++)
    {
    for (int i = 0; i < 11; i++)
      {
      //EA4GZI   SWSerial_Write(ResponseHeader[i]);
      }
      
    for (k = 0; k < readCount; k ++)
      {
      if (protocolType == RESPONSE_SPECTRUM)
        {
        //Spectrum Data
        //Sampling Range
        setFrequency(startFreq + (k * sendOption1));
        //Wait time for charging
        //delay(10);

#ifdef USE_I2CSMETER 
        readedValue = GetI2CSmeterValue(I2CMETER_UNCALCS);
#else
        //ADC
        readedValue = analogRead(ANALOG_SMETER);
        readedValue -= (sendOption2 * 3); //0 ~ 765
        //Down Scale
        readedValue /= 2;
        if (readedValue < 0) { readedValue = 0; }
        else if (readedValue>255) { readedValue=255; }
#endif        
        }
      else
        {
        readedValue = EEPROM.read(((sendOption2 == RESPONSE_EEPROM_HEX_R) ? (readCount - k - 1) : k) + sendOption1);
        }

      if (protocolType == RESPONSE_EEPROM && sendOption2 == RESPONSE_EEPROM_STR) //None HEX
        {
        //EA4GZI SWSerial_Write(readedValue);
        }
      else
        {
        //EA4GZI         SWSerial_Write(HexCodes[readedValue >> 4]);
        //EA4GZI         SWSerial_Write(HexCodes[readedValue & 0xf]);
        }
      }
   
    } //end of for
}

//****************************************************************
//Receive command and processing from External device (LCD or MCU)
//****************************************************************
int spectrumSendCount = 10;   //count of full scan and Send
int spectrumOffset = 0;    //offset position
int spectrumScanCount = 100;  //Maximum 200
unsigned int spectrumIncStep = 1000;   //Increaase Step
extern uint8_t receivedCommandLength;
extern void SWSerial_Read(uint8_t * receive_cmdBuffer);
uint8_t swr_buffer[20];

char checkCount = 0;
char checkCountSMeter = 0;

//execute interval : 0.25sec
void idle_process()
{
  //S-Meter Display
  if (((conf.displayOption1 & 0x08) == 0x08 && (conf.sdrModeOn == 0)) && (++checkCountSMeter > SMeterLatency))
  {
#ifdef USE_I2CSMETER 
    readconf();
#else
    int newSMeter;
    newSMeter = analogRead(ANALOG_SMETER) / 4;
    currentSMeter = newSMeter;
    conf.scaledSMeter = 0;
    for (byte s = 8; s >= 1; s--) {
      if (currentSMeter > conf.sMeterLevels[s]) {
        conf.scaledSMeter = s;
        break;
      }
    }
#endif  
    checkCountSMeter = 0; //Reset Latency time
    } //end of S-Meter
  sendUIData(1);
}

//AutoKey LCD Display Routine
void Display_AutoKeyTextIndex(byte textIndex)
{
  byte diplayAutoCWLine = 0;
  if ((conf.displayOption1 & 0x01) == 0x01)
    diplayAutoCWLine = 1;
  softBuffLines[diplayAutoCWLine][0] = byteToChar(textIndex);
  softBuffLines[diplayAutoCWLine][1] = ':';
}

void DisplayVersionInfo(const __FlashStringHelper * fwVersionInfo)
{
  tft.drawString("Version:",0,20);
  tft.drawString(fwVersionInfo,140,20);
}

void  initButtons()
{
  btMainact[0]=(inTx==0);   // RX
  btMainact[1]=(inTx==1);   // TX
  btMainact[2]=(conf.vfoActive==VFO_B);   // Vfo A/B
  btMainact[3]=0;   // Band Down
  btMainact[4]=0;   // Band Up
  btMainact[5]=(conf.isUSB==0);   // LSB
  btMainact[6]=(conf.isUSB==1);   // USB
  btMainact[7]=(conf.cwMode>0);   // CW
  btMainact[8]=(conf.ritOn==1);     // RIT
  btMainact[9]=(conf.splitOn==1);   // Split
  btMainact[10]=0;  // Page Dorwn
  btMainact[11]=0;  // 
  btMainact[12]=0;  // Cal
  btMainact[13]=0;  // Set
  btMainact[14]=0;  // Page Up
}

long getNumberTFT()
{
  displayKey();
  char auxC[20]="";
  uint16_t x, y;
  while (true) 
    {
    if (tft.getTouch(&x, &y))
      {
      if (tft.getRotation()==1) { y=tft.height()-y;}
      else if (tft.getRotation()==3) { x=tft.width()-x; y=tft.height()-y;}
      for (byte i=0; i<10; i++)    // check KeyN buttons
        {
        if (btKey[i].contains(x,y)) 
          {
          strcat(auxC, itoa(i,buff,10));
          tft.setTextSize(4); tft.setTextColor(TFT_WHITE, TFT_BLACK);
          tft.drawString(auxC,0,0);
          delay(100);
          }
        }
      for (byte i=0; i<3; i++)    // check YN buttons
        {
        if (btYN[i].contains(x,y)) 
          {
          delay(100);
          inEntN=false;
          if (i==0) { return atol(auxC);   }   // OK
          else if (i==1) { return -1; }     // Cancel
          }
        }
      }
    }
}

int getCharTFT()
{
  displayKey();
  uint16_t x, y;
  strcpy(auxtft,"");
  while (true) 
    {
    if (tft.getTouch(&x, &y))
      {
      if (tft.getRotation()==1) { y=tft.height()-y;}
      else if (tft.getRotation()==3) { x=tft.width()-x; y=tft.height()-y;}
      for (byte i=0; i<50; i++)    // check KeyA buttons
        {
        if (btKey[i].contains(x,y)) 
          {
          strcat(auxtft, btKeytext[i]);
          tft.setTextSize(4); tft.setTextColor(TFT_WHITE, TFT_BLACK);
          tft.drawString(auxtft,0,0);
          delay(100);
          }
        }
      for (byte i=0; i<3; i++)    // check YN buttons
        {
        if (btYN[i].contains(x,y)) 
          {
          delay(100);
          inEntN=false;
          if (i==0) { return strlen(auxtft);   }        // OK
          else if (i==1) { return -1; }     // Cancel
          else if (i==2)                 // Backspace
            { 
            if (strlen(auxtft)>0) auxtft[strlen(auxtft)-1]=0; 
            tft.drawString("                    ",0,0);  
            tft.drawString(auxtft,0,0); }     
          }
        }
      }
    }
}

void handletfttouch()
{
  uint16_t x, y;
  if (tft.getTouch(&x, &y))
    {
    if (tft.getRotation()==1) { y=tft.height()-y;}
    else if (tft.getRotation()==3) { x=tft.width()-x; y=tft.height()-y;}
    if (tftpage==0)
      {
      for (byte i=0; i<5; i++)    // check flot buttons
        { 
        if (btFlot[i].contains(x,y)) 
          {
          if (i==0) 
            {
            long auxL=getNumberTFT();  // retorna con OK o Cancel
            if (auxL!=-1) { setFrequency(auxL); }
            }
          updateDisplay(true,false);
          }
        }
      for (byte i=0; i<9; i++)    // check step buttons
        { if (btFreq[i].contains(x,y)) { conf.tuneStepIndex=i; displayFreq(); } }
      for (byte i=0; i<10;i++)    // check buttons function
        {
        if (btMain[i].contains(x,y)) 
          {
          if (i==0)  { stopTx(); }   // RX
          else if (i==1)  { txTFT=true; startTx(TX_SSB,1); }    // TX
          else if (i==2)    // vfo
            {
            conf.vfoActive=conf.vfoActive==VFO_A?VFO_B:VFO_A;
            btMainact[2]=(conf.vfoActive==VFO_B);
            strcpy(btMaintext[2],conf.vfoActive==VFO_A?"A/B":"B/A");
            if (conf.vfoActive==VFO_A) conf.frequency=conf.frequencyA;
            else conf.frequency=conf.frequencyB;
            }
          else if (i==3) { setNextHamBandFreq(conf.frequency, -1); } //Prior Band
          else if (i==4) { setNextHamBandFreq(conf.frequency, 1); }  //Next Band
          else if (i==5)    // LSB
            {
            conf.isUSB=0; btMainact[5]=1; btMainact[6]=0;
            setFrequency(conf.frequency);
            }
          else if (i==6)    // USB
            {
            conf.isUSB=1; btMainact[5]=0; btMainact[6]=1;
            setFrequency(conf.frequency);
            }
          else if (i==7)    // CW
            {
            if (conf.cwMode==0) conf.cwMode=1; else conf.cwMode=0;  btMainact[7]=(conf.cwMode>0);
            setFrequency(conf.frequency);
            }
          else if (i==8)    // RIT
            {
            if (conf.ritOn==1) ritDisable(); else ritEnable(conf.frequency);
            btMainact[8]=(conf.ritOn==1); 
            }
          else if (i==9)    // SPL
            {
            if (conf.splitOn==0) 
              {
              conf.splitOn=1;
              btMainact[i]=true;
              ritDisable();
              }
            else 
              {
              conf.splitOn=0;  
              btMainact[i]=false;
              }
            }
          if (i!=1) saveconf();
          updateDisplay(true,false);
          }
        }
      }
    else if (tftpage==1)    // Setup page
      {
      for (byte i=0; i<5;i++)
        {
         if (btSet[i].contains(x,y)) 
           {
           if (i==0) { conf.lang=conf.lang==0?1:0; }   // language
           else if (i==1)                              // CallSign
             { 
             int auxI=getCharTFT(); 
             if (auxI !=-1) { strcpy(conf.CallSign,auxtft);  }
             }   
           else if (i==2) {  }                         // Latitude
           else if (i==3) {  }                         // Longitude
           else if (i==4) {  }                         // 
           saveconf(); updateDisplay(true,false);
           }
        }
      }
    else if (tftpage==2)    // WiFi page
      {
      for (byte i=0; i<5;i++)
        {
        if (btNet[i].contains(x,y)) 
          {
          if (i==0)      { conf.autoWiFi=conf.autoWiFi==0?1:0;  }   // AutoConnect
          else if (i==1) {  }                                       // SSID
          else if (i==2) {  }                                       // Password
          else if (i==3)                                            // WiFi Mode
            { if (conf.wifimode<2) conf.wifimode++; else conf.wifimode=0; }   
          else if (i==4) { conf.staticIP=conf.staticIP==0?1:0; }                                         //  Static IP
          saveconf();  updateDisplay(true,false);
          }
        }
      }
    else if (tftpage==3)    // Calibration page
      {
      for (byte i=0; i<5;i++)
        {
         if (btCal[i].contains(x,y)) 
           {
           if (i==0)      { setupFreq(); }   // CAL
           else if (i==1) { setupBFO();  }   // BFO
           else if (i==2) {  }      // 
           else if (i==3) {  }      // 
           else if (i==4) { ESP.restart(); } // Reset
           updateDisplay(true,false);
           }
        }
      }
    if ((!inEntN) && (!inEntA))
      {
      for (byte i=0; i<5;i++)     // navigation buttons
        {
         if (btNav[i].contains(x,y)) 
           {
           if (i==0)  { tftpage=tftpage>0?tftpage-1:3;    }
           else if (i==1) {     }
           else if (i==2) {     }
           else if (i==3) {     }
           else if (i==4) { tftpage=tftpage<3?tftpage+1:0; }
           updateDisplay(true,false);
           }
        }
      for (byte i=0; i<5;i++)   // buttons status line
        {
         if (btSta[i].contains(x,y)) 
           {
           if (i==0)  
             { if (WiFi.isConnected()) WiFi.disconnect();  
               else
                 {
                 initWiFi();
                 initFTP();
                 initHTML();
                 initTime();
                 initWebserver();
                 checkMyIP();
                 }
             }
           else if (i==1) {     }
           else if (i==2) {     }
           else if (i==3) {     }
           else if (i==4) {     }
           updateDisplay(true,false);
           }
        }
      }
    } 
}

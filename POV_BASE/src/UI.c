/*******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2015 Evgeny Dolgalev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "UI.h"

#include "stm32f0xx_rtc.h"

#include "FM.h"
#include "debug.h"
#include "sound.h"
#include "string.h"
#include "timework.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    FALSE = 0,
    TRUE = !FALSE
} bool;

/* Private define ------------------------------------------------------------*/
#define BitSet(p, m) ((p) |= (1 << (m)))
#define BitReset(p, m) ((p) &= ~(1 << (m)))
#define BitFlip(p, m) ((p) ^= (m))
#define BitWrite(c, p, m) ((c) ? BitSet(p, m) : BitReset(p, m))
#define BitIsSet(reg, bit) (((reg) & (1 << (bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1 << (bit))) == 0)

/* Private macro -------------------------------------------------------------*/
#define IsInMENU BitIsSet(currMenuID, 7)

/* Private variables ---------------------------------------------------------*/
/* Display screens */
//Name			Next			Previous, 		Parent, 	Child      	ID	Text
M_M(MAnalogClock, MDigitalClock, MDAClock, NULL_MENU, NULL_MENU, 0, "$>AClk");
M_M(MDigitalClock, MDAClock, MAnalogClock, NULL_MENU, NULL_MENU, 32, "$>DClk");
M_M(MDAClock, MAnalogClock, MDigitalClock, NULL_MENU, NULL_MENU, 64, "$>DAClk");
M_M(MMetronome, NULL_MENU, NULL_MENU, NULL_MENU, NULL_MENU, 96, "$>Metr");

/* Main menus */
//Name			//Next		//Previous, //Parent, 	//Child 	//ID	Text
M_M(MMENU, NULL_MENU, NULL_MENU, NULL_MENU, SetClock, 128, "#>MENU");

//Name			//Next		//Previous, //Parent, 	//Child 	 		//Text
M_M(SetClock, SetDate, NULL_MENU, MMENU, Set_Hour, 144, "Clock");
M_M(SetDate, SetVol, SetClock, MMENU, Set_Date, 160, "Date");
M_M(SetVol, SetTone, SetDate, MMENU, Set_VClick, 176, "Volume");
M_M(SetTone, SetMetr, SetVol, MMENU, Set_TClick, 192, "Tone");
M_M(SetMetr, SetColors, SetTone, MMENU, Set_4Strk, 208, "Metronome");
M_M(SetColors, SetWork, SetMetr, MMENU, Set_ColArr, 224, "Colors");
M_M(SetWork, NULL_MENU, SetColors, MMENU, Set_OnOff, 240, "Auto ON/OFF");

//Name			//Next		//Previous, //Parent, 	//Child 	//ID	//Text
M_M(Set_Hour, Set_Min, NULL_MENU, SetClock, _Set_Hour, 145, "Hr");
M_M(Set_Min, Set_Sec, Set_Hour, SetClock, _Set_Min, 146, "Min");
M_M(Set_Sec, Set_Corr, Set_Min, SetClock, _Set_Sec, 147, "Sec");
M_M(Set_Corr, NULL_MENU, Set_Sec, SetClock, _Set_Corr, 148, "Corr"); //..151

M_M(Set_Date, Set_Mon, NULL_MENU, SetDate, _Set_Date, 161, "Date");
M_M(Set_Mon, Set_Year, Set_Date, SetDate, _Set_Mon, 162, "Mon");
M_M(Set_Year, NULL_MENU, Set_Mon, SetDate, _Set_Year, 163, "Year"); //..167

M_M(Set_VClick, Set_VMetr, NULL_MENU, SetVol, _Set_VClick, 177, "Click");
M_M(Set_VMetr, Set_VHrStr, Set_VClick, SetVol, _Set_VMetr, 178, "Metr");
M_M(Set_VHrStr, NULL_MENU, Set_VMetr, SetVol, _Set_VHrStr, 179, "HrStrk"); //..183

M_M(Set_TClick, Set_TMtr1, NULL_MENU, SetTone, _Set_TClick, 193, "Click");
M_M(Set_TMtr1, Set_TMtr2, Set_TClick, SetTone, _Set_TMtr1, 194, "Metr1");
M_M(Set_TMtr2, Set_THrStr, Set_TMtr1, SetTone, _Set_TMtr2, 195, "Metr2");
M_M(Set_THrStr, NULL_MENU, Set_TMtr2, SetTone, _Set_THrStr, 196, "HrStrk"); //..199

M_M(Set_4Strk, NULL_MENU, NULL_MENU, SetMetr, _Set_4Strk, 209, "4thStike"); //..215

M_M(Set_ColArr, Set_ColMrk, NULL_MENU, SetColors, _Set_ColArr, 225, "Arrow");
M_M(Set_ColMrk, Set_ColTim, Set_ColArr, SetColors, _Set_ColMrk, 226, "Marks");
M_M(Set_ColTim, Set_ColDat, Set_ColMrk, SetColors, _Set_ColTim, 227, "Time");
M_M(Set_ColDat, Set_ColPend, Set_ColTim, SetColors, _Set_ColDat, 228, "Date");
M_M(Set_ColPend, NULL_MENU, Set_ColDat, SetColors, _Set_ColPend, 229, "Pendulum"); //..231

M_M(Set_OnOff, Set_OnTim, NULL_MENU, SetWork, _Set_OnOff, 241, "On:1/Off:0");
M_M(Set_OnTim, Set_OffTim, Set_OnOff, SetWork, _Set_OnTim, 242, "OnTime");
M_M(Set_OffTim, NULL_MENU, Set_OnTim, SetWork, _Set_OffTim, 243, "OffTime"); //!244,!245,!246,!247

/* MENU ASSIGNMENT */
//Name			//Next		//Previous, //Parent, 	//Child 		  //ID	//Text
M_M(_Set_Hour, NULL_MENU, NULL_MENU, Set_Hour, NULL_MENU, 145 | (1 << 3), ": #");
M_M(_Set_Min, NULL_MENU, NULL_MENU, Set_Min, NULL_MENU, 146 | (1 << 3), ": #");
M_M(_Set_Sec, NULL_MENU, NULL_MENU, Set_Sec, NULL_MENU, 147 | (1 << 3), ": #");
M_M(_Set_Corr, NULL_MENU, NULL_MENU, Set_Corr, NULL_MENU, 148 | (1 << 3), ": #");

M_M(_Set_Date, NULL_MENU, NULL_MENU, Set_Date, NULL_MENU, 161 | (1 << 3), ": #");
M_M(_Set_Mon, NULL_MENU, NULL_MENU, Set_Mon, NULL_MENU, 162 | (1 << 3), ": #");
M_M(_Set_Year, NULL_MENU, NULL_MENU, Set_Year, NULL_MENU, 163 | (1 << 3), ": #");

M_M(_Set_VClick, NULL_MENU, NULL_MENU, Set_VClick, NULL_MENU, 177 | (1 << 3), ": #");
M_M(_Set_VMetr, NULL_MENU, NULL_MENU, Set_VMetr, NULL_MENU, 178 | (1 << 3), ": #");
M_M(_Set_VHrStr, NULL_MENU, NULL_MENU, Set_VHrStr, NULL_MENU, 179 | (1 << 3), ": #");

M_M(_Set_TClick, NULL_MENU, NULL_MENU, Set_TClick, NULL_MENU, 193 | (1 << 3), ": #");
M_M(_Set_TMtr1, NULL_MENU, NULL_MENU, Set_TMtr1, NULL_MENU, 194 | (1 << 3), ": #");
M_M(_Set_TMtr2, NULL_MENU, NULL_MENU, Set_TMtr2, NULL_MENU, 195 | (1 << 3), ": #");
M_M(_Set_THrStr, NULL_MENU, NULL_MENU, Set_THrStr, NULL_MENU, 196 | (1 << 3), ": #");

M_M(_Set_4Strk, NULL_MENU, NULL_MENU, Set_4Strk, NULL_MENU, 209 | (1 << 3), ": #");

M_M(_Set_ColArr, NULL_MENU, NULL_MENU, Set_ColArr, NULL_MENU, 225 | (1 << 3), ": #");
M_M(_Set_ColMrk, NULL_MENU, NULL_MENU, Set_ColMrk, NULL_MENU, 226 | (1 << 3), ": #");
M_M(_Set_ColTim, NULL_MENU, NULL_MENU, Set_ColTim, NULL_MENU, 227 | (1 << 3), ": #");
M_M(_Set_ColDat, NULL_MENU, NULL_MENU, Set_ColDat, NULL_MENU, 228 | (1 << 3), ": #");
M_M(_Set_ColPend, NULL_MENU, NULL_MENU, Set_ColPend, NULL_MENU, 229 | (1 << 3), ": #");

M_M(_Set_OnOff, NULL_MENU, NULL_MENU, Set_OnOff, NULL_MENU, 241 | (1 << 3), ": #");
M_M(_Set_OnTim, NULL_MENU, NULL_MENU, Set_OnTim, NULL_MENU, 242 | (1 << 3), ": #");
M_M(_Set_OffTim, NULL_MENU, NULL_MENU, Set_OffTim, NULL_MENU, 243 | (1 << 3), ": #");

MenuItem Null_Menu = {(void *)0, (void *)0, (void *)0, (void *)0, 0, {0x00}};
MenuItem *CurrMenuItem;
MenuItem *CurrDisplayMenuItem;

uint8_t currMenuID = 0;

RTC_TimeTypeDef TimeTemp;
RTC_DateTypeDef DateTemp;

enum
{
    GREEN,
    RED,
    BLUE,
    YELLOW,
    MAGENTA,
    CYAN,
    WHITE
};

SettingsArray sett = {
    .prevMode = 1,

    .toneM1 = 40,
    .toneM2 = 40,
    .toneHrStrk = 40,
    .toneClick = 45,

    .volClick = 2,
    .volMetr = 2,
    .volHrStrk = 2,

    .colDate = CYAN,
    .colTime = BLUE,
    .colArrow = GREEN,
    .colHMrk = WHITE,
    .colPend = BLUE,
};

uint16_t metrValue = 0;
uint16_t metrBPM = 128;
volatile uint16_t metrCounter = 0;
volatile bool metrFlag = 0;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void delay_ms(volatile uint32_t);

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : UIInit
* Description    : Initialize User Interface
*******************************************************************************/
void UIInit()
{
    DebugSendNum(RTC_ReadBackupRegister(0));
    DebugSendNum(RTC_ReadBackupRegister(1));
    DebugSendNum(RTC_ReadBackupRegister(2));

    if(RTC_ReadBackupRegister(0) != 0 ||
       RTC_ReadBackupRegister(1) != 0 ||
       RTC_ReadBackupRegister(2) != 0)
    {
        /* Backup to Settings structure */
        for(uint8_t i = 0; i < 3; i++)
            BkpToSett(i);
    }

    /* Check correct metronome BPM */
    if(sett.metrBPM < 40 || sett.metrBPM > 160)
    {
        sett.metrBPM = 120;
    }

    for(uint8_t i = 0; i < 3; i++)
        SettingsToBkp(i);

    /* Previous display mode */
    switch(sett.prevMode)
    {
    case 0:
        MenuChange((MenuItem *)&MAnalogClock);
        break;
    case 1:
        MenuChange((MenuItem *)&MDigitalClock);
        break;
    default: // 2, 3
        MenuChange((MenuItem *)&MDAClock);
        break;
    }

    RTCCalibrate();
}

/*******************************************************************************
* Function Name  : UIDispatcher
* Description    : Handle new button event
* Input			 : button number
*******************************************************************************/
void UIDispatcher(uint8_t button)
{
    if(IsInMENU)
    {
        switch(button)
        { /* In menu */
        case B_LEFT:
            if(PARENT == &NULL_MENU)
            {
                /* Send TimeDate */
                GetTimeDate(&TimeTemp, &DateTemp);
                FMSendData(TimeDateToCounter(&TimeTemp, &DateTemp), 32);
                /* Calibrate RTC */
                RTCCalibrate();
                /* Send settings*/
                SendSettArray();

                MenuChange(CurrDisplayMenuItem);
            }
            else
                MenuChange(PARENT);
            break;

        case B_RIGHT:
            MenuChange(CHILD);
            break;

        case B_UP:
            MenuChange(NEXT);

            if(CHILD == &NULL_MENU) SetUp(); //menu assignment mode
            break;

        case B_DOWN:
            MenuChange(PREVIOUS);

            if(CHILD == &NULL_MENU) SetDown(); //menu assignment mode
            break;

        case B_ONOFF:
            break;

        case B_AVTV:
            break;

        case B_MUTE:
            /* Send TimeDate */
            GetTimeDate(&TimeTemp, &DateTemp);
            FMSendData(TimeDateToCounter(&TimeTemp, &DateTemp), 32);
            /* Calibrate RTC */
            RTCCalibrate();
            /* Send settings*/
            SendSettArray();

            MenuChange(CurrDisplayMenuItem);
            break;
        }
    }
    else
    { /* Not in menu */
        switch(button)
        {
        case B_LEFT:
            if(CurrMenuItem == &MMetronome) MetrDown(1);
            break;

        case B_RIGHT:
            if(CurrMenuItem == &MMetronome) MetrUp(1);
            break;

        case B_UP:
            MenuChange(NEXT);
            if(CurrMenuItem == &MMetronome) MetrUp(10);
            break;

        case B_DOWN:
            MenuChange(PREVIOUS);
            if(CurrMenuItem == &MMetronome) MetrDown(10);
            break;

        case B_ONOFF:
            MenuChange(CurrDisplayMenuItem);
            break;

        case B_AVTV:
            if(CurrMenuItem != &MMetronome)
            {
                MenuChange(&MMetronome);
                MetrEnter();
            }
            else
                MenuChange(CurrDisplayMenuItem);
            break;

        case B_MUTE:
            MenuChange(&MMENU);
            break;
        }
    }
}

/*******************************************************************************
* Function Name  : MenuChange
* Description    : Change current menu
* Input			 : pointer to new menu
*******************************************************************************/
void MenuChange(MenuItem *NewMenu)
{
    if(NewMenu == &NULL_MENU)
    {
        ////////DebugSendString("!!!WRONG!!!");
        return;
    }

    CurrMenuItem = NewMenu;
    currMenuID = CurrMenuItem->number & ~(1 << 3); //3rd bit indicating "assignment mode"

    //Debug>ModeName__
    char f[80] = {'\0'}; // f[0] = '\0';
    MenuItem *TempMenu = CurrMenuItem;
    do
    {
        if(TempMenu->Child != &NULL_MENU) strcat_(f, ">");
        strcatWRev(f, (char *)TempMenu->Text);
        TempMenu = TempMenu->Parent;
    } while(TempMenu != &NULL_MENU);
    reverse(f);
    DebugSendString(f);
    //__Debug>ModeName

    FMSendData(CurrMenuItem->number, 8);

    if(BitIsSet(CurrMenuItem->number, 3)) SendSett();

    switch(currMenuID)
    {
    case 0:
        sett.prevMode = 0;
        SettingsToBkp(0);
        CurrDisplayMenuItem = &MAnalogClock;
        break;
    case 32:
        sett.prevMode = 1;
        SettingsToBkp(0);
        CurrDisplayMenuItem = &MDigitalClock;
        break;
    case 64:
        sett.prevMode = 2;
        SettingsToBkp(0);
        CurrDisplayMenuItem = &MDAClock;
        break;
    }

    metrFlag = FALSE;

    if(CurrMenuItem == &MMetronome) MetrEnter();
}

/*******************************************************************************
* Function Name  : SettingsToBkp
* Description    : Write settings variables to debug registers
*******************************************************************************/
void SettingsToBkp(uint8_t bkpNum)
{
    uint32_t data = 0;

    switch(bkpNum)
    {
    case 0:
        data = (sett.prevMode) |                      //2
               (sett.metr4Strk << (2)) |              //1
               (sett.toneM1 << (2 + 1)) |             //6
               (sett.toneM2 << (2 + 1 + 6)) |         //6
               (sett.toneHrStrk << (2 + 1 + 6 + 6)) | //6
               (sett.corr << (2 + 1 + 6 + 6 + 6));    //10
        break;

    case 1:
        data = (sett.toneClick) |                 //6
               (sett.volClick << (6)) |           //3
               (sett.volMetr << (6 + 3)) |        //3
               (sett.volHrStrk << (6 + 3 + 3)) |  //3
               (sett.metrBPM << (6 + 3 + 3 + 3)); //7
        break;

    case 2:
        data = (sett.colDate) |                              //3
               (sett.colTime << (3)) |                       //3
               (sett.colArrow << (3 + 3)) |                  //3
               (sett.colHMrk << (3 + 3 + 3)) |               //3
               (sett.colPend << (3 + 3 + 3 + 3)) |           //3
               (sett.OnTime << (3 + 3 + 3 + 3 + 3)) |        //5
               (sett.OffTime << (3 + 3 + 3 + 3 + 3 + 5)) |   //5
               (sett.OnTime << (3 + 3 + 3 + 3 + 3 + 5 + 5)); //1
        break;

    default:
        data = 0;
        break;
    }

    RTC_WriteBackupRegister((uint32_t)bkpNum, data);
}

/*******************************************************************************
* Function Name  : BkpToSett
* Description    : Read Backup registers and write it to settings variables
*******************************************************************************/
void BkpToSett(uint8_t bkpNum)
{
#define BIT_MASK(x) ((1 << (x)) - 1)

    uint32_t data = RTC_ReadBackupRegister((uint32_t)bkpNum);

    switch(bkpNum)
    {
    case 0:
        sett.prevMode = (data)&BIT_MASK(2);
        sett.metr4Strk = (data >> (2)) & BIT_MASK(1);
        sett.toneM1 = (data >> (2 + 1)) & BIT_MASK(6);
        sett.toneM2 = (data >> (2 + 1 + 6)) & BIT_MASK(6);
        sett.toneHrStrk = (data >> (2 + 1 + 6 + 6)) & BIT_MASK(6);
        sett.corr = (data >> (2 + 1 + 6 + 6 + 6)) & BIT_MASK(10);
        break;

    case 1:
        sett.toneClick = (data)&BIT_MASK(6);
        sett.volClick = (data >> (6)) & BIT_MASK(3);
        sett.volMetr = (data >> (6 + 3)) & BIT_MASK(3);
        sett.volHrStrk = (data >> (6 + 3 + 3)) & BIT_MASK(3);
        sett.metrBPM = (data >> (6 + 3 + 3 + 3)) & BIT_MASK(7);
        break;

    case 2:
        sett.colDate = (data)&BIT_MASK(3);
        sett.colTime = (data >> (3)) & BIT_MASK(3);
        sett.colArrow = (data >> (3 + 3)) & BIT_MASK(3);
        sett.colHMrk = (data >> (3 + 3 + 3)) & BIT_MASK(3);
        sett.colPend = (data >> (3 + 3 + 3 + 3)) & BIT_MASK(3);
        sett.OnTime = (data >> (3 + 3 + 3 + 3 + 3)) & BIT_MASK(5);
        sett.OffTime = (data >> (3 + 3 + 3 + 3 + 3 + 5)) & BIT_MASK(5);
        sett.WorkOnOff = (data >> (3 + 3 + 3 + 3 + 3 + 5 + 5)) & BIT_MASK(1);
        break;

    default:
        break;
    }
}

/*******************************************************************************
* Function Name  : SetUp
* Description    : Settings: increase parameter
*******************************************************************************/
void SetUp()
{
    GetTimeDate(&TimeTemp, &DateTemp);

    switch(currMenuID)
    {
        /* CLOCK */
    case 145: //#>Menu>Clock->Hour
        if(++(TimeTemp.RTC_Hours) >= 24) TimeTemp.RTC_Hours = 0;
        RTC_SetTime(RTC_Format_BIN, &TimeTemp);
        break;

    case 146: //#>Menu>Clock->Minute
        if(++(TimeTemp.RTC_Minutes) >= 60) TimeTemp.RTC_Minutes = 0;
        RTC_SetTime(RTC_Format_BIN, &TimeTemp);
        break;

    case 147: //#>Menu>Clock->Seconds
        TimeTemp.RTC_Seconds = 0;
        RTC_SetTime(RTC_Format_BIN, &TimeTemp);
        break;

    case 148: //#>Menu>Clock->Correction
        if(sett.corr < 511) sett.corr++;
        break;

        /* DATE */
    case 161: //#>Menu>Date>Date
        if(++DateTemp.RTC_Date > 31) DateTemp.RTC_Date = 1;
        RTC_SetDate(RTC_Format_BIN, &DateTemp);
        break;

    case 162: //#>Menu>Date>Month
        if(++DateTemp.RTC_Month > 12) DateTemp.RTC_Month = 1;
        RTC_SetDate(RTC_Format_BIN, &DateTemp);
        break;

    case 163: //#>Menu>Date>Year
        if(++DateTemp.RTC_Year >= 80) DateTemp.RTC_Year = 80;
        RTC_SetDate(RTC_Format_BIN, &DateTemp);
        break;

        /*  VOLUME */
    case 177: //#>Menu>Volume->Click
        if(sett.volClick < 7) sett.volClick++;
        break;

    case 178: //#>Menu>Volume->Metr
        if(sett.volMetr < 7) sett.volMetr++;
        break;

    case 179: //#>Menu>Volume->HrStrk
        if(sett.volHrStrk < 7) sett.volHrStrk++;
        break;

        /* TONE */
    case 193: //#>Menu>Tone->Click
        if(sett.toneClick < 63) sett.toneClick++;
        break;

    case 194: //#>Menu>Tone->Metr1
        if(sett.toneM1 < 63) sett.toneM1++;
        break;

    case 195: //#>Menu>Tone->Metr2
        if(sett.toneM2 < 63) sett.toneM2++;
        break;

    case 196: //#>Menu>Tone->HrStrk
        if(sett.toneHrStrk < 63) sett.toneHrStrk++;
        break;

        /* METRONOME */
    case 209: //#>Menu>Metronome->4thStrike
        if(sett.metr4Strk != 1) sett.metr4Strk = 1;
        break;

        /* COLORS */
    case 225: //#>Menu>Colors->Arrow
        if(sett.colArrow < 6) sett.colArrow++;
        break;

    case 226: //#>Menu>Colors->Marks
        if(sett.colHMrk < 6) sett.colHMrk++;
        break;

    case 227: //#>Menu>Colors->Time
        if(sett.colTime < 6) sett.colTime++;
        break;

    case 228: //#>Menu>Colors->Date
        if(sett.colDate < 6) sett.colDate++;
        break;

    case 229: //#>Menu>Colors->Pendulum
        if(sett.colPend < 6) sett.colPend++;
        break;

        /* AUTO ON/OFF */
    case 241:
        if(sett.WorkOnOff == 0) sett.WorkOnOff = 1;
        break;

    case 242:
        if(sett.OnTime < 23) sett.OnTime++;
        break;

    case 243:
        if(sett.OffTime < 23) sett.OffTime++;
        break;
    }

    SendSett();
}

/*******************************************************************************
* Function Name  : SetDown
* Description    : Settings: decrease parameter
*******************************************************************************/
void SetDown()
{
    GetTimeDate(&TimeTemp, &DateTemp);

    switch(currMenuID)
    {
        /* CLOCK */
    case 145: //#>Menu>Clock->Hour
        if(TimeTemp.RTC_Hours > 0)
            TimeTemp.RTC_Hours--;
        else
            TimeTemp.RTC_Hours = 23;
        RTC_SetTime(RTC_Format_BIN, &TimeTemp);
        break;

    case 146: //#>Menu>Clock->Minute
        if(TimeTemp.RTC_Minutes > 0)
            TimeTemp.RTC_Minutes--;
        else
            TimeTemp.RTC_Minutes = 59;
        RTC_SetTime(RTC_Format_BIN, &TimeTemp);
        break;

    case 147: //#>Menu>Clock->Seconds
        TimeTemp.RTC_Seconds = 0;
        RTC_SetTime(RTC_Format_BIN, &TimeTemp);
        break;

    case 148: //#>Menu>Clock->Correction
        if(sett.corr > -511) sett.corr--;
        break;

        /* DATE */
    case 161: //#>Menu>Date>Date
        if(DateTemp.RTC_Date > 1)
            DateTemp.RTC_Date--;
        else
            DateTemp.RTC_Date = 1;
        RTC_SetDate(RTC_Format_BIN, &DateTemp);
        break;

    case 162: //#>Menu>Date>Month
        if(DateTemp.RTC_Month > 1)
            DateTemp.RTC_Month--;
        else
            DateTemp.RTC_Month = 1;
        RTC_SetDate(RTC_Format_BIN, &DateTemp);
        break;

    case 163: //#>Menu>Date>Year
        if(DateTemp.RTC_Year > 0)
            DateTemp.RTC_Year--;
        else
            DateTemp.RTC_Year = 0;
        RTC_SetDate(RTC_Format_BIN, &DateTemp);
        break;

        /*  VOLUME */
    case 177: //#>Menu>Volume->Click
        if(sett.volClick > 0) sett.volClick--;
        break;

    case 178: //#>Menu>Volume->Metr
        if(sett.volMetr > 0) sett.volMetr--;
        break;

    case 179: //#>Menu>Volume->HrStrk
        if(sett.volHrStrk > 0) sett.volHrStrk--;
        break;

        /* TONE */
    case 193: //#>Menu>Tone->Click
        if(sett.toneClick > 0) sett.toneClick--;
        break;

    case 194: //#>Menu>Tone->Metr1
        if(sett.toneM1 > 0) sett.toneM1--;
        break;

    case 195: //#>Menu>Tone->Metr2
        if(sett.toneM2 > 03) sett.toneM2--;
        break;

    case 196: //#>Menu>Tone->HrStrk
        if(sett.toneHrStrk > 0) sett.toneHrStrk--;
        break;

        /* METRONOME */
    case 209: //#>Menu>Metronome->4thStrike
        if(sett.metr4Strk != 0) sett.metr4Strk = 0;
        break;

        /* COLORS */
    case 225: //#>Menu>Colors->Arrow
        if(sett.colArrow > 0) sett.colArrow--;
        break;

    case 226: //#>Menu>Colors->Marks
        if(sett.colHMrk > 0) sett.colHMrk--;
        break;

    case 227: //#>Menu>Colors->Time
        if(sett.colTime > 0) sett.colTime--;
        break;

    case 228: //#>Menu>Colors->Date
        if(sett.colDate > 0) sett.colDate--;
        break;

    case 229: //#>Menu>Colors->Pendulum
        if(sett.colPend > 0) sett.colPend--;
        break;

        /* AUTO ON/OFF */
    case 241:
        if(sett.WorkOnOff != 0) sett.WorkOnOff = 0;
        break;

    case 242:
        if(sett.OnTime > 0) sett.OnTime--;
        break;

    case 243:
        if(sett.OffTime > 0) sett.OffTime--;
        break;
    }

    SendSett();
}

/*******************************************************************************
* Function Name  : SendSett
* Description    : Send current settings to the Rotor
*******************************************************************************/
void SendSett()
{
    GetTimeDate(&TimeTemp, &DateTemp);

    switch(currMenuID)
    {
        /* CLOCK */
    case 145: //#>Menu>Clock->Hour
        FMSendData(TimeTemp.RTC_Hours, 16);
        break;

    case 146: //#>Menu>Clock->Minute
        FMSendData(TimeTemp.RTC_Minutes, 16);
        break;

    case 147: //#>Menu>Clock->Seconds
        FMSendData(TimeTemp.RTC_Seconds, 16);
        break;

    case 148: //#>Menu>Clock->Correction
        SettingsToBkp(0);
        FMSendData(sett.corr, 16);
        break;

        /* DATE */
    case 161: //#>Menu>Date>Date
        FMSendData(DateTemp.RTC_Date, 16);
        break;

    case 162: //#>Menu>Date>Month
        FMSendData(DateTemp.RTC_Month, 16);
        break;

    case 163: //#>Menu>Date>Year
        FMSendData(DateTemp.RTC_Year, 16);
        break;

        /*  VOLUME */
    case 177: //#>Menu>Volume->Click
        SettingsToBkp(1);
        FMSendData(sett.volClick, 16);
        break;

    case 178: //#>Menu>Volume->Metr
        SettingsToBkp(1);
        FMSendData(sett.volMetr, 16);
        break;

    case 179: //#>Menu>Volume->HrStrk
        SettingsToBkp(1);
        FMSendData(sett.volHrStrk, 16);
        break;

        /* TONE */
    case 193: //#>Menu>Tone->Click
        SettingsToBkp(1);
        FMSendData(sett.toneClick, 16);
        SoundPlayNote(0, WAVE_NULL, 100, 2);
        SoundPlayNote(sett.toneClick, WAVE_SIN, 100, 2);
        break;

    case 194: //#>Menu>Tone->Metr1
        SettingsToBkp(0);
        FMSendData(sett.toneM1, 16);
        SoundPlayNote(0, WAVE_NULL, 100, 2);
        SoundPlayNote(sett.toneM1, WAVE_SIN, 100, 2);
        break;

    case 195: //#>Menu>Tone->Metr2
        SettingsToBkp(0);
        FMSendData(sett.toneM2, 16);
        SoundPlayNote(0, WAVE_NULL, 100, 2);
        SoundPlayNote(sett.toneM2, WAVE_SIN, 100, 2);
        break;

    case 196: //#>Menu>Tone->HrStrk
        SettingsToBkp(0);
        FMSendData(sett.toneHrStrk, 16);
        SoundPlayNote(0, WAVE_NULL, 100, 2);
        SoundPlayNote(sett.toneHrStrk, WAVE_SIN, 100, 2);
        break;

        /* METRONOME */
    case 209: //#>Menu>Metronome->4thStrike
        SettingsToBkp(1);
        FMSendData(sett.metr4Strk, 16);
        break;

        /* COLORS */
    case 225: //#>Menu>Colors->Arrow
        SettingsToBkp(2);
        FMSendData(sett.colArrow, 16);
        break;

    case 226: //#>Menu>Colors->Marks
        SettingsToBkp(2);
        FMSendData(sett.colHMrk, 16);
        break;

    case 227: //#>Menu>Colors->Time
        SettingsToBkp(2);
        FMSendData(sett.colTime, 16);
        break;

    case 228: //#>Menu>Colors->Date
        SettingsToBkp(2);
        FMSendData(sett.colDate, 16);
        break;

    case 229: //#>Menu>Colors->Pendulum
        SettingsToBkp(2);
        FMSendData(sett.colPend, 16);
        break;

        /* AUTO ON/OFF */
    case 241:
        SettingsToBkp(2);
        FMSendData(sett.WorkOnOff, 16);
        break;

    case 242:
        SettingsToBkp(2);
        FMSendData(sett.OnTime, 16);
        break;

    case 243:
        SettingsToBkp(2);
        FMSendData(sett.OffTime, 16);
        break;
    }
}

/*******************************************************************************
* Function Name  : SendSettArray
* Description    : Send all settings to Rotor
*******************************************************************************/
void SendSettArray()
{
    /* Send SettingsArray */
    for(uint32_t i = 0; i < 3; i++)
    {
        FMSendData(253 + i, 8);
        FMSendData(RTC_ReadBackupRegister(i), 32);
    }
}

/*******************************************************************************
* Function Name  : SendPWM
* Description    : Send PWM brightness to Rotor
*******************************************************************************/
void SendPWM(uint16_t value)
{
    FMSendData(252, 8);
    FMSendData(value, 32);
}

/*******************************************************************************
* Function Name  : RTCCalibrate
* Description    : Write RTC Calibration registers
*******************************************************************************/
void RTCCalibrate()
{
    /* RTC calibration */
    uint32_t CalP = (BitIsReset(sett.corr, 9) && sett.corr != 0) ? RTC_SmoothCalibPlusPulses_Set : RTC_SmoothCalibPlusPulses_Reset;
    uint16_t CalM = CalP ? (512 - (sett.corr & 511)) : (-sett.corr & 511);
    RTC_SmoothCalibConfig(RTC_SmoothCalibPeriod_32sec, CalP, CalM);
    DebugSendNumWDesc("CalP =", CalP);
    DebugSendNumWDesc("CalM =", CalM);
}

/*******************************************************************************
* Function Name  : MetrEnter
* Description    : Initialize metronome settings
*******************************************************************************/
void MetrEnter()
{
    metrBPM = sett.metrBPM;
    metrValue = 30000 / metrBPM;
    FMSendData(metrValue, 16);
    MetronomeSound();
}

/*******************************************************************************
* Function Name  : MetrUp
* Description    : Increase metronome BPM
*******************************************************************************/
void MetrUp(uint8_t amount)
{
    if(metrBPM <= +(160 - amount)) metrBPM += amount;

    /* count period */
    metrValue = 30000 / metrBPM;

    /* send period value to rotor */
    FMSendData(metrValue, 16);

    /* write BPM to backup register */
    sett.metrBPM = metrBPM;
    SettingsToBkp(1);

    MetronomeSound();
}

/*******************************************************************************
* Function Name  : MetrDown
* Description    : Decrease metronome BPM
*******************************************************************************/
void MetrDown(uint8_t amount)
{
    if(metrBPM >= (40 + amount)) metrBPM -= amount;

    /* count period */
    metrValue = 30000 / metrBPM;

    /* send period value to rotor */
    FMSendData(metrValue, 16);

    /* write BPM to backup register */
    sett.metrBPM = metrBPM;
    SettingsToBkp(1);

    MetronomeSound();
}

/*******************************************************************************
* Function Name  : MetronomeSound
* Description    : Process metronome program
*******************************************************************************/
void MetronomeSound()
{
    if(metrValue == 0) return;
    metrCounter = metrValue * 2;
    metrFlag = ENABLE;

    /* 4 Strike */
    static uint8_t var4 = 3;

    if(++var4 >= 4 && sett.metr4Strk)
    {
        var4 = 0;
        //metronome with no sound doesn't make sense
        SoundPlayNote(sett.toneM2, WAVE_TRIANGLE, 100, !sett.volMetr ? 1 : sett.volMetr);
    }
    else
        //metronome with no sound doesn't make sense
        SoundPlayNote(sett.toneM1, WAVE_TRIANGLE, 100, !sett.volMetr ? 1 : sett.volMetr);
}

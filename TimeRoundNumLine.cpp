//---------------------------------------------------------------------------
// Ishimoku indicator
//---------------------------------------------------------------------------

#include <windows.h>
#include <math.h>
#include "IndicatorInterfaceUnit.h"
#include "TechnicalFunctions.h"


// External variables
//PChar Currency = NULL;
TChartInfo ChartInfo;
int TimeDiff = 7;
int FontSizeArrow = 10;
PChar FontNameArrow = "Wingdings 3";
// External variables
int period_D=480;
int period_L=80;
int period_M=20;
int period_S=5;

int Shift = 0;
int MAtype = 0;
int ApplyToPrice = 0;
int VShift = 0;
int UpArrow = 0x70;
int DownArrow = 0x71;
int FontSize = 10;
int adjustTimeBar;
//PChar FontName = "Segoe UI Semibold";
PChar FontName = "Segoe UI";

TDateTime TimeNow;
TDateTime Time_0_prev;
SYSTEMTIME mytime;
TColor FontColor;
TColor LineColor;

int iCount;
// Buffers
//TIndexBuffer TenkanSen, KijunSen, ChinkouSpan, SenkouSpanA, SenkouSpanB;

char time_text[256];
char text[256];
char obj_name_tmp[256];
int i;
int digits;
double price;
int iModulo = 1;

int iNumRoundNum = 0;
int iGapBetweenVL = 90;
int iGapBetweenTime = 15;
int iLastX_VL;
int iLastX_Time;
TIndexBuffer SMA_D, SMA_L, SMA_M;
int iGap;
int iDigitsOverride;
double dNum_tmp;

//---------------------------------------------------------------------------
double PipsToPrice(double pips)
{
    price = 0;

    if (digits == 3 || digits == 5) {

        price = 10 * pips / pow(10,digits);
        //MyPoint=Point*10;
    }
    if (digits == 2 || digits == 4) {
        price = pips / pow(10, digits);
        //MyPoint=Point*10;
    }

    return(price);
}

//---------------------------------------------------------------------------
// Initialize indicator
//---------------------------------------------------------------------------
EXPORT void __stdcall Init()
{
  int index_temp;

  // define properties
  IndicatorShortName("TimeRoundNumLine");
  SetOutputWindow(ow_ChartWindow);

  // register options
  //AddSeparator("Common");

  RegOption("TimeDiff", ot_Integer, &TimeDiff);
  SetOptionRange("TimeDiff", -24, +24);
  TimeDiff = 7;

  RegOption("FontName", ot_PChar, &FontName);
  //SetOptionRange("Tenkan-sen period", 1, MaxInt);
  //FontName = "Segoe UI Semibold";
  FontName = "Segoe UI";

  RegOption("FontSize", ot_Integer, &FontSize);
  SetOptionRange("FontSize", 1, MaxInt);
  FontSize = 8;

  RegOption("FontColor", ot_Color, &FontColor);
  FontColor = clCadetBlue;

  RegOption("adjustTimeBar", ot_Integer, &adjustTimeBar);
  SetOptionRange("adjustTimeBar", -24, +24);
  adjustTimeBar = 0;

  RegOption("LineColor", ot_Color, &LineColor);
  LineColor = clSilver;

  RegOption("NumRoundNum", ot_Integer, &iNumRoundNum);
  SetOptionRange("NumRoundNum", 0, 15);
  iNumRoundNum = 15;

  RegOption("GapBetweenVL", ot_Integer, &iGapBetweenVL);
  SetOptionRange("GapBetweenVL", 0, 300);
  iGapBetweenVL = 90;

  RegOption("GapBetweenTime", ot_Integer, &iGapBetweenTime);
  SetOptionRange("GapBetweenTime", 0, 200);
  iGapBetweenTime = 15;

  RegOption("DigitsOverride", ot_Integer, &iDigitsOverride);
  SetOptionRange("DigitsOverride", 0, 6);
  iDigitsOverride = 0;

  digits = Digits();

  for (index_temp = ObjectsTotal() - 1; 0 <= index_temp; index_temp--) {
      //ObjectName(index_temp);
      strncpy_s(obj_name_tmp, ObjectName(index_temp), 10);
      if (!strncmp(obj_name_tmp, "TimeTxtScr", 10)) {
          ObjectDelete(ObjectName(index_temp));
      }
      else if (!strncmp(obj_name_tmp, "VTimeL", 6)) {
          ObjectDelete(ObjectName(index_temp));
      }
  }

  Time_0_prev = 0.0;
}
//---------------------------------------------------------------------------
EXPORT void __stdcall OnParamsChange()
{
    int index_temp;

    for (index_temp = ObjectsTotal() - 1; 0 <= index_temp; index_temp--) {
        //ObjectName(index_temp);
        strncpy_s(obj_name_tmp, ObjectName(index_temp), 10);
        if (!strncmp(obj_name_tmp, "TimeTxtScr", 10)) {
            ObjectDelete(ObjectName(index_temp));
        }
        else if (!strncmp(obj_name_tmp, "VTimeL", 6)) {
            ObjectDelete(ObjectName(index_temp));
        }
    }
}

//---------------------------------------------------------------------------
// Deinitialize indicator
//---------------------------------------------------------------------------
EXPORT void __stdcall Done()
{

    int index_temp;

    for (index_temp = ObjectsTotal() - 1; 0 <= index_temp; index_temp--) {
        //ObjectName(index_temp);
        strncpy_s(obj_name_tmp, ObjectName(index_temp), 10);
        if (!strncmp(obj_name_tmp, "TimeTxtScr", 10)) {
            ObjectDelete(ObjectName(index_temp));
        }
        else if (!strncmp(obj_name_tmp, "VTimeL", 6)) {
            ObjectDelete(ObjectName(index_temp));
        }
    }
}
//---------------------------------------------------------------------------
// Calculate requested bar
//---------------------------------------------------------------------------
EXPORT void __stdcall Calculate(int index)
{
    TDateTime time_at_index;
    int index_temp;
    int iLocalHour;
    //int power_num;
    int iNum_tmp;

    iModulo = 1;
    if (index == 0) {
        GetChartInfo(ChartInfo);
        for (index_temp = ObjectsTotal() - 1; 0 <= index_temp; index_temp--) {
            //ObjectName(index_temp);
            strncpy_s(obj_name_tmp, ObjectName(index_temp), 10);
            //if (!strncmp(ObjectName(index_temp), "TimeTxtScr", 10)) {
            //    ObjectDelete(ObjectName(index_temp));
            //}
            if (!strncmp(obj_name_tmp, "TimeTxtScr", 10)) {
                ObjectDelete(ObjectName(index_temp));
            }
            else if (!strncmp(obj_name_tmp, "VTimeL", 6)) {
                ObjectDelete(ObjectName(index_temp));
            }
        }

        if (5 == Timeframe()) {
            if (ChartInfo.BarWidth < 2) {
                iModulo = 8;
            }else if (ChartInfo.BarWidth < 4) {
                iModulo = 4;
            }
            else if (ChartInfo.BarWidth < 6) {
                iModulo = 2;
            }
        }
        else if (15 <= Timeframe() && Timeframe() < 30) {
            if (ChartInfo.BarWidth < 4) {
                iModulo = 16;
            }
            else if (ChartInfo.BarWidth < 6) {
                iModulo = 8;
            }
        }
        else if (30 <= Timeframe() && Timeframe() < 60) {
            if (ChartInfo.BarWidth < 4) {
                iModulo = 8;
            }
            else if (ChartInfo.BarWidth < 6) {
                iModulo = 4;
            }
        }
        else if (60 <= Timeframe() && Timeframe() < 240) {
            if (ChartInfo.BarWidth < 4) {
                iModulo = 16;
            }
            else if (ChartInfo.BarWidth < 6) {
                iModulo = 8;
            }
        }

        // if scalse is larger than 4 hours, such as 1 day, 1 week then, do not draw
        if (60*4 < Timeframe()) { return; }

        iLastX_VL = 0;
        iLastX_Time = 0;
        iCount = 0;
        iGap = 0;
        // debug iModulo = 1;
        for (index_temp = 0; index_temp < ChartInfo.FirstIndex; index_temp++) {
            time_at_index = Time(index_temp);
            VariantTimeToSystemTime(time_at_index, &mytime);
            if (0 < iModulo) {
                if (mytime.wMinute == 0) {
                    sprintf_s(text, "VTimeL%d-%d-%d-%d", mytime.wYear, mytime.wMonth, mytime.wDay, mytime.wHour);
                    //if (iCount % iModulo == 0) {
                    iGap = iLastX_VL - ChartToScrX(index_temp);
                    if (iGap < 0) { iGap = -iGap; }
                    if (iGapBetweenVL < iGap) {
                        if (!ObjectExists(text)) {
                            ObjectCreate(text, obj_VLine, 0, time_at_index, 0);
                        }
                        ObjectSet(text, OBJPROP_STYLE, psDot);
                        ObjectSet(text, OBJPROP_COLOR, LineColor);
                        ObjectSet(text, OBJPROP_WIDTH, 1);
                        ObjectSet(text, OBJPROP_TIME1, time_at_index);
                        iLastX_VL = ChartToScrX(index_temp);
                    }
                    else {
                        if (ObjectExists(text)) {
                            ObjectDelete(text);
                        }
                    }
                    sprintf_s(text, "TimeTxtScr%d-%d-%d-%d", mytime.wYear, mytime.wMonth, mytime.wDay, mytime.wHour);
                    iGap = iLastX_Time - ChartToScrX(index_temp);
                    if (iGap < 0) { iGap = -iGap; }
                    //if (iCount % iModulo == 0) {
                    if (iGapBetweenTime < iGap) {
                        iLocalHour = mytime.wHour + TimeDiff;
                        if (24 <= iLocalHour) {
                            iLocalHour -= 24;
                        }
                        else if (iLocalHour < 0) {
                            iLocalHour += 24;
                        }

                        //sprintf_s(time_text, "%d  %d", iLocalHour, ChartInfo.BarWidth);
                        sprintf_s(time_text, "%d", iLocalHour);

                        if (!ObjectExists(text)) {
                          ObjectCreate(text, obj_Text, 0, 0, 0);
                        }
                        ObjectSetText(text, time_text, FontSize, FontName, FontColor);
                        ObjectSet(text, OBJPROP_HALIGNMENT, taCenter);
                        ObjectSet(text, OBJPROP_COLOR, FontColor);
                        ObjectSet(text, OBJPROP_SCREENCOORDS, 1);
                        ObjectSet(text, OBJPROP_TIME1, ChartToScrX(index_temp + adjustTimeBar));
                        ObjectSet(text, OBJPROP_PRICE1, 20);
                        iLastX_Time = ChartToScrX(index_temp + adjustTimeBar);
                    }
                    else {
                        if (ObjectExists(text)) {
                            ObjectDelete(text);
                        }
                    }
                    iCount++;
                }
            }
            else {
                // delete all VTimeL & TimeText
                for (index_temp = ObjectsTotal() - 1; 0 <= index_temp; index_temp--) {
                    //ObjectName(index_temp);
                    strncpy_s(obj_name_tmp, ObjectName(index_temp), 10);
                    if (!strncmp(obj_name_tmp, "TimeTxtScr", 10)) {
                        ObjectDelete(ObjectName(index_temp));
                    }
                    else if (!strncmp(obj_name_tmp, "VTimeL", 6)) {
                        ObjectDelete(ObjectName(index_temp));
                    }
                }
            }
        }

        // for round number
        if(iDigitsOverride == 0){
          iNum_tmp = int (Close(0) * pow(10, digits));
        }else{
          iNum_tmp = int (Close(0) * pow(10, iDigitsOverride));
        }
        iNum_tmp /= 1000;
        iNum_tmp *= 1000;

        dNum_tmp = iNum_tmp;
        if(iDigitsOverride == 0){
          dNum_tmp /= pow(10, digits);
        }else{
          dNum_tmp /= pow(10, iDigitsOverride);
        }

        sprintf_s(text, "H_RN0");
        if (!ObjectExists(text)) {
          ObjectCreate(text, obj_HLine, 0, dNum_tmp, 0);
        }
        ObjectSet(text, OBJPROP_STYLE, psDot);
        ObjectSet(text, OBJPROP_COLOR, LineColor);
        ObjectSet(text, OBJPROP_WIDTH, 1);
        ObjectSet(text, OBJPROP_PRICE1, dNum_tmp);

        for(int k = 1 ; k <= iNumRoundNum ; k++){
          dNum_tmp = iNum_tmp + k*1000;
          if(iDigitsOverride == 0){
            dNum_tmp /= pow(10, digits);
          }else{
            dNum_tmp /= pow(10, iDigitsOverride);
          }
          sprintf_s(text, "H_RN%d_U", k);
          if (!ObjectExists(text)) {
            ObjectCreate(text, obj_HLine, 0, dNum_tmp, 0);
          }
          ObjectSet(text, OBJPROP_STYLE, psDot);
          ObjectSet(text, OBJPROP_COLOR, LineColor);
          ObjectSet(text, OBJPROP_WIDTH, 1);
          ObjectSet(text, OBJPROP_PRICE1, dNum_tmp);

          dNum_tmp = iNum_tmp - k*1000;
          if(iDigitsOverride == 0){
            dNum_tmp /= pow(10, digits);
          }else{
            dNum_tmp /= pow(10, iDigitsOverride);
          }
          sprintf_s(text, "H_RN%d_D", k);
          if (!ObjectExists(text)) {
            ObjectCreate(text, obj_HLine, 0, dNum_tmp, 0);
          }
          ObjectSet(text, OBJPROP_STYLE, psDot);
          ObjectSet(text, OBJPROP_COLOR, LineColor);
          ObjectSet(text, OBJPROP_WIDTH, 1);
          ObjectSet(text, OBJPROP_PRICE1, dNum_tmp);
        }
    }
}

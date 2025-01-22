//--------------------------------------------------------------
// File     : main.c
// Datum    : 15.08.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : CMSIS_BOOT, M4_CMSIS_CORE
// Funktion : Demo der Touch-Library
// Hinweis  : Diese zwei Files muessen auf 8MHz stehen
//              "cmsis_boot/stm32f4xx.h"
//              "cmsis_boot/system_stm32f4xx.c"
//--------------------------------------------------------------

#include "main.h"
#include "stm32_ub_lcd_st7783.h"
#include "stm32_ub_touch_ads7843.h"


int main(void)
{
  SystemInit(); // Инициализация настроек кварца

  UB_LCD_Init();    // Инициализация дисплея
  UB_Touch_Init();  // Инициализация тачскрина

  // Цвет экрана - синий
  UB_LCD_FillScreen(RGB_COL_BLACK);

  while(1)
  {
	  // Касание тачскрина
	  if(Touch_Data.status==TOUCH_PRESSED) {
		  // Когда происходит прикосновение,
		  // нарисовать пиксель по координатам прикосновения
		  UB_LCD_SetCursor2Draw(Touch_Data.pos.xp,Touch_Data.pos.yp);
		  LCD_RAM=RGB_COL_RED;
	  }

  }
}


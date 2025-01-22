//--------------------------------------------------------------
// File     : stm32_ub_touch_ads7843.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_TOUCH_ADS7843_H
#define __STM32F4_UB_TOUCH_ADS7843_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include <stdbool.h>


// Статус тачскрина
typedef enum {
  TOUCH_PRESSED  = 0,   // Тачскрин нажат
  TOUCH_RELEASED = 1	// Тачскрин отпущен
}Touch_Status_t;

// Координаты пикселя
typedef struct {
  uint16_t xp;
  uint16_t yp;
}Touch_Pixel_t;

// Постоянные значения для преобразования исходных данных в данные дисплея
typedef struct {
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint16_t xp1;
  uint16_t yp1;
  uint16_t xp2;
  uint16_t yp2;
  uint16_t xp3;
  uint16_t yp3;
}Touch_Konstanten_t;


//-----------------------------------------
// Внутренняя структура для калибровки
//-----------------------------------------
typedef struct {
  uint32_t timer_cnt;      // Счетчик таймера
  Touch_Pixel_t raw;       // Необработанные данные координат пикселя
  Touch_Pixel_t temp;      // Промежуточные значения координат пикселя
  Touch_Konstanten_t wert; // Постоянные значения
}Touch_Kal_t;
Touch_Kal_t Touch_Kal;


//-----------------------------------------
// Глобальная структура данных сенсора
//-----------------------------------------
typedef struct {
  Touch_Status_t status;   // Статус (нажат/отпущен)
  Touch_Pixel_t pos;       // Показать координаты пиксля
}Touch_Data_t;
Touch_Data_t Touch_Data;


//--------------------------------------------------------------
// Команды для контроллера тачскрина
//--------------------------------------------------------------
#define  UB_TOUCH_CMD_CH3  0x900000   // CH=X+, Mode=12bit, DiffEnded, PwrDown
#define  UB_TOUCH_CMD_CH4  0xD00000   // CH=Y+, Mode=12bit, DiffEnded, PwrDown


//--------------------------------------------------------------
// Определения
// Примечание: паузы сделаны без использования таймера, используются другие функции оптимизации -O0
//--------------------------------------------------------------
#define  UB_TOUCH_MESS_DELAY   100    // Пауза начала измерения
#define  UB_TOUCH_CLK_DELAY    1000   // Пауза для SPI-Clock-Signal


//--------------------------------------------------------------
// MAX_DIFF определяет расстояние между двумя нажатыми пикселями
// расстояние может быть слишком большим, чтобы назвать их соседними
//--------------------------------------------------------------
#define  UB_TOUCH_MAX_DIFF     30     // максимальное расстояние между пикселями


//--------------------------------------------------------------
// Косание точек калибровки
//--------------------------------------------------------------
#define   UB_TOUCH_KAL_XP1     120     // 50% от MAXX
#define   UB_TOUCH_KAL_YP1     288     // 90% от MAXY
#define   UB_TOUCH_KAL_XP2      24     // 10% от MAXX
#define   UB_TOUCH_KAL_YP2     160     // 50% от MAXY
#define   UB_TOUCH_KAL_XP3     216     // 90% от MAXX
#define   UB_TOUCH_KAL_YP3      32     // 10% от MAXY

#define   UB_TOUCH_KAL_CBG    RGB_COL_BLACK  // Цвет фона для калибровки
#define   UB_TOUCH_KAL_CPKT   RGB_COL_WHITE  // Цвет крестика
#define   UB_TOUCH_KAL_COK    RGB_COL_GREEN  // Фон, если калибровка прошла успешно
#define   UB_TOUCH_KAL_CERR   RGB_COL_RED    // Фон, если калибровка не прошла успешно

#define   UB_TOUCH_KAL_DELAY   10     // Пауза после калибровки (10 = 500ms)

//--------------------------------------------------------------
#define   UB_TOUCH_DC_TIME     10     // DoubleClick-время (10=500ms)
#define   UB_TOUCH_DC_TIMEOUT  20     // DoubleClick-перерыв (20=1sec)



//--------------------------------------------------------------
// Таймер тачскрина => TIM5
// Основная частота = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// TIM_Frq = TIM_CLK/(periode+1)/(prescaler+1)
// TIM_Frq = 20Hz => 50ms (Установить не менее 30 мс!!)
//--------------------------------------------------------------
#define   UB_TOUCH_TIM              TIM5
#define   UB_TOUCH_TIM_CLK          RCC_APB1Periph_TIM5
#define   UB_TOUCH_TIM_PERIODE      4999
#define   UB_TOUCH_TIM_PRESCALE     839
#define   UB_TOUCH_TIM_IRQ          TIM5_IRQn
#define   UB_TOUCH_TIM_ISR_HANDLER  TIM5_IRQHandler



//--------------------------------------------------------------
// Глобальная функция
//--------------------------------------------------------------
ErrorStatus UB_Touch_Init(void);
ErrorStatus UB_Toch_Calibrate(void);
bool UB_Touch_OnPressed(void);
bool UB_Touch_OnClick(void);
bool UB_Touch_OnRelease(void);
bool UB_Touch_OnDoubleClick(void);

//--------------------------------------------------------------
#endif // __STM32F4_UB_TOUCH_ADS7843_H

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


// ������ ���������
typedef enum {
  TOUCH_PRESSED  = 0,   // �������� �����
  TOUCH_RELEASED = 1	// �������� �������
}Touch_Status_t;

// ���������� �������
typedef struct {
  uint16_t xp;
  uint16_t yp;
}Touch_Pixel_t;

// ���������� �������� ��� �������������� �������� ������ � ������ �������
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
// ���������� ��������� ��� ����������
//-----------------------------------------
typedef struct {
  uint32_t timer_cnt;      // ������� �������
  Touch_Pixel_t raw;       // �������������� ������ ��������� �������
  Touch_Pixel_t temp;      // ������������� �������� ��������� �������
  Touch_Konstanten_t wert; // ���������� ��������
}Touch_Kal_t;
Touch_Kal_t Touch_Kal;


//-----------------------------------------
// ���������� ��������� ������ �������
//-----------------------------------------
typedef struct {
  Touch_Status_t status;   // ������ (�����/�������)
  Touch_Pixel_t pos;       // �������� ���������� ������
}Touch_Data_t;
Touch_Data_t Touch_Data;


//--------------------------------------------------------------
// ������� ��� ����������� ���������
//--------------------------------------------------------------
#define  UB_TOUCH_CMD_CH3  0x900000   // CH=X+, Mode=12bit, DiffEnded, PwrDown
#define  UB_TOUCH_CMD_CH4  0xD00000   // CH=Y+, Mode=12bit, DiffEnded, PwrDown


//--------------------------------------------------------------
// �����������
// ����������: ����� ������� ��� ������������� �������, ������������ ������ ������� ����������� -O0
//--------------------------------------------------------------
#define  UB_TOUCH_MESS_DELAY   100    // ����� ������ ���������
#define  UB_TOUCH_CLK_DELAY    1000   // ����� ��� SPI-Clock-Signal


//--------------------------------------------------------------
// MAX_DIFF ���������� ���������� ����� ����� �������� ���������
// ���������� ����� ���� ������� �������, ����� ������� �� ���������
//--------------------------------------------------------------
#define  UB_TOUCH_MAX_DIFF     30     // ������������ ���������� ����� ���������


//--------------------------------------------------------------
// ������� ����� ����������
//--------------------------------------------------------------
#define   UB_TOUCH_KAL_XP1     120     // 50% �� MAXX
#define   UB_TOUCH_KAL_YP1     288     // 90% �� MAXY
#define   UB_TOUCH_KAL_XP2      24     // 10% �� MAXX
#define   UB_TOUCH_KAL_YP2     160     // 50% �� MAXY
#define   UB_TOUCH_KAL_XP3     216     // 90% �� MAXX
#define   UB_TOUCH_KAL_YP3      32     // 10% �� MAXY

#define   UB_TOUCH_KAL_CBG    RGB_COL_BLACK  // ���� ���� ��� ����������
#define   UB_TOUCH_KAL_CPKT   RGB_COL_WHITE  // ���� ��������
#define   UB_TOUCH_KAL_COK    RGB_COL_GREEN  // ���, ���� ���������� ������ �������
#define   UB_TOUCH_KAL_CERR   RGB_COL_RED    // ���, ���� ���������� �� ������ �������

#define   UB_TOUCH_KAL_DELAY   10     // ����� ����� ���������� (10 = 500ms)

//--------------------------------------------------------------
#define   UB_TOUCH_DC_TIME     10     // DoubleClick-����� (10=500ms)
#define   UB_TOUCH_DC_TIMEOUT  20     // DoubleClick-������� (20=1sec)



//--------------------------------------------------------------
// ������ ��������� => TIM5
// �������� ������� = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// TIM_Frq = TIM_CLK/(periode+1)/(prescaler+1)
// TIM_Frq = 20Hz => 50ms (���������� �� ����� 30 ��!!)
//--------------------------------------------------------------
#define   UB_TOUCH_TIM              TIM5
#define   UB_TOUCH_TIM_CLK          RCC_APB1Periph_TIM5
#define   UB_TOUCH_TIM_PERIODE      4999
#define   UB_TOUCH_TIM_PRESCALE     839
#define   UB_TOUCH_TIM_IRQ          TIM5_IRQn
#define   UB_TOUCH_TIM_ISR_HANDLER  TIM5_IRQHandler



//--------------------------------------------------------------
// ���������� �������
//--------------------------------------------------------------
ErrorStatus UB_Touch_Init(void);
ErrorStatus UB_Toch_Calibrate(void);
bool UB_Touch_OnPressed(void);
bool UB_Touch_OnClick(void);
bool UB_Touch_OnRelease(void);
bool UB_Touch_OnDoubleClick(void);

//--------------------------------------------------------------
#endif // __STM32F4_UB_TOUCH_ADS7843_H

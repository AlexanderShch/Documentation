//--------------------------------------------------------------
// File     : stm32_ub_touch_ads7843.c
// Datum    : 21.08.2013
// Version  : 1.7
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO, TIM, MISC, STM32_UB_LCD_ST7783
// Funktion : Touch-Funktionen
//            (Chip = ADS7843, или XPT2046, или UH7843)
//
// Подключение : Для подключения тачскрина используются следующие пины микроконтроллера:
//            PA15 -> ADS7843_ChipSelect
//            PB1  -> ADS7843_DIN
//            PB4  -> ADS7843_Clk
//            PB5  -> ADS7843_DOUT
//            PB15 -> ADS7843_PEN
//            PC2  -> ADS7843_BUSY (в программном коде не используется)
//
//  Для опроса прикосновение используется Timer5 (TIM5) с циклическим опрашиванием (50ms)
//  а полученные данные хронятся в структуре:
//
//            Touch_Data.pos.xp = X-Position [0...239]
//            Touch_Data.pos.yp = Y-Position [0...319]
//            Touch_Data.status = [TOUCH_PRESSED, TOUCH_RELEASED]
//              -> Координаты сохраняются по событию "TOUCH_PRESSED"
//--------------------------------------------------------------




//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_touch_ads7843.h"
#include "stm32_ub_lcd_st7783.h"

//--------------------------------------------------------------
// Внутреннее перечисление
//--------------------------------------------------------------
typedef enum {
  KAL_PKT1 =0,
  KAL_PKT2,
  KAL_PKT3,
  KAL_READY,
  KAL_OK,
  KAL_ERR
}TOUCH_KAL_PKT_t;

typedef enum {
  MW_NONE =0,
  MW_4
}TOUCH_MW_t;

//--------------------------------------------------------------
// Внутренняя функция
//--------------------------------------------------------------
void P_Touch_InitIO(void);
void P_Touch_InitTIM(void);
void P_Touch_InitNVIC(void);
void P_Touch_Read(void);
void P_Touch_ReadRaw(TOUCH_MW_t mw);
void P_Touch_CalcPos(void);
ErrorStatus P_Touch_CheckPos(void);
void P_Touch_KalDrawPoint(TOUCH_KAL_PKT_t pkt);
void P_Touch_DrawKreuz(uint16_t xp, uint16_t yp);
void P_Touch_KalSetData(void);
void P_Touch_CLK(BitAction wert);
void P_Touch_CS(BitAction wert);
void P_Touch_DOUT(BitAction wert);
uint32_t P_Touch_DIN(void);
uint32_t P_Touch_PEN(void);
uint16_t P_Touch_Frame(uint32_t frame);
void P_Touch_Delay(volatile uint32_t nCount);


//--------------------------------------------------------------
// Инициализация тачскрина
// Return_wert :
//  -> ERROR   , когда тачскрин не найден
//  -> SUCCESS , когда тачскрин найден
//--------------------------------------------------------------
ErrorStatus UB_Touch_Init(void)
{
  ErrorStatus ret_wert=ERROR;

  // инициализация переменных
  Touch_Data.status=TOUCH_RELEASED;
  Touch_Data.pos.xp=0;
  Touch_Data.pos.yp=0;

  
  // калиброочные значения
  Touch_Kal.timer_cnt=0;
  Touch_Kal.raw.xp=0x8000;
  Touch_Kal.raw.yp=0x8000;
  Touch_Kal.temp.xp=0;
  Touch_Kal.temp.yp=0;

  //---------------------------------------------
  // Стандартные калибровки UB 
  // эти данные могут быть получены с помощью функции
  // "UB_Toch_Calibrate" определяемая
  // после калибровки отладчиком
  // и записываются как константа
  //---------------------------------------------
  Touch_Kal.wert.A=-0.077;
  Touch_Kal.wert.B=0.002;
  Touch_Kal.wert.C=248.238;
  Touch_Kal.wert.D=-0.002;
  Touch_Kal.wert.E=-0.101;
  Touch_Kal.wert.F=355.533;
  Touch_Kal.wert.xp1=1685;
  Touch_Kal.wert.yp1=625;
  Touch_Kal.wert.xp2=2971;
  Touch_Kal.wert.yp2=1853;
  Touch_Kal.wert.xp3=521;
  Touch_Kal.wert.yp3=3166;

  // инициализация IO-линий
  P_Touch_InitIO();

  //-----------------------------------------
  // Проверка работы сенсорного контроллера
  //-----------------------------------------
  P_Touch_ReadRaw(MW_4);

  // Проверка, проходят ли измерения
  if((Touch_Kal.raw.xp<=0x0FFF) && (Touch_Kal.raw.yp<=0x0FFF)) {
    ret_wert=SUCCESS;

    // инициализация таймера
    P_Touch_InitTIM();

    // Инициализация прерываний
    P_Touch_InitNVIC();
  }

  return(ret_wert);
}




//--------------------------------------------------------------
// Калибровка тачскрина
// Необходимо произвести нажатие в трех точках
// Return_wert :
//  -> ERROR   , когда настройка не прошла успешно
//  -> SUCCESS , когда настройка прошла успешно
//--------------------------------------------------------------
ErrorStatus UB_Toch_Calibrate(void)
{
  ErrorStatus ret_wert=ERROR;
  uint16_t xp1,yp1,xp2,yp2,xp3,yp3,ok;
  
  Touch_Kal.timer_cnt=0;
  while(Touch_Kal.timer_cnt<10) {
    // ожтдание нажатия
    if(Touch_Data.status==TOUCH_PRESSED) Touch_Kal.timer_cnt=0;
  }
  //---------------------------------
  // определение координат первой точки
  //---------------------------------
  P_Touch_KalDrawPoint(KAL_PKT1);
  Touch_Kal.timer_cnt=0;
  while(Touch_Kal.timer_cnt<UB_TOUCH_KAL_DELAY) {
    // ожидание, пока не произайдет нажатие
    if(Touch_Data.status==TOUCH_RELEASED) Touch_Kal.timer_cnt=0;
  }
  // сохранение данных
  xp1=Touch_Kal.raw.xp;
  yp1=Touch_Kal.raw.yp;
  P_Touch_KalDrawPoint(KAL_PKT2);
  Touch_Kal.timer_cnt=0;
  while(Touch_Kal.timer_cnt<10) {
    // ждет окончания нажатия
    if(Touch_Data.status==TOUCH_PRESSED) Touch_Kal.timer_cnt=0;
  }
  //---------------------------------
  // определение координат второй точки
  //---------------------------------
  Touch_Kal.timer_cnt=0;
  while(Touch_Kal.timer_cnt<UB_TOUCH_KAL_DELAY) {
    // ожидание, пока не произайдет нажатие
    if(Touch_Data.status==TOUCH_RELEASED) Touch_Kal.timer_cnt=0;
  }
  // сохранение данных
  xp2=Touch_Kal.raw.xp;
  yp2=Touch_Kal.raw.yp;
  P_Touch_KalDrawPoint(KAL_PKT3);
  Touch_Kal.timer_cnt=0;
  while(Touch_Kal.timer_cnt<10) {
    // ждет окончания нажатия
    if(Touch_Data.status==TOUCH_PRESSED) Touch_Kal.timer_cnt=0;
  }
  //---------------------------------
  // определение координат третьей точки
  //---------------------------------
  Touch_Kal.timer_cnt=0;
  while(Touch_Kal.timer_cnt<UB_TOUCH_KAL_DELAY) {
    // ожидание, пока не произайдет нажатие
    if(Touch_Data.status==TOUCH_RELEASED) Touch_Kal.timer_cnt=0;
  }
  // сохранение данных
  xp3=Touch_Kal.raw.xp;
  yp3=Touch_Kal.raw.yp;
  P_Touch_KalDrawPoint(KAL_READY);
  Touch_Kal.timer_cnt=0;
  while(Touch_Kal.timer_cnt<10) {
    // ждет окончания нажатия
    if(Touch_Data.status==TOUCH_PRESSED) Touch_Kal.timer_cnt=0;
  }
  // Калибровка завершена
  // проверка правильности координат
  ok=0;
  if((yp3>yp2) && (yp2>yp1)) ok++;
  if((yp3<yp2) && (yp2<yp1)) ok++;
  if((xp2>xp1) && (xp1>xp3)) ok++;
  if((xp2<xp1) && (xp1<xp3)) ok++;
  if(ok>=2) {
    // значение впорядке
    // преобразование и хранение данных
    Touch_Kal.wert.xp1=xp1;
    Touch_Kal.wert.yp1=yp1;
    Touch_Kal.wert.xp2=xp2;
    Touch_Kal.wert.yp2=yp2;
    Touch_Kal.wert.xp3=xp3;
    Touch_Kal.wert.yp3=yp3;
    P_Touch_KalSetData();
    P_Touch_KalDrawPoint(KAL_OK);
    ret_wert=SUCCESS;
  } 
  else {
    P_Touch_KalDrawPoint(KAL_ERR);
    ret_wert=ERROR;
  }
  
  return(ret_wert);
}


//--------------------------------------------------------------
// Событие OnPressed, прикосновение
// ret_wert, верно, пока не окончится нажатие
//--------------------------------------------------------------
bool UB_Touch_OnPressed(void)
{
  bool ret_wert=false;

  if(Touch_Data.status==TOUCH_PRESSED) {
    ret_wert=true;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Событие OnClick 
// ret_wert, верно только один раз, в момент нажатия
//--------------------------------------------------------------
bool UB_Touch_OnClick(void)
{
  bool ret_wert=false;
  static bool old_wert=false;

  if(Touch_Data.status==TOUCH_PRESSED) {
    if(old_wert==false) {
      ret_wert=true;
    }
    old_wert=true;
  }
  else {
    old_wert=false;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Событие OnRelease
// ret_wert, верно только один раз, в момент отпускания
//--------------------------------------------------------------
bool UB_Touch_OnRelease(void)
{
  bool ret_wert=false;
  static bool old_wert=false;

  if(Touch_Data.status==TOUCH_RELEASED) {
    if(old_wert==true) {
      ret_wert=true;
    }
    old_wert=false;
  }
  else {
    old_wert=true;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Событие OnDoubleClick
// ret_wert, верно только один раз, когда совершено два нажатия подряд
//--------------------------------------------------------------
bool UB_Touch_OnDoubleClick(void)
{
  bool ret_wert=false;
  static uint8_t mode=0;

  if(mode==0) {
    // Ожидание первого нажатия
    if(Touch_Data.status==TOUCH_PRESSED) {
      mode=1;
      Touch_Kal.timer_cnt=0;
    }
  }
  else if(mode==1) {
    // Ожидание первого отпускания
    if(Touch_Data.status==TOUCH_RELEASED) {
      mode=2;
    }
    else {
      // Проверка на длительность нажатия
      if(Touch_Kal.timer_cnt>UB_TOUCH_DC_TIMEOUT) {
        mode=3;
      }
    }
  }
  else if(mode==2) {
    // Ожидание второго нажатия
    if(Touch_Data.status==TOUCH_PRESSED) {
      mode=3;
      if(Touch_Kal.timer_cnt<UB_TOUCH_DC_TIME) {
        ret_wert=true;
      }
    }
    else {
      // Проверка на длительность нажатия
      if(Touch_Kal.timer_cnt>UB_TOUCH_DC_TIMEOUT) {
        mode=0;
      }
    }
  }
  else {
    // Ожидание второго отпускания
    if(Touch_Data.status==TOUCH_RELEASED) {
      mode=0;
    }
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Внутренняя функция
// Инициализация всех выводов сенсорного контроллера
// PA15 = OUT = ChipSelect
// PB1  = OUT = Data-Out
// PB4  = OUT = Clock
// PB5  =  IN = Data-In
// PB15 =  IN = Pen-Interrupt
// PC2  =  IN = Busy (не используется)
//--------------------------------------------------------------
void P_Touch_InitIO(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  //-----------------------------------------
  // Включение тактирования Port-A, Port-B, Port-C
  //-----------------------------------------
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);

  //-----------------------------------------
  // Инициализация всех пинов Port-A
  //-----------------------------------------
  // Настройки
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //-----------------------------------------
  // Инициализация всех пинов Port-B
  //-----------------------------------------
  // Настройки
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  // Eigaenge
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //-----------------------------------------
  // Инициализация всех пинов Port-C 
  //-----------------------------------------
  // Настройки
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //-----------------------------------------
  // Установка всех пинов
  //-----------------------------------------
  P_Touch_CS(Bit_SET);
  P_Touch_CLK(Bit_RESET);
  P_Touch_DOUT(Bit_RESET);
}


//--------------------------------------------------------------
// Внутренняя функция
// Инициализация таймера
//--------------------------------------------------------------
void P_Touch_InitTIM(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  // Включение тактрования
  RCC_APB1PeriphClockCmd(UB_TOUCH_TIM_CLK, ENABLE);

  // Инициализация таймера
  TIM_TimeBaseStructure.TIM_Period =  UB_TOUCH_TIM_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = UB_TOUCH_TIM_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(UB_TOUCH_TIM, &TIM_TimeBaseStructure);

  // Включение таймера
  TIM_ARRPreloadConfig(UB_TOUCH_TIM, ENABLE);
  TIM_Cmd(UB_TOUCH_TIM, ENABLE);
}


//--------------------------------------------------------------
// Внутренняя функция
// Инициализация прерывания
//--------------------------------------------------------------
void P_Touch_InitNVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //---------------------------------------------
  // инициализация таймера по прерыванию
  //---------------------------------------------

  TIM_ITConfig(UB_TOUCH_TIM,TIM_IT_Update,ENABLE);

  // NVIC конфигурация
  NVIC_InitStructure.NVIC_IRQChannel = UB_TOUCH_TIM_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// Эта функция вызывается переодически по таймеру
//
// Зачитывание прикосновений и сохранение в структуру "Touch_Data"
//    Touch_Data.status => [TOUCH_RELEASED, TOUCH_PRESSED]
//    Touch_Data.pos.xp => X-координата прикосновения
//    Touch_Data.pos.yp => Y-координата прикосновения
//
// -> Если прикосновение не происходит,
//    значение координат недействительно
//--------------------------------------------------------------
void P_Touch_Read(void)
{
  Touch_Status_t akt_status=TOUCH_RELEASED;

  // Зачитать прикосновение
  if(P_Touch_PEN()==Bit_RESET) {
    akt_status=TOUCH_PRESSED;
  }
  else {
    akt_status=TOUCH_RELEASED;
  }

  if(akt_status==TOUCH_PRESSED) {
    // Когда касаются сенсора, оценить прикосновение
    // Захват данных косания (4 средних)
    P_Touch_ReadRaw(MW_4);
    // Преобразование данных тачскрина в координаты на дисплее
    P_Touch_CalcPos();
    // Проверка действительности данных
    if(P_Touch_CheckPos()==SUCCESS) {
      Touch_Data.status=TOUCH_PRESSED;
    }
  }
  else {
    Touch_Data.status=TOUCH_RELEASED;
    // Данные сенсора не должны быть прочитаны
  }
}

//--------------------------------------------------------------
// Внутренняя функция
// Чтение "сырых" данных с сенсора
//  mw => [MW_NONE, MW_4]
//--------------------------------------------------------------
void P_Touch_ReadRaw(TOUCH_MW_t mw)
{
  uint16_t n,wert;
  uint32_t xp=0,yp=0;

  if(mw==MW_NONE) {
    // без усреднения
    // читать в CH-4 (X)
    Touch_Kal.raw.xp=P_Touch_Frame(UB_TOUCH_CMD_CH4);
    Touch_Kal.raw.yp=P_Touch_Frame(UB_TOUCH_CMD_CH3);
  }
  else {
    // усреднить первое значение
    P_Touch_Frame(UB_TOUCH_CMD_CH4);
    // Усреднение 4х знчений
    for(n=0;n<4;n++) {
      // читать в CH-4 (X)
      wert=P_Touch_Frame(UB_TOUCH_CMD_CH4);
      xp+=wert;
      // читать в CH-3 (Y)
      wert=P_Touch_Frame(UB_TOUCH_CMD_CH3);
      yp+=wert;
    }
    // полученные значения
    Touch_Kal.raw.xp=(xp>>2);
    Touch_Kal.raw.yp=(yp>>2);
  }
}

//--------------------------------------------------------------
// Внутренняя функция
// Преобразование сенсорных значений в пиксельные координаты
//--------------------------------------------------------------
void P_Touch_CalcPos(void)
{
  float f1,f2,XTf,YTf;
  int32_t i1,i2,i3;
  int32_t XD,YD;
  
  // Исходные данные
  XTf=(float)(Touch_Kal.raw.xp);
  YTf=(float)(Touch_Kal.raw.yp);
  
  // Расчет X-координаты
  f1=Touch_Kal.wert.A*XTf;
  i1=(int)(f1);
  f2=Touch_Kal.wert.B*YTf;
  i2=(int)(f2);
  i3=(int)(Touch_Kal.wert.C);
  XD=i1+i2+i3;
  
  // Расчет Y-координаты
  f1=Touch_Kal.wert.D*XTf;
  i1=(int)(f1);
  f2=Touch_Kal.wert.E*YTf;
  i2=(int)(f2);
  i3=(int)(Touch_Kal.wert.F);
  YD=i1+i2+i3;
  
  // Проверка значения в пределах отображения
  if(XD>=LCD_MAXX) XD=(LCD_MAXX-1);
  if(XD<0) XD=0;

  if(YD>=LCD_MAXY) YD=(LCD_MAXY-1);
  if(YD<0) YD=0;
  
  Touch_Kal.temp.xp=XD;
  Touch_Kal.temp.yp=YD;
}


//--------------------------------------------------------------
// Внутренняя функция
// Проверка правильности данных координат
// Return_wert :
//  -> ERROR   , Данные верны
//  -> SUCCESS , Данные не верны
//--------------------------------------------------------------
ErrorStatus P_Touch_CheckPos(void)
{
  ErrorStatus ret_wert=SUCCESS;
  static uint16_t old_x=0,old_y=0;
  uint16_t diff;

  if(Touch_Kal.temp.xp>=old_x) {
    diff=Touch_Kal.temp.xp-old_x;
  }
  else {
    diff=old_x-Touch_Kal.temp.xp;
  }
  if(diff>UB_TOUCH_MAX_DIFF) ret_wert=ERROR;

  if(Touch_Kal.temp.yp>=old_y) {
    diff=Touch_Kal.temp.yp-old_y;
  }
  else {
    diff=old_y-Touch_Kal.temp.yp;
  }
  if(diff>UB_TOUCH_MAX_DIFF) ret_wert=ERROR;

  old_x=Touch_Kal.temp.xp;
  old_y=Touch_Kal.temp.yp;

  if(ret_wert==SUCCESS) {
    // если данные впорядке, сохранить их
    Touch_Data.pos.xp=Touch_Kal.temp.xp;
    Touch_Data.pos.yp=Touch_Kal.temp.yp;
  }

  return(ret_wert);
}

//--------------------------------------------------------------
// Внутренняя функция
// Рисование изодражения калибровки
//  pkt => [KAL_PKT1,KAL_PKT2,KAL_PKT3,KAL_READY,KAL_OK,KAL_ERR]
//--------------------------------------------------------------
void P_Touch_KalDrawPoint(TOUCH_KAL_PKT_t pkt)
{

  if(pkt==KAL_PKT1) {
    P_Touch_DrawKreuz(UB_TOUCH_KAL_XP1,UB_TOUCH_KAL_YP1);
  }
  else if(pkt==KAL_PKT2) {
    P_Touch_DrawKreuz(UB_TOUCH_KAL_XP2,UB_TOUCH_KAL_YP2);
  }  
  else if(pkt==KAL_PKT3) {
    P_Touch_DrawKreuz(UB_TOUCH_KAL_XP3,UB_TOUCH_KAL_YP3);
  }
  else if(pkt==KAL_READY) {
    UB_LCD_FillScreen(UB_TOUCH_KAL_CBG);
  }
  else if(pkt==KAL_OK) {
    UB_LCD_FillScreen(UB_TOUCH_KAL_COK);
  }
  else {
    UB_LCD_FillScreen(UB_TOUCH_KAL_CERR);
  }
}

//--------------------------------------------------------------
// Внутренняя функция
// Преобразование калибровки в Xp, Yp
//--------------------------------------------------------------
void P_Touch_DrawKreuz(uint16_t xp, uint16_t yp)
{
  UB_LCD_FillScreen(UB_TOUCH_KAL_CBG);
  UB_LCD_SetCursor2Draw(xp,yp-1);
  LCD_RAM = UB_TOUCH_KAL_CPKT;
  UB_LCD_SetCursor2Draw(xp-1,yp);
  LCD_RAM = UB_TOUCH_KAL_CPKT;
  UB_LCD_SetCursor2Draw(xp,yp+1);
  LCD_RAM = UB_TOUCH_KAL_CPKT;
  UB_LCD_SetCursor2Draw(xp+1,yp);
  LCD_RAM = UB_TOUCH_KAL_CPKT;
  UB_LCD_SetCursor2Draw(xp,yp);
  LCD_RAM = UB_TOUCH_KAL_CPKT;
}

//--------------------------------------------------------------
// Внутренняя функция
// Вычисление калибровки
//--------------------------------------------------------------
void P_Touch_KalSetData(void)
{
  float XD1,YD1,XD2,YD2,XD3,YD3;
  float XT1f,YT1f,XT2f,YT2f,XT3f,YT3f;
  float f1,f2,f3,f4,f5,f6,fz,fn;  
  
  XD1=(float)(UB_TOUCH_KAL_XP1);
  YD1=(float)(UB_TOUCH_KAL_YP1);
  XD2=(float)(UB_TOUCH_KAL_XP2);
  YD2=(float)(UB_TOUCH_KAL_YP2); 
  XD3=(float)(UB_TOUCH_KAL_XP3);
  YD3=(float)(UB_TOUCH_KAL_YP3); 
  
  XT1f=(float)(Touch_Kal.wert.xp1);
  YT1f=(float)(Touch_Kal.wert.yp1);
  XT2f=(float)(Touch_Kal.wert.xp2);
  YT2f=(float)(Touch_Kal.wert.yp2);
  XT3f=(float)(Touch_Kal.wert.xp3);
  YT3f=(float)(Touch_Kal.wert.yp3);
  
  // Расчет A
  f1=XD1*(YT2f-YT3f);  
  f2=XD2*(YT3f-YT1f);  
  f3=XD3*(YT1f-YT2f);  
  f4=XT1f*(YT2f-YT3f);  
  f5=XT2f*(YT3f-YT1f);  
  f6=XT3f*(YT1f-YT2f);
  fz=f1+f2+f3;
  fn=f4+f5+f6; 
  if(fn==0) fn=0.1;
  Touch_Kal.wert.A=fz/fn;
  
  // Расчет B 
  f1=Touch_Kal.wert.A*(XT3f-XT2f);
  f2=XD2;
  f3=XD3;
  fz=f1+f2-f3;
  fn=(YT2f-YT3f);
  if(fn==0) fn=0.1;
  Touch_Kal.wert.B=fz/fn;
  
  // Расчет C 
  f1=XD3;
  f2=Touch_Kal.wert.A*XT3f;
  f3=Touch_Kal.wert.B*YT3f;
  Touch_Kal.wert.C=f1-f2-f3;
  
  // Расчет D 
  f1=YD1*(YT2f-YT3f);
  f2=YD2*(YT3f-YT1f);
  f3=YD3*(YT1f-YT2f);
  f4=XT1f*(YT2f-YT3f);
  f5=XT2f*(YT3f-YT1f);
  f6=XT3f*(YT1f-YT2f);  
  fz=f1+f2+f3;
  fn=f4+f5+f6;  
  if(fn==0) fn=0.1;
  Touch_Kal.wert.D=fz/fn;
  
  // Расчет E 
  f1=Touch_Kal.wert.D*(XT3f-XT2f);
  f2=YD2;
  f3=YD3;
  fz=f1+f2-f3;
  fn=(YT2f-YT3f);
  if(fn==0) fn=0.1;
  Touch_Kal.wert.E=fz/fn;
  
  // Расчет F 
  f1=YD3;
  f2=Touch_Kal.wert.D*XT3f;
  f3=Touch_Kal.wert.E*YT3f;
  Touch_Kal.wert.F=f1-f2-f3;
}

//--------------------------------------------------------------
// Внутренняя функция
// Установка уровня CLK
//--------------------------------------------------------------
void P_Touch_CLK(BitAction wert) {
  if(wert==Bit_RESET) {
    GPIOB->BSRRH = GPIO_Pin_4;
  }
  else {
    GPIOB->BSRRL = GPIO_Pin_4;
  }
}

//--------------------------------------------------------------
// Внутренняя функция
// Установка уровня ChipSelect
//--------------------------------------------------------------
void P_Touch_CS(BitAction wert) {
  if(wert==Bit_RESET) {
    GPIOA->BSRRH = GPIO_Pin_15;
  }
  else {
    GPIOA->BSRRL = GPIO_Pin_15;
  }
}

//--------------------------------------------------------------
// Внутренняя функция
// Установка уровня Data-Out
//--------------------------------------------------------------
void P_Touch_DOUT(BitAction wert) {
  if(wert==Bit_RESET) {
    GPIOB->BSRRH = GPIO_Pin_1;
  }
  else {
    GPIOB->BSRRL = GPIO_Pin_1;
  }
}

//--------------------------------------------------------------
// Внутренняя функция
// Установка уровня DIN 
//--------------------------------------------------------------
uint32_t P_Touch_DIN(void)
{
  return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
}

//--------------------------------------------------------------
// Внутренняя функция
// Установка уровня PEN 
//--------------------------------------------------------------
uint32_t P_Touch_PEN(void)
{
  return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);
}

//--------------------------------------------------------------
// Внутренняя функция
// Отправка 24рамки на ADS7843
// и получение значения данных 12 бит
// Примечание: паузы сделаны без таймера!!
//--------------------------------------------------------------
uint16_t P_Touch_Frame(uint32_t frame) {
  uint16_t n;
  uint32_t maske_out=0x800000; // 24Bit маска передачи данных
  uint16_t maske_in=0x800;     // 12Bit маска приема данных
  uint16_t data_in=0x00;

  // небольшая пауза
  P_Touch_Delay(UB_TOUCH_MESS_DELAY);

  // Chip-Select в низкий уровень
  P_Touch_CS(Bit_RESET);
  P_Touch_Delay(UB_TOUCH_CLK_DELAY);
  // передать 24 битный кадр
  for(n=0;n<24;n++) {
    if((frame & maske_out)==0) {
      P_Touch_DOUT(Bit_RESET);
    }
    else {
      P_Touch_DOUT(Bit_SET);
    }
    P_Touch_CLK(Bit_SET);
    if((n>=9) && (n<=20)) {
      if(P_Touch_DIN()!=Bit_RESET) {
        data_in|=maske_in;
      }
      maske_in = (maske_in>>1);
    }
    P_Touch_Delay(UB_TOUCH_CLK_DELAY);
    P_Touch_CLK(Bit_RESET);
    P_Touch_Delay(UB_TOUCH_CLK_DELAY);
    maske_out = (maske_out>>1);
  }
  // Chip-Select в высокий уровень
  P_Touch_Delay(UB_TOUCH_CLK_DELAY);
  P_Touch_CS(Bit_SET);
  P_Touch_Delay(UB_TOUCH_CLK_DELAY);

  return(data_in);
}


//--------------------------------------------------------------
// Пауза по таймеру
//--------------------------------------------------------------
void P_Touch_Delay(volatile uint32_t nCount)
{
  while(nCount--)
  {
  }
}

//--------------------------------------------------------------
// ISR таймера
//--------------------------------------------------------------
void UB_TOUCH_TIM_ISR_HANDLER(void)
{
  // есть только один источник прерывания
  TIM_ClearITPendingBit(UB_TOUCH_TIM, TIM_IT_Update);

  Touch_Kal.timer_cnt++;

  // Чтения из тачскрина
  P_Touch_Read();
}

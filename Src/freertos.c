/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "SSD1306.h"
#include "string.h"
#include "rtc.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId ReceiverHandle;
osThreadId Sender_1Handle;
osThreadId Sender_2Handle;
osSemaphoreId myBinarySem01Handle;

/* USER CODE BEGIN Variables */
const char Chars[] = "01234567890ABCDEF";
const char* Day[] = { "RR","Mon","Tue","Wed","Thu","Fri","Sat","Sun" };


//it will be rewritten after program start
char run_string[] = "max lenth is 20 symb"; 

const char *ptMess;

time  clock;
date  calendar;
char  WeekDay[]="Sun";  // 

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartReceiverTask(void const * argument);
void StartSenderTask_1(void const * argument);
void StartSenderTask_2(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of myBinarySem01 */
  osSemaphoreDef(myBinarySem01);
  myBinarySem01Handle = osSemaphoreCreate(osSemaphore(myBinarySem01), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of Receiver */
  osThreadDef(Receiver, StartReceiverTask, osPriorityNormal, 0, 64);
  ReceiverHandle = osThreadCreate(osThread(Receiver), NULL);

  /* definition and creation of Sender_1 */
  osThreadDef(Sender_1, StartSenderTask_1, osPriorityLow, 0, 64);
  Sender_1Handle = osThreadCreate(osThread(Sender_1), NULL);

  /* definition and creation of Sender_2 */
  osThreadDef(Sender_2, StartSenderTask_2, osPriorityLow, 0, 64);
  Sender_2Handle = osThreadCreate(osThread(Sender_2), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartReceiverTask function */
void StartReceiverTask(void const * argument)
{
    static _Bool rot_flag = 1;
    static uint32_t batt_vltg = 3218;
  /* USER CODE BEGIN StartReceiverTask */
  /* Infinite loop */
  for(;;)
  {

    LCD_Clear();  
    LCD_2xFStr(clock.all,2,2);

      
    if(rot_flag)
    {
        clock.units.hD = clock.units.mD = '}';
    }        
    else
    {
        clock.units.hD = clock.units.mD = '~'; 
    }        
    rot_flag ^= 1;

    LCD_FStr(WeekDay,0,0);     
    LCD_FStr(calendar.all,4,0);

    LCD_Batt(batt_vltg,120,0);
    if((++batt_vltg) > 4096) batt_vltg = 3218;   //simulate charging process

    LCD_FStr(run_string,0,3);

    LCD_Update();
    osDelay(20);     
  }
  /* USER CODE END StartReceiverTask */
}

/* StartSenderTask_1 function */
void StartSenderTask_1(void const * argument)
{
  /* USER CODE BEGIN StartSenderTask_1 */
    strcpy(clock.all,"23}59}50");      
    strcpy(calendar.all,"01-01-18");
    
  /* Infinite loop */
  for(;;)
  {
    HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BCD);
       
    clock.units.hT = Chars[sTime.Hours>>4];
    clock.units.hU = Chars[sTime.Hours&0x0F];   
    clock.units.mT = Chars[sTime.Minutes>>4];
    clock.units.mU = Chars[sTime.Minutes&0x0F];
    clock.units.sT = Chars[sTime.Seconds>>4];
    clock.units.sU = Chars[sTime.Seconds&0x0F];
       
    strcpy(WeekDay,Day[sDate.WeekDay]);
    calendar.units.YearT  = Chars[sDate.Year>>4];
    calendar.units.YearU  = Chars[sDate.Year&0x0F];   
    calendar.units.MonthT = Chars[sDate.Month>>4];
    calendar.units.MonthU = Chars[sDate.Month&0x0F];
    calendar.units.DateT  = Chars[sDate.Date>>4];
    calendar.units.DateU  = Chars[sDate.Date&0x0F];
      
    osDelay(50);
  }
  /* USER CODE END StartSenderTask_1 */
}

/* StartSenderTask_2 function */
void StartSenderTask_2(void const * argument)
{
  /* USER CODE BEGIN StartSenderTask_2 */
    const char Message[] = " This is the run string where you can write everything you want to write<<<";
    const char *ptmp;
  /* Infinite loop */
  for(;;)
  {
    ptmp = ptMess;    //start from the last symbol of the last session
    ptMess++;          //go to the next symbol
    if(*ptMess == '\0')ptMess = Message; 

    //fill the run_string buffer
    for(uint8_t str_cnt = 0; str_cnt < sizeof(run_string); str_cnt++) 
    {
         if(*(++ptmp) == '\0') //loop 
         {
            ptmp = Message;         
         }
         run_string[str_cnt] =* ptmp;  //write a new symbol
    } 
    osDelay(100);
  }
  /* USER CODE END StartSenderTask_2 */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


// 112
/* Definitions for TX_To_Server */
osThreadId_t TX_To_ServerHandle;
const osThreadAttr_t TX_To_Server_attributes = {
  .name = "TX_To_Server",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Data_Queue */
osMessageQueueId_t Data_QueueHandle;
const osMessageQueueAttr_t Data_Queue_attributes = {
  .name = "Data_Queue"
};

// 174
void TransferToServer(void *argument);

// 291
  /* Create the queue(s) */
  /* creation of Data_Queue */
  Data_QueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &Data_Queue_attributes);

// 312
  /* creation of TX_To_Server */
  TX_To_ServerHandle = osThreadNew(TransferToServer, NULL, &TX_To_Server_attributes);

// 1192
/* USER CODE BEGIN Header_TransferToServer */
/**
* @brief Function implementing the TX_To_Server thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TransferToServer */
void TransferToServer(void *argument)
{
  /* USER CODE BEGIN TransferToServer */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END TransferToServer */
}
#include "LoRaApi.h"
#include <stdio.h>
#include <string.h>
#include "tos_k.h"
#include "atcmd-board.h"
/*LORA����״̬*/
uint8_t g_join_state = 0;
uint8_t g_st_state = 0;
/*���ļ���*/
uint32_t g_rx_num = 0;

//�������ȼ�
#define TASK1_PRIO		3
//�����ջ��С
#define TASK1_STK_SIZE 		(1024 * 4)
//������ƿ�
k_task_t Task1TCB;
//�����ջ
k_stack_t TASK1_STK[TASK1_STK_SIZE];
//������
void task1(void *arg);
void lora_send_timer(void* context)
{
	g_st_state =2;
}
void task1(void *arg)
{
	at_cmd_init();
	LoRa_Init();
	printf_device_info();
	TimerInit( &TxNextPacketTimer, lora_send_timer );
	while(1)
	{
		LoRaMacProcess();
		if(g_join_state == 0)
		{
				printf("start join...\r\n");
				/*����*/
				LoRa_JoinNetwork(1);
				g_join_state = 1;
		}
		if(g_join_state == 2)
		{
				/*������ʱ��������*/
				if(g_st_state == 0)
				{
						g_st_state = 1;
						TimerSetValue( &TxNextPacketTimer, 60000 );
						TimerStart( &TxNextPacketTimer);
						printf("\r\ntime to ");
				}
				else if(g_st_state == 2)
				{
						g_st_state = 3;
						printf("send!\r\n");
						/*����lora����*/
						uint8_t tmpbuf[] = "tos tiny!";
						Lora_Send(0, 2, tmpbuf, sizeof(tmpbuf));
						g_rx_num++;
				}
		}
		at_cmd_handle();
	}
}

extern uint8_t sx1276_band;
int main(void)
{
	k_err_t err;
	TOS_CPU_CPSR_ALLOC();
	BoardInitMcu( );
	BoardInitPeriph( );
	sx1276_band = 2;
	printf("welcome to tencent IoTOS\r\n");
	err = tos_knl_init();		//��ʼ��TOS
	if (err != K_ERR_NONE)
	{
		printf("tos init failed\r\n");
		return err;
	}
	TOS_CPU_INT_DISABLE();//�����ٽ���
	err = tos_task_create((k_task_t 	* )&Task1TCB,		//������ƿ�
					 (char	* )"task1", 		//��������
									 (k_task_entry_t )task1, 			//������
									 (void		* )0,					//���ݸ��������Ĳ���
									 (k_prio_t	  )TASK1_PRIO,     //�������ȼ�
									  TASK1_STK,	//�����ջ����ַ
									 (size_t)TASK1_STK_SIZE,		//�����ջ��С
									 (size_t	  )0);			//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�

	TOS_CPU_INT_ENABLE();	//�˳��ٽ���
	tos_knl_start();      //����TOS
  while (1)
  {

  }
}


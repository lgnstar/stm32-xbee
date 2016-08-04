#include "zig.h"
/**
  * @brief  This function turn data to the Transmit packet.
  * @param  T_packet:������ݰ��� T_data:Ҫ��������ݡ�  dest_add:�����Ŀ�ĵ�ַ   datanum:��������ݳ���
  * @retval packet_length
  */
uint16_t Tramsmit_packet(uint8_t *T_packet,uint8_t *T_data,uint8_t *dest_add,uint16_t datanum)
{
	uint16_t i,length; //У��λ��Ҫ����ĳ���
	uint8_t check=0x00;
	/*д�����ݰ�����*/
	uint16_t packet_length=datanum+18;
	length=packet_length-4;
	T_packet[0]=0x7E;
	T_packet[2]=(unsigned char)(length)&0x00ff;
	T_packet[1]=(unsigned char)(length>>8)&0x00ff;
	T_packet[3]=0x10;
	T_packet[4]=0x01;
	T_packet[13]=0xFF;
	T_packet[14]=0xFE;
	/*д��Ŀ�ĵ�ַ*/
  for(i=0;i<8;i++)
		{
			T_packet[i+5]=dest_add[i];
		}
		
  /*д������*/
	for(i=0;i<datanum;i++)
		{
			T_packet[i+17]=T_data[i];
		}
	
		/*����У��λ*/
		for(i=0;i<length;i++)
			{
				check=(uint8_t)(check+T_packet[i+3]);
			}
		check=0xFF-check;
		T_packet[packet_length-1]=check;
		return packet_length;
}

uint16_t Recieve_packet(uint8_t *R_packet,uint8_t *R_data,uint8_t *resource_add,uint16_t p_num)
{
	uint16_t i,data_length;
	data_length=p_num-16;
	for(i=0;i<8;i++)
		{
			resource_add[i]=R_packet[i+5];
		}
	for(i=0;i<data_length;i++)
		{
			R_data[i]=R_packet[i+15];
		}
	return data_length;
}

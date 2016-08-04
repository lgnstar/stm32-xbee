#include "bsp_bmp.h"
#include "ff.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_usart1.h"
#include "delay.h"
#include "bsp_ov7725.h"
#include "bsp_sccb.h"
#include "zig.h"
#define RGB24TORGB16(R,G,B) ((unsigned short int)((((R)>>3)<<11) | (((G)>>2)<<5)	| ((B)>>3)))
extern volatile uint8_t usart_flag;
extern volatile uint8_t Ov7725_vsync ;
extern uint8_t dest_address[8];
uint8_t pColorData[960];					/* һ�����ɫ���ݻ��� 320 * 3 = 960 */
uint8_t bmp_data[120];
uint8_t pack_data[200];
uint8_t pack_head[100];
//tagRGBQUAD dataOfBmp[17*19];
FATFS bmpfs[2]; 
FIL bmpfsrc, bmpfdst; 
FRESULT bmpres;
/* bmp  �ļ�ͷ 54���ֽ� */
unsigned char bmp_head[54] =
	{
		0x42, 0x4d, 0, 0, 0, 0, 
		0, 0, 0, 0, 54, 0, 
		0, 0, 40,0, 0, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 1, 0, 24, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 
		0, 0, 0
	};
/* �������Ҫ��ӡbmp��ص���ʾ��Ϣ,��printfע�͵�����
 * ��Ҫ��printf()���轫���������ļ���������
 */
#define BMP_DEBUG_PRINTF(FORMAT,...)  printf(FORMAT,##__VA_ARGS__)	 


/* ������BMP�ļ���ͷ��Ϣ */
static void headerdeal(BITMAPFILEHEADER* pBmpHead,tagBITMAPINFOHEADER* pBmpInforHead)
{
		/* �ļ���С 4���ֽ� */	
		bmp_head[2] = (unsigned char)((*pBmpHead).bfSize &0x000000ff);
	  bmp_head[3] = ((*pBmpHead).bfSize >> 8) & 0x000000ff;
	  bmp_head[4] = ((*pBmpHead).bfSize >> 16) & 0x000000ff;
	  bmp_head[5] = ((*pBmpHead).bfSize >> 24) & 0x000000ff;
	  /*ʵ��λͼ���ݵ�ƫ���ֽ���*/
		bmp_head[10] = (*pBmpHead).bfOffBits &0x000000ff;
	  bmp_head[11] = ((*pBmpHead).bfOffBits >> 8) & 0x000000ff;
	  bmp_head[12] = ((*pBmpHead).bfOffBits >> 16) & 0x000000ff;
	  bmp_head[13] = ((*pBmpHead).bfOffBits >> 24) & 0x000000ff;	
	
	  /*ָ���˽ṹ��ĳ���*/
		bmp_head[14] = (*pBmpInforHead).biSize & 0x000000ff;
	  bmp_head[15] = ((*pBmpInforHead).biSize >> 8) &0x000000ff;
	  bmp_head[16] = ((*pBmpInforHead).biSize>> 16) &0x000000ff;
	  bmp_head[17] = ((*pBmpInforHead).biSize >> 24) &0x000000ff;
		/* λͼ�� 4���ֽ� */
		bmp_head[18] = (*pBmpInforHead).biWidth & 0x000000ff;
	  bmp_head[19] = ((*pBmpInforHead).biWidth >> 8) &0x000000ff;
	  bmp_head[20] = ((*pBmpInforHead).biWidth >> 16) &0x000000ff;
	  bmp_head[21] = ((*pBmpInforHead).biWidth >> 24) &0x000000ff;
	  /* λͼ�� 4���ֽ� */
		bmp_head[22] = (*pBmpInforHead).biHeight &0x000000ff;
	  bmp_head[23] = ((*pBmpInforHead).biHeight >> 8) &0x000000ff;
	  bmp_head[24] = ((*pBmpInforHead).biHeight >> 16) &0x000000ff;
	  bmp_head[25] = ((*pBmpInforHead).biHeight >> 24) &0x000000ff;	
	  /* ƽ���� */
		bmp_head[26] = (*pBmpInforHead).biPlanes &0x00ff;
	  bmp_head[27] = ((*pBmpInforHead).biPlanes >> 8) &0x00ff;
	  /* ������ɫλ�� */
		bmp_head[28] = (*pBmpInforHead).biBitCount &0x00ff;
	  bmp_head[29] = ((*pBmpInforHead).biBitCount >> 8) &0x00ff;	
	  /* ѹ����ʽ*/
		bmp_head[30] = (*pBmpInforHead).biCompression &0x000000ff;
	  bmp_head[31] = ((*pBmpInforHead).biCompression >> 8) &0x000000ff;
	  bmp_head[32] = ((*pBmpInforHead).biCompression >> 16) &0x000000ff;
	  bmp_head[33] = ((*pBmpInforHead).biCompression >> 24) &0x000000ff;	
	  /*ʵ��λͼ����ռ�õ��ֽ���*/
		bmp_head[34] = (*pBmpInforHead).biSizeImage &0x000000ff;
	  bmp_head[35] = ((*pBmpInforHead).biSizeImage >> 8) &0x000000ff;
	  bmp_head[36] = ((*pBmpInforHead).biSizeImage >> 16) &0x000000ff;
	  bmp_head[37] = ((*pBmpInforHead).biSizeImage >> 24) &0x000000ff;	
		/*X����ֱ���	*/
		bmp_head[38] = (*pBmpInforHead).biXPelsPerMeter &0x000000ff;
	  bmp_head[39] = ((*pBmpInforHead).biXPelsPerMeter >> 8) &0x000000ff;
	  bmp_head[40] = ((*pBmpInforHead).biXPelsPerMeter >> 16) &0x000000ff;
	  bmp_head[41] = ((*pBmpInforHead).biXPelsPerMeter >> 24) &0x000000ff;	
		/*Y����ֱ���	*/
		bmp_head[42] = (*pBmpInforHead).biYPelsPerMeter &0x000000ff;
	  bmp_head[43] = ((*pBmpInforHead).biYPelsPerMeter >> 8) &0x000000ff;
	  bmp_head[44] = ((*pBmpInforHead).biYPelsPerMeter >> 16) &0x000000ff;
	  bmp_head[45] = ((*pBmpInforHead).biYPelsPerMeter >> 24) &0x000000ff;		
		/*ʹ�õ���ɫ��	*/
		bmp_head[46] = (*pBmpInforHead).biClrUsed &0x000000ff;
	  bmp_head[47] = ((*pBmpInforHead).biClrUsed >> 8) &0x000000ff;
	  bmp_head[48] = ((*pBmpInforHead).biClrUsed >> 16) &0x000000ff;
	  bmp_head[49] = ((*pBmpInforHead).biClrUsed >> 24) &0x000000ff;				
		/*��Ҫ��ɫ��	*/
		bmp_head[50] = (*pBmpInforHead).biClrImportant &0x000000ff;
	  bmp_head[51] = ((*pBmpInforHead).biClrImportant >> 8) &0x000000ff;
	  bmp_head[52] = ((*pBmpInforHead).biClrImportant >> 16) &0x000000ff;
	  bmp_head[53] = ((*pBmpInforHead).biClrImportant >> 24) &0x000000ff;			
}
/*
 * ����bmpͼƬ, 24λ���ɫ
 * ͼƬ��Ⱥ͸߶ȸ���ͼƬ��С����
 */
void bmp_sd_send(unsigned char *pic_name)
{
	int i, j;
	int width, height;
	BITMAPFILEHEADER bitHead;
	BITMAPINFOHEADER bitInfoHead;
	WORD fileType;
  uint16_t packet_length;
	unsigned int read_num;
	unsigned char tmp_name[20];
	sprintf((char*)tmp_name,"0:%s",pic_name);  	
		/* ���ļ�*/
	f_mount(0, &bmpfs[0]);
	bmpres = f_open( &bmpfsrc , (char *)tmp_name, FA_OPEN_EXISTING | FA_READ);	
/*-------------------------------------------------------------------------------------------------------*/
	if(bmpres == FR_OK)
	{

		/* ��ȡ�ļ�ͷ��Ϣ  �����ֽ�*/         
		f_read(&bmpfsrc,&fileType,sizeof(WORD),&read_num);     

		/* �ж��ǲ���bmp�ļ� "BM"*/
//		if(fileType != 0x4d42)
//		{
//			BMP_DEBUG_PRINTF("file is not .bmp file!\r\n");
//			return;
//		}      

		/* ��ȡBMP�ļ�ͷ��Ϣ*/
		f_read(&bmpfsrc,&bitHead,sizeof(tagBITMAPFILEHEADER),&read_num);        
		/* ��ȡλͼ��Ϣͷ��Ϣ */
		f_read(&bmpfsrc,&bitInfoHead,sizeof(BITMAPINFOHEADER),&read_num);        
		headerdeal(&bitHead,&bitInfoHead);
		/* ���Ͱ�������������ݰ����� */
		packet_length=Tramsmit_packet(pack_head,bmp_head,dest_address,54);
		
		/*����ͷ�ļ��� */
   for(i=0;i<packet_length;i++)	   
	  {
		   BMP_DEBUG_PRINTF("%c",pack_head[i]);
		}
 	 while(usart_flag!=0x73);  //�ȴ�Э���ڵ�ظ������־λ,���ƴ����ٶ�
		usart_flag=0;
	}    
	else
	{
		BMP_DEBUG_PRINTF("file open fail!\r\n");
		return;
	}    
/*-------------------------------------------------------------------------------------------------------*/
	width = bitInfoHead.biWidth;
	height = bitInfoHead.biHeight;

	/* ����λͼ��ʵ�ʿ�Ȳ�ȷ����Ϊ32�ı���	*/
//	l_width = WIDTHBYTES(width* bitInfoHead.biBitCount);	
  width=width/8;
	height=height*8;
//	if(l_width > 960)
//	{
//		BMP_DEBUG_PRINTF("\n SORRY, PIC IS TOO BIG (<=320)\n");
//		return;
//	}		
	/* �ж��Ƿ���24bit���ɫͼ */
	if(bitInfoHead.biBitCount >= 24)
	{
		for(i=0;i< height; i++)
		{	  
			/* ��ȡһ��bmp�����ݵ�����bmp_data���� */
			f_read(&bmpfsrc,bmp_data,width*3,&read_num);
			
//			for(j=0; j<width; j++) 											   //һ����Ч��Ϣ
//			{
//				k = j*3;																	 //һ���е�K�����ص����
//				red = bmp_data[k+2];
//				green = bmp_data[k+1];
//				blue = 	bmp_data[k];
//			}    
		  packet_length=Tramsmit_packet(pack_data,bmp_data,dest_address,120);   //�����ZigBee��������֡
			for(j=0;j<packet_length;j++)
			{
				BMP_DEBUG_PRINTF("%c",pack_data[j]); //�������ݰ�
			}
				while(usart_flag!=0x74);  //�ȴ�Э���ڵ�ظ������־λ�����ƴ����ٶ�
			  usart_flag=0;
		}
	}    
	else 
	{        
		BMP_DEBUG_PRINTF("SORRY, THIS PIC IS NOT A 24BITS REAL COLOR");
		return ;
	}
	f_close(&bmpfsrc);   	
}

void bmp_sd_save(unsigned short int Width,unsigned short int Height,unsigned char *filename)
{
	uint16_t i,j;
	uint16_t Camera_Data;		
	long file_size;     
	long width;
	long height;
	unsigned char r,g,b;	
	unsigned int mybw;
	
	/* ��*�� +������ֽ� + ͷ����Ϣ */
	file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + 54;		
		/* �ļ���С 4���ֽ� */
	bmp_head[2] = (unsigned char)(file_size &0x000000ff);
	bmp_head[3] = (file_size >> 8) & 0x000000ff;
	bmp_head[4] = (file_size >> 16) & 0x000000ff;
	bmp_head[5] = (file_size >> 24) & 0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	width=Width;	
	bmp_head[18] = width & 0x000000ff;
	bmp_head[19] = (width >> 8) &0x000000ff;
	bmp_head[20] = (width >> 16) &0x000000ff;
	bmp_head[21] = (width >> 24) &0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	height = Height;
	bmp_head[22] = height &0x000000ff;
	bmp_head[23] = (height >> 8) &0x000000ff;
	bmp_head[24] = (height >> 16) &0x000000ff;
	bmp_head[25] = (height >> 24) &0x000000ff;	

	/* ע��һ�����������߼���Ϊ0 */
	f_mount(0, &bmpfs[0]);
	   bmpres = f_open( &bmpfsrc , (char*)filename,  FA_OPEN_ALWAYS | FA_WRITE);
	if ( bmpres == FR_OK )
	{    
		/* ��Ԥ�ȶ���õ�bmpͷ����Ϣд���ļ����� */
		bmpres = f_write(&bmpfsrc, bmp_head,sizeof(unsigned char)*54, &mybw);		
		
		/* �����ǽ�ָ�����ڵ����ݶ�����д���ļ�����ȥ */
			for(i=0; i<240;i++)
				{
					for(j=0;j<320;j++)
					{
						READ_FIFO_PIXEL(Camera_Data);		/* ��FIFO����һ��rgb565���ص�Camera_Data���� */		
            r =  GETR_FROM_RGB16(Camera_Data);
            g =  GETG_FROM_RGB16(Camera_Data);
            b =  GETB_FROM_RGB16(Camera_Data);		

            bmpres = f_write(&bmpfsrc, &b,sizeof(unsigned char), &mybw);//���浽�ļ���
            bmpres = f_write(&bmpfsrc, &g,sizeof(unsigned char), &mybw);
            bmpres = f_write(&bmpfsrc, &r,sizeof(unsigned char), &mybw);						
					}			
				}
	}
	f_close(&bmpfsrc); 
}


void Lcd_show_bmp(unsigned short int x, unsigned short int y,unsigned char *pic_name)
{
	int i, j, k;
	int width, height, l_width;

	BYTE red,green,blue;
	BITMAPFILEHEADER bitHead;
	BITMAPINFOHEADER bitInfoHead;
	WORD fileType;

	unsigned int read_num;
	unsigned char tmp_name[20];
	sprintf((char*)tmp_name,"0:%s",pic_name);
  f_mount(0, &bmpfs[0]);  
	bmpres = f_open( &bmpfsrc , (char *)tmp_name, FA_OPEN_EXISTING | FA_READ);	
/*-------------------------------------------------------------------------------------------------------*/
	if(bmpres == FR_OK)
	{
		/* ��ȡ�ļ�ͷ��Ϣ  �����ֽ�*/         
		f_read(&bmpfsrc,&fileType,sizeof(WORD),&read_num); 
		/* ��ȡBMP�ļ�ͷ��Ϣ*/
		f_read(&bmpfsrc,&bitHead,sizeof(tagBITMAPFILEHEADER),&read_num);        
		/* ��ȡλͼ��Ϣͷ��Ϣ */
		f_read(&bmpfsrc,&bitInfoHead,sizeof(BITMAPINFOHEADER),&read_num);	
	}    
	else
	{
		return;
	}    
/*-------------------------------------------------------------------------------------------------------*/
	width = bitInfoHead.biWidth;
	height = bitInfoHead.biHeight;

	/* ����λͼ��ʵ�ʿ�Ȳ�ȷ����Ϊ32�ı���	*/
	l_width = WIDTHBYTES(width* bitInfoHead.biBitCount);	

	if(l_width > 960)
	{
		return;
	}
	
	/* ����Lcd Gram ɨ�跽��Ϊ: ���½�->���Ͻ� */
	Lcd_GramScan( 3 );
	
	/* ��һ��ͼƬ��С�Ĵ���*/
	LCD_OpenWindow(x, y, width, height);
	
	/* �ж��Ƿ���24bit���ɫͼ */
	if(bitInfoHead.biBitCount >= 24)
	{
		for(i=0;i< height; i++)
		{	  
			/* ��ȡһ��bmp�����ݵ�����pColorData���� */
			#if 0
				for(j=0; j< l_width; j++)	 													
				{                
					f_read(&bmpfsrc,pColorData+j,1,&read_num);
				}            
			#elif 1				
				f_read(&bmpfsrc,pColorData,l_width/2,&read_num);
				f_read(&bmpfsrc,pColorData+l_width/2,l_width/2,&read_num);
			#else
				f_read(&bmpfsrc,pColorData,l_width,&read_num);
			#endif

			for(j=0; j<width; j++) 											   //һ����Ч��Ϣ
			{
				k = j*3;																	 //һ���е�K�����ص����
				red = pColorData[k+2];
				green = pColorData[k+1];
				blue = 	pColorData[k];
				LCD_WR_Data(RGB24TORGB16(red,green,blue)); //д��LCD-GRAM
				//BMP_DEBUG_PRINTF((char*)RGB24TORGB16(red,green,blue));  //CSA���ڴ���ͼ������
			}            
		}        	 																					     
	}    
	else 
	{        
	//	BMP_DEBUG_PRINTF("SORRY, THIS PIC IS NOT A 24BITS REAL COLOR");
		return ;
	}
	f_close(&bmpfsrc);   
	
}
/* -------------------------------------end of file--------------------------------- */


#include "include.h"
GPIO_InitTypeDef gpio_init_struct; //配置touch中断引脚
/******************************************************************************
* Function Name  : TP_INT_Config
* Description    : Capacitive touch screen configuration //配置touch中断引脚
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
void TP_INT_Config(void)
{
  // 配置 PTE9 为GPIO功能,输入,内部上拉，上升沿产生中断
  gpio_init_struct.GPIO_PTx = PTC;      //PORTB
  gpio_init_struct.GPIO_Pins = GPIO_Pin14;     //引脚8
  gpio_init_struct.GPIO_Dir = DIR_INPUT;        //输入
  gpio_init_struct.GPIO_PinControl = INPUT_PULL_UP|IRQC_FA;     //内部上拉|下降沿中断
  gpio_init_struct.GPIO_Isr = portb_isr;        //中断函数
  LPLD_GPIO_Init(gpio_init_struct);
  //使能中断
  LPLD_GPIO_EnableIrq(gpio_init_struct);
}

void portb_isr()
{
  ctp_dev.tpsta|=0X80;//标记有效                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       的触摸	
}

_m_ctp_dev ctp_dev;	 

_m_ctp_dev ctp_dev1,ctp_dev2,ctp_dev3,ctp_dev4,ctp_dev5;

// touch screen configuration parameter (touch screen manufacturers provide)  不必设置

const uint8 GTP_CFG_DATA[]=
{
  0x12,0x10,0x0E,0x0C,0x0A,0x08,0x06,0x04,0x02,0x00,0x05,0x55,0x15,0x55,0x25,0x55,
  0x35,0x55,0x45,0x55,0x55,0x55,0x65,0x55,0x75,0x55,0x85,0x55,0x95,0x55,0xA5,0x55,
  0xB5,0x55,0xC5,0x55,0xD5,0x55,0xE5,0x55,0xF5,0x55,0x1B,0x03,0x00,0x00,0x00,0x13,
  0x13,0x13,0x0F,0x0F,0x0A,0x50,0x30,0x05,0x03,0x64,0x05,0xe0,0x01,0x20,0x03,0x00,
  0x00,0x32,0x2C,0x34,0x2E,0x00,0x00,0x04,0x14,0x22,0x04,0x00,0x00,0x00,0x00,0x00,
  0x20,0x14,0xEC,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x30,
  0x25,0x28,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x01, 
};	 

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected GT811 register.
* Input          : - reg: address of the selected register.
*                  - buf: Need to write the BUF pointer.
*                  - len: The length of the array
* Output         : None
* Return         : ret
-0  succeed
-1  error
* Attention		 : None
*******************************************************************************/
uint8 GT811_WR_Reg(uint16 reg,uint8 *buf,uint8 len)
{
  uint8 i;
  uint8 ret=0;
  CT_I2C_Start();	
  CT_I2C_Send_Byte(CT_CMD_WR);   
  CT_I2C_Wait_Ack();
  CT_I2C_Send_Byte(reg>>8);   	
  CT_I2C_Wait_Ack(); 	 										  		   
  CT_I2C_Send_Byte(reg&0XFF);   	
  CT_I2C_Wait_Ack();  
  for(i=0;i<len;i++)
  {	   
    CT_I2C_Send_Byte(buf[i]);  
    ret=CT_I2C_Wait_Ack();
    if(ret)break;  
  }
  CT_I2C_Stop();					
  return ret; 
}

/*******************************************************************************
* Function Name  : GT811_RD_Reg
* Description    : Writes to the selected GT811 register.
* Input          : - reg: address of the selected register.
*                  - buf: Need to read the BUF pointer.
*                  - len: The length of the array
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/		  
void GT811_RD_Reg(uint16 reg,uint8 *buf,uint8 len)
{
  uint8 i;
  CT_I2C_Start();	
  CT_I2C_Send_Byte(CT_CMD_WR);  
  CT_I2C_Wait_Ack();
  CT_I2C_Send_Byte(reg>>8);   
  CT_I2C_Wait_Ack(); 	 										  		   
  CT_I2C_Send_Byte(reg&0XFF);   	
  CT_I2C_Wait_Ack();  
  CT_I2C_Start();  	 	   
  CT_I2C_Send_Byte(CT_CMD_RD);     
  CT_I2C_Wait_Ack();	   
  for(i=0;i<len;i++)
  {	   
    buf[i]=CT_I2C_Read_Byte(i==(len-1)?0:1); 
  } 
  CT_I2C_Stop();   
}

/*******************************************************************************
* Function Name  : GT811_Send_Cfg
* Description    : Writes to the selected GT811 register.
* Input          : - buf: Need to write the BUF pointer.
*                  - len: The length of the array
* Output         : None
* Return         : ret
-0  succeed
-1  error
* Attention		 : None
*******************************************************************************/
uint8 GT811_Send_Cfg(uint8 * buf,uint16 cfg_len)
{
  uint8 ret=0;
  uint8 retry=0;
  for(retry=0;retry<5;retry++)
  {
    ret=GT811_WR_Reg(CT_CONFIG_REG,buf,cfg_len);
    if(ret==0)break;
    touch_delay();	 
  }
  return ret;
}


/*******************************************************************************
* Function Name  : GT811_Init
* Description    : GT811 initialization
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
uint8 GT811_Init(void)
{
  uint16 version=0;
  uint8 temp;
  
  CT_CLK_Init();
  CT_SDA_IN();
  //I2C initialization
  TP_INT_Config();
  GT811_RD_Reg(0X717,&temp,1);				//Read version of eight
  version=(uint16)temp<<8; 
  GT811_RD_Reg(0X718,&temp,1);				//Low read version 8
  version|=temp;
  //printf("version:%x\r\n",version); 
  if(version==0X2010)							//Version is correct, send the configuration parameters
  { 
    temp=GT811_Send_Cfg((uint8*)GTP_CFG_DATA,sizeof(GTP_CFG_DATA));
  }else temp=2;	//wrong edition
  return temp; 
}

/*******************************************************************************
* Function Name  : GT811_Scan
* Description    : If there is a touch scanning
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/						   									 
void GT811_Scan(void)
{			   
  uint8 buf[34];
  if((ctp_dev.tpsta&0X80)==0)return;		   	 
  GT811_RD_Reg(CT_READ_XY_REG,buf,34);	     
  ctp_dev.tpsta=buf[0]&0X1F;					       
#if CT_EXCHG_XY==1								            
  ctp_dev.y[0]=480-(((uint16)buf[2]<<8)+buf[3]);	//Touch point 1 coordinates
  ctp_dev.x[0]=((uint16)buf[4]<<8)+buf[5];	  
  ctp_dev.ppr[0]=buf[6];	 
  ctp_dev.y[1]=480-(((uint16)buf[7]<<8)+buf[8]);	//Touch point 2 coordinates
  ctp_dev.x[1]=((uint16)buf[9]<<8)+buf[10];	  
  ctp_dev.ppr[1]=buf[11];
  ctp_dev.y[2]=480-(((uint16)buf[12]<<8)+buf[13]);//Touch point 3 coordinates
  ctp_dev.x[2]=((uint16)buf[14]<<8)+buf[15];	  
  ctp_dev.ppr[2]=buf[16];					   
  ctp_dev.y[3]=480-(((uint16)buf[17]<<8)+buf[24]);//Touch point 4 coordinates
  ctp_dev.x[3]=((uint16)buf[25]<<8)+buf[26];	  
  ctp_dev.ppr[3]=buf[27];
  ctp_dev.y[4]=480-(((uint16)buf[28]<<8)+buf[29]);//Touch point 5 coordinates
  ctp_dev.x[4]=((uint16)buf[30]<<8)+buf[31];	  
  ctp_dev.ppr[4]=buf[32];
#else 
  ctp_dev.y[0]=((uint16)buf[2]<<8)+buf[3];		//Touch point 1 coordinates
  ctp_dev.x[0]=800-(((uint16)buf[4]<<8)+buf[5]);	  
  ctp_dev.ppr[0]=buf[6];	 
  ctp_dev.y[1]=((uint16)buf[7]<<8)+buf[8];		//Touch point 2 coordinates
  ctp_dev.x[1]=800-(((uint16)buf[9]<<8)+buf[10]);	  
  ctp_dev.ppr[1]=buf[11];
  ctp_dev.y[2]=((uint16)buf[12]<<8)+buf[13];		//Touch point 3 coordinates
  ctp_dev.x[2]=800-(((uint16)buf[14]<<8)+buf[15]);	  
  ctp_dev.ppr[2]=buf[16];					   
  ctp_dev.y[3]=((uint16)buf[17]<<8)+buf[24];		//Touch point 4 coordinates
  ctp_dev.x[3]=800-(((uint16)buf[25]<<8)+buf[26]);	  
  ctp_dev.ppr[3]=buf[27];
  ctp_dev.y[4]=((uint16)buf[28]<<8)+buf[29];		//Touch point 5 coordinates
  ctp_dev.x[4]=800-(((uint16)buf[30]<<8)+buf[31]);	  
  ctp_dev.ppr[4]=buf[32];
#endif	 		    
}


_lcd_dev lcddev;
uint16 CT_Color_table[]={White,Magenta,Yellow,Blue,Cyan,Yellow};

/*******************************************************************************
* Function Name  : LCD_Draw_Coordinates
* Description    : None
* Input          : - x0
*                  - y0
*                  - color:According to the color
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/	
void LCD_Draw_Coordinates( uint16_t x0, uint16_t y0,uint16_t color)
{
  Text_color(color); 
  Circle_Coordinate_Radius(x0,479-y0,20);	
  Draw_circle_fill();	
  
  LCD_DrawLine(0,479-y0,790,479-y0,color);	
  LCD_DrawLine(0,478-y0,790,478-y0,color);	
  
  LCD_DrawLine(x0,0,x0,479,color);	
  LCD_DrawLine(x0+1,0,x0+1,479,color);
  Chk_Busy(); 
}
/*******************************************************************************
* Function Name  : LCD_Draw_Coordinates
* Description    : None
* Input          : - x0
*                  - y0
*                  - color:According to the color
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/	
void LCD_Clear_Coordinates(uint16_t ctout)
{
  uint8 i=0;
  LCD_DrawSquare(15,30,210,350,Black,1);
  
  switch (ctout)
  {
  case 1:
    LCD_Draw_Coordinates(ctp_dev1.x[i],ctp_dev1.y[i],Black);
    break;
  case 3:					
    for(i=0;i<2;i++)
      LCD_Draw_Coordinates(ctp_dev2.x[i],ctp_dev2.y[i],Black);
    break;
  case 7:
    for(i=0;i<3;i++)
      LCD_Draw_Coordinates(ctp_dev3.x[i],ctp_dev3.y[i],Black);
    break;
  case 0x0f:			
    for(i=0;i<4;i++)
      LCD_Draw_Coordinates(ctp_dev4.x[i],ctp_dev4.y[i],Black);
    break;
  case 0x1f:
    for(i=0;i<5;i++)
      LCD_Draw_Coordinates(ctp_dev5.x[i],ctp_dev5.y[i],Black);
    break;
  default:
    
    break;
  }	
}
/*******************************************************************************
* Function Name  : LCD_Draw_Coordinates
* Description    : None
* Input          : - x0
*                  - y0
*                  - x_vlue
*                  - y_vlue
*                  - color
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/	
void LCD_Display_Coordinates(uint16_t x0, uint16_t y0, uint16_t x_vlue, uint16_t y_vlue, uint16_t color)
{
  uint8 j=0,h1,h2,h3;
  LCD_DrawSquare(0,0,10,479,Blue,1);
  LCD_DrawSquare(0,469,899,479,Blue,1);
  LCD_DrawSquare(0,469,8,32,Blue,1);
  LCD_DrawSquare(789,0,899,479,Blue,1);
  
  LCD_DrawSquare(0,0,200,32,Blue,1);
  //	LCD_DrawSquare(200,0,250,32,Blue,1);
  LCD_DrawSquare(648,0,799,32,Blue,1);
  
  LCD_Internal_Font_ROM(200,0,White,Blue,1,0,2,0);
  Show_String("Capacitive touch screen test",1);
  
  LCD_Internal_Font_ROM(x0+15,y0+32,color,Black,0,0,0,0);
  Show_String("coordinates ",1);
  
  Show_String("x:",1);
  h1=x_vlue/100+0x30;
  h2=x_vlue%100/10+0x30;
  h3=x_vlue%10+0x30;
  Show_String("aa",1);
  //Show_String(&h2,1);
  //Show_String(&h3,1);
  Show_String("  y:",1);
  h1=y_vlue/100+0x30;
  h2=y_vlue%100/10+0x30;
  h3=y_vlue%10+0x30;
  Show_String(&h1,1);
  Show_String(&h2,1);
  Show_String(&h3,1);
  
}

/*******************************************************************************
* Function Name  : CT_TEST
* Description    : Capacitance screen test
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/	
void CT_TEST(void)
{
  //int flag = 0;
  
  uint8 	ctout;			
  uint8 i=0,t=0;
  Text_color(Red); 
  while( GT811_Init() )		
  {
    //LCD_Internal_Font_ROM(0,0,Black,White,0,0,4,0);
    //Show_String("GT811 initialization OK",1);
  }
  
  Geometric_Coordinate(16,16,800,479);
  while(1)
  {
    GT811_Scan();
    
    if(ctp_dev.tpsta&0X1F)		//Touch screen is pressed
    {	
      if(ctp_dev.tpsta&(1<<t))
      {			
        switch (ctp_dev.tpsta)
        {
        case 1:
          if(ctout != 1)						
            LCD_Clear_Coordinates(ctout);
          for(i=0;i<1;i++)
          {
            
            //if(!((ctp_dev1.x[i] ==ctp_dev.x[i]) && (ctp_dev1.y[i] == ctp_dev.y[i])))
            //LCD_Draw_Coordinates(ctp_dev1.x[i],ctp_dev1.y[i],Black);
            
            ctp_dev1.x[i]=ctp_dev.x[i]; 
            ctp_dev1.y[i]=ctp_dev.y[i]; 	
            LCD_Display_Coordinates(0,0,ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);									
            //LCD_Draw_Coordinates(ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);
            
          }	
          ctout =1;
          break;
        case 3:
          
          if(ctout != 3)
            LCD_Clear_Coordinates(ctout);
          for(i=0;i<2;i++)
          {
            if(!((ctp_dev2.x[i] == ctp_dev.x[i]) && (ctp_dev2.y[i] == ctp_dev.y[i])))
              LCD_Draw_Coordinates(ctp_dev2.x[i],ctp_dev2.y[i],Black);				
            ctp_dev2.x[i]=ctp_dev.x[i]; 
            ctp_dev2.y[i]=ctp_dev.y[i]; 	
            LCD_Display_Coordinates(0,0+16*i,ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);										
            LCD_Draw_Coordinates(ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);										
          }
          ctout =3;
          break;
        case 7:
          if(ctout != 7)
            LCD_Clear_Coordinates(ctout);
          for(i=0;i<3;i++)
          {
            
            if(!((ctp_dev3.x[i] ==ctp_dev.x[i]) && (ctp_dev3.y[i] == ctp_dev.y[i])))
              LCD_Draw_Coordinates(ctp_dev3.x[i],ctp_dev3.y[i],Black);					
            ctp_dev3.x[i]=ctp_dev.x[i]; 
            ctp_dev3.y[i]=ctp_dev.y[i]; 
            LCD_Display_Coordinates(0,0+16*i,ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);	
            LCD_Draw_Coordinates(ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);
          }							
          ctout =7;
          break;
        case 0x0f:
          if(ctout != 0x0f)
            LCD_Clear_Coordinates(ctout);
          for(i=0;i<4;i++)
          {
            
            if(!((ctp_dev4.x[i] ==ctp_dev.x[i]) && (ctp_dev4.y[i] == ctp_dev.y[i])))
              LCD_Draw_Coordinates(ctp_dev4.x[i],ctp_dev4.y[i],Black);					
            ctp_dev4.x[i]=ctp_dev.x[i]; 
            ctp_dev4.y[i]=ctp_dev.y[i]; 
            LCD_Display_Coordinates(0,0+16*i,ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);	
            LCD_Draw_Coordinates(ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);
          }										
          
          ctout =0x0f;
          break;
        case 0x1f:
          if(ctout != 0x1f)
            LCD_Clear_Coordinates(ctout);
          for(i=0;i<5;i++)
          {
            
            if(!((ctp_dev5.x[i] ==ctp_dev.x[i]) && (ctp_dev5.y[i] == ctp_dev.y[i])))
              LCD_Draw_Coordinates(ctp_dev5.x[i],ctp_dev5.y[i],Black);					
            ctp_dev5.x[i]=ctp_dev.x[i]; 
            ctp_dev5.y[i]=ctp_dev.y[i];
            LCD_Display_Coordinates(0,0+16*i,ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);	
            LCD_Draw_Coordinates(ctp_dev.x[i],ctp_dev.y[i],CT_Color_table[i]);
          }	
          ctout =0x1f;
          break;
          
        default:
          
          break;
        }					
      }
      Key_Scan();
    }
    
  }
  
}


#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

uint8_t x, y, z;
uint8_t	x_address = 0x29, y_address = 0x2B, z_address = 0x2D; //sensor datasht syf 16 dan yazdik bu degerleri

void GPIO_Config()
{
	GPIO_InitTypeDef	GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOD,&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOE ,& GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA , GPIO_PinSource6 , GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA , GPIO_PinSource7 , GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA , GPIO_PinSource8 , GPIO_AF_SPI1);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

void SPI_Config()
{
	SPI_InitTypeDef		SPI_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;	// SPI frequency is APB2 frequency / 2

	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge; // data sampled at two edge (sample = ornekleme , deneme)

	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;// clock is high when idle (idle = bosta) //bosta iken saat 1 seviyesinde

	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;// one packet of data is 8 bits wide (wide = genisliginde , genislik,uzunluk)

	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines (seperate = ayirmak , bolmek)
	/* tek yolu yada cift yonlu hem okuma hem de yazma yapabiliriz */

	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB; // data is transmitted MSB first , yuksek oncelikli yapmýs olduk

	SPI_InitStruct.SPI_Mode = SPI_Mode_Master; // transmit in master mode, NSS pin has to be always high

	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high (managemet = yonetim  , pull = cekmek ,asilmak)
	/* NSS sinyali yazilim tarafindan yonetilmesini sagladik */
	SPI_Init(SPI1,&SPI_InitStruct);
	SPI_Cmd(SPI1,ENABLE);

	GPIO_SetBits(GPIOE , GPIO_Pin_3);
}

void SPI_Write(uint8_t address , uint8_t data) /* SPI a data yazdigimiz fonksiyon */
{
	GPIO_ResetBits(GPIOE , GPIO_Pin_3); //spý uzerinden data gonderirken e pinini low konumuna getirmeliyiz

	while(!SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)); // data gonderme(TXE) flag inin 1 olup olmadigini kontrol ediyoruz

	SPI_I2S_SendData(SPI1 , address); // onde adress datasi gonderiyoruz , yani once adresimizi belirlemeliyiz.

	while(! SPI_I2S_GetFlagStatus(SPI1 , SPI_I2S_FLAG_RXNE)); // bana bir data geliryor mu flag ini kontrol ediyoruz,yani data almaya hazir mi bakiyoruz

	SPI_I2S_ReceiveData(SPI1);	//spý1 den data aliyoruz

	while(!SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE));

	SPI_I2S_SendData(SPI1 , data); // bu sefer data bilgimizi gonderiyoruz

	while(! SPI_I2S_GetFlagStatus(SPI1 , SPI_I2S_FLAG_RXNE));

	SPI_I2S_ReceiveData(SPI1);

	GPIO_SetBits(GPIOE , GPIO_Pin_3); // gonderme islemlerimiz bittigi icin tekrardan aktif yapiyoruz.
}

uint8_t SPI_Read(uint8_t address , uint8_t data)
{
	GPIO_ResetBits(GPIOE , GPIO_Pin_3); // okuma ve yazma yaparken sensor uzerindeki cs pinini reset etmemiz gerejkmektedir
	/*AN2335 Sensor datashet de syf 15 de bu bilgi yazmaktadir*/

	address = address | 0x80; //okuma islemi yapilacak

	while(!SPI_I2S_GetFlagStatus(SPI1 , SPI_I2S_FLAG_TXE));

	SPI_I2S_SendData(SPI1 , address);

	GPIO_SetBits(GPIOE , GPIO_Pin_3);

	return SPI_I2S_ReceiveData(SPI1); // SPI1 icerisinde bulunan degeri geri donderiyoruz.

}

int main(void)
{
	GPIO_Config();
	SPI_Config();
	SPI_Write(0x20 , 0x67); // CTRL_REG 100 Hz +-8g (syf : 18)

  while (1)
  {
	  //x = SPI_Read(x_address , myData);

	  x = SPI_Read(x_address , 0x00);//adresten data okumasi yapiyoruz. data bolumunu 0 yapiyoruz cunku data alacagimiz icin bir manada temizlemis oluyoruz ve her seferinde yeni bir veri geliyor
	  y = SPI_Read(y_address , 0x00);
	  z = SPI_Read(z_address , 0x00);
  }
}


void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
  /* TODO, implement your code here */
  return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
  /* TODO, implement your code here */
  return -1;
}

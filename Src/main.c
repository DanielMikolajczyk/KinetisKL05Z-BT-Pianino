/*-------------------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - projekt
					Instrument muzyczny sterowany z telefonu za pomoca modulu bluetooth
					autor: Adrian Przywara & Daniel Mikolajczyk
					wersja: 27.12.2021r.
----------------------------------------------------------------------------*/
	
#include "MKL05Z4.h"
#include "uart0.h"
#include "frdm_bsp.h"
#include "TPM.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define END	0x21		// Koniec komendy
#define CR	0xd		// Koniec komendy
#define LF	0xa		// Koniec komendy
#define MOD_MAX	40082
#define ZEGAR 1310720
#define DIV_CORE	16
	
uint16_t	mod_curr=MOD_MAX;
uint16_t	ampl;
uint16_t Tony[]={40082, 37826, 35701, 33702, 31813, 30027, 28339, 26748, 25249, 23830, 22493, 21229};
uint16_t	Oktawa[]={1, 2, 4, 8, 16, 32, 64, 128};
uint8_t	ton=0;
int8_t	gama=6;
uint8_t k_curr=5;
uint8_t sekunda=0;	
uint8_t play=0;
char rx_buf[16];
uint8_t rx_buf_pos=0;
char temp;
uint8_t rx_FULL=0;

void SysTick_Handler(void)	// Podprogram obslugi przerwania od SysTick'a
{
	sekunda+=1;
	if(sekunda==4){
		switch(play)
		{
			case 0	:
				TPM0->SC &= ~(TPM_SC_CMOD(3));
				TPM0->CNT=0;
				sekunda=0;
				break;
			case 1	:
				mod_curr=Tony[ton]/Oktawa[gama];
				TPM0->MOD = mod_curr;
				ampl=(int)((int)mod_curr*k_curr)/100;
				TPM0->CONTROLS[3].CnV = ampl;
				TPM0->SC |= TPM_SC_CMOD(1);
				play = 0;
				sekunda=0;
				break;
		}
	}
}

void UART0_IRQHandler()
{
	if(UART0->S1 & UART0_S1_RDRF_MASK)
	{
		temp=UART0->D;	// Odczyt wartosci z bufora odbiornika i skasowanie flagi RDRF
		if(!rx_FULL)
		{
			if(temp!=END)
			{
				rx_buf[rx_buf_pos] = temp;	// Kompletuj komende
				rx_buf_pos++;
			}
			else
			{
				rx_buf[rx_buf_pos] = 0;
				rx_FULL=1;
			}
		}
	NVIC_EnableIRQ(UART0_IRQn);
	}
}

void volume(char* str){
	k_curr = atoi(str);
}

void octave(char* str){
	if(strcmp(str,"up")==0){
		gama+=1;
		if(gama==8) gama=7;
	}
	else if(strcmp(str,"down")==0){
		gama-=1;
		if(gama==(-1)) gama=0;
	}
}

void note(char* str){
	switch (str[0])
  {
  	case 'c':
			ton = 0;
  		break;
  	case 'd':
			ton = 2;
  		break;
  	case 'e':
			ton = 4;
  		break;
  	case 'f':
			ton = 5;
  		break;
  	case 'g':
			ton = 7;
  		break;
  	case 'a':
			ton = 9;
  		break;
  	case 'b':
			ton = 11;
  		break;
  	case 's':
			switch (str[1])
      {
      	case 'c':
					ton = 1;
      		break;
      	case 'd':
					ton = 3;
      		break;
      	case 'f':
					ton = 6;
      		break;
      	case 'g':
					ton = 8;
      		break;
      	case 'a':
					ton = 10;
      		break;
      }
  		break;
  }
	play = 1;
}	
int main(void)
{
	UART0_Init();		// Inicjalizacja portu szeregowego UART0
	PWM_Init();				// Inicjalizacja licznika TPM0 (PWM „Low-true pulses”)
	
	SysTick_Config(SystemCoreClock/DIV_CORE );
		
	while(1)
	{
		if(rx_FULL)		// Czy dana gotowa?
		{
			switch (rx_buf[0])
      {
      	case 'v':
					volume(rx_buf+1);
      		break;
      	case 'o':
					octave(rx_buf+1);
      		break;
				case 'n':
					note(rx_buf+1);
					break;
      	default:
      		break;
      }

			rx_buf_pos=0;
			rx_FULL=0;	// Dana skonsumowana
		}
	}
}
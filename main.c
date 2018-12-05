/*
 * ESP8266 - Arduino Nano / Atmega 328p interrupt networking initialization
 * 
 * Author: Roger Kupari 12/2018
 * 
 * 
 */


#define F_CPU 16000000UL
#define BAUD 9600


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>


// communications to/from ESP and networking settings
char espReady[20] = "IF=Start 2016\r\n";
char espWifiOk[20] = "IF=connected\r\n";
char ssid[20] = "SI:Nk5\n\r";
char password[20] = "PW:testi111\n\r";
char inTopic[20] = "IT:inTopic\n\r"; 
char outTopic[20] = "OT:outTopic\n\r"; 
char clientId[20] =  "CI:ESP8266_89\n\r";
char mqtt_server[20] = "SA:18.195.243.110\n\r";
char channel="11\n";
char cfgReady[20] = "CO\n\r";

char * cfg [7][20] = {espReady, ssid, password, clientId, mqtt_server, outTopic, inTopic};

                    

// Rx buffer and index to checking process rx_buf
char rx_buf[20][20];
int column = 0;
int row = 0;

// Is wififi configured?
bool wifiCfg = false;
// What is the current status of wifi config?
int cfgStatus = 0;

// Is the test words pushed to broker?
bool tested = false;
// Is wifi connection ok?
bool wfiOk = false;

// Receiver buffer
char tx_buf[20];

// Function prototypes
void usart_tx(char data[]);
void wifiSetup();





void setup(){
  //Usart initiliziation
  UCSR0B |= (1<<7) | (1<<4) | (1<<3); // Interrupt, Receiver & transmitter enable
  UCSR0C |= (1<<2) | (1<<1); // Asynchronous, Parity disabled, 1 stop bit, 8 data bit
  UBRR0H = 0x00;
  UBRR0L = ((F_CPU/16)/BAUD)-1; // 9600 baud
  
   
  
}


void loop()
{
    
 
  if(wifiCfg)
  {
    wifiSetup();
    
  }

  if(cfgStatus == 10 && !wifiCfg && !tested && wfiOk)
  {
    sprintf(tx_buf, "ME:testi 5\n\r");
    usart_tx(tx_buf);
    tested = true;
    for(int b = 0; b<4; b++)
    {
      usart_tx(tx_buf);
      _delay_ms(1000);
    }
  }


   

}

void usart_tx(char data[]){

        for(int i = 0; data[i]!='\0'; i++)
        {
          while (!(UCSR0A & (1<<UDRE0)));
          UDR0 = data[i];
        }

        _delay_ms(1000);
        

  
}

ISR(USART_RX_vect)
{
  char data = UDR0;
  rx_buf[row][column] = data;
  column++;

  
  if(data == 0x0A)
  {

    if(strcmp(rx_buf[row], espReady) == 0)
    {
      cfgStatus = 1;
      wifiCfg = true;
      wfiOk =false;
      row = 0;
      column = 0;
    }
    else if(strcmp(rx_buf[row], espWifiOk) == 0)
    {
      _delay_ms(5000);
      wfiOk = true;
      usart_tx(rx_buf[row]);
    }
   
    else
    {
      
      row++;
      column = 0; 
      
    }
   
    
    
  }
 
  
}

void wifiSetup()
{

  switch (cfgStatus)
  {
    case 1: sprintf(tx_buf, "%s", ssid); cfgStatus++; usart_tx(tx_buf); break;
    case 2: sprintf(tx_buf, "%s", password);cfgStatus++; usart_tx(tx_buf); break;
    
    case 3: sprintf(tx_buf, "%s", clientId); cfgStatus++; usart_tx(tx_buf); break;
    case 4: sprintf(tx_buf, "%s", mqtt_server); cfgStatus++; usart_tx(tx_buf); break;
    case 5: sprintf(tx_buf, "%s", outTopic); cfgStatus++; usart_tx(tx_buf); break;
    case 6: sprintf(tx_buf, "%s", inTopic); cfgStatus++; usart_tx(tx_buf); break;
    case 7: sprintf(tx_buf, "%s", cfgReady); cfgStatus++; usart_tx(tx_buf); break;
    case 8: sprintf(tx_buf, "Config ok\n"); cfgStatus = 10; wifiCfg = false; usart_tx(tx_buf); break;

    
  }


  
}

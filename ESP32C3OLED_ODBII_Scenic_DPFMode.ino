//24/09/2025: v1.0 integrazione con ESP32S3 OLED

// libreria BLE BT https://github.com/vdvornichenko/obd-ble-serial 
//espressif v.3.2.1
//abilitare USB CDC On Boot per avere la seriale

#include <Arduino.h>
#include "ELMduino.h"
#include <BLEClientSerial.h>

BLEClientSerial MyBLESerial;

#include <U8g2lib.h>

#define DEBUG_PORT Serial
#define ELM_PORT   MyBLESerial

#define LCDWidth   u8g2.getDisplayWidth()

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Pin Definitions
#define SDA_PIN 5
#define SCL_PIN 6

typedef enum { DPF_STATE,
               DPF_MASS } obd_pid_states;
obd_pid_states obd_state = DPF_STATE;

// Display Configuration
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.42" OLED

uint8_t address[6]  = {0x01, 0x23, 0x45, 0x67, 0x89, 0xBA};

ELM327 myELM327;

bool dpfON = false;
bool alertMsg = false;

int nb_query_state = SEND_COMMAND; // Set the inital query state ready to send a command
int dpfstate = 0;
float massDPF = 0;
float massVal = 0;

byte A, B, C, D;

String msg;
char sMass[10];

bool connected = false;

void setup()
{
  //Inizializzazione display
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  delay(2000);

  String s;
  Serial.begin(115200);
  delay(100);
  
  // Reset Display on first boot
  displayAmbientnode();

  // displayDPF("ON");

  // delay(5000);

  // displayAlert();

  // delay(5000);
  
  // displayDPF("OFF");

  // delay(5000);
  
  // byte ex1 = 5;
  // byte ex2 = 111;
  // char test[10];
  // float total = ((ex1*256)+ex2)/100.0;
  // sprintf(test, "%.2f", total);
  // displayMass(test);

  // delay(5000);
  
  // displayMsg("CONN BT");
  // delay(1000);

  ELM_PORT.begin("ESP32-C3", "FFF0", "FFF1", "FFF2");

  displayMsg("Conn BT");

  //Tentativo di connessione
  while (!connected)
  {
    if (!ELM_PORT.connect("OBDBLE"))  //OBDII
    {
      displayMsg("KO BT!");
      Serial.println("Couldn't connect to OBD scanner - Phase 1");
      delay(10000);
      displayMsg("Conn BT");
    }
    else
      connected = true;
  }

  displayMsg("Conn ELM");

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    displayMsg("KO ELM!");
    Serial.println("Couldn't connect to OBD scanner - Phase 2");
    while (1);
  }

  Serial.println("Connected to ELM327");

  delay(1000);

  displayMsg("OK ELM");
}

void loop()
{
  switch (obd_state)
  {
    case DPF_STATE:
    {
      if (nb_query_state == SEND_COMMAND)         // We are ready to send a new command
      {
          msg = "DPF...";

          // if (!dpfON)
          // {
          //   displayMsg("Stato..");
          // }
      }

      dpfstate = (int)myELM327.processPID(34, 8278, 1, 1, 1, 0);  //22 2056 DPF Mode
      nb_query_state = WAITING_RESP;          // Set the query state so we are waiting for response
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)    // Our response is fully received, let's get our data
      {   
          Serial.print("DPF State: ");   
          Serial.println(dpfstate);   
          
          if (dpfstate != 1)
          {
            dpfON = true;
            alertMsg = !alertMsg;
            //alterna ON a ! per avvisare che è in rigenerazione
            if (alertMsg)
              displayDPF("ON");
            else
              displayAlert();
          }
          else
          {
            dpfON = false;
            alertMsg = false;
            Serial.println("DPF OFF");
            //displayDPF("OFF");
          }
            
          nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
          obd_state = DPF_MASS;
          delay(5000);                            // Wait 2 seconds until we query again
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {                                           // If state == ELM_GETTING_MSG, response is not yet complete. Restart the loop.
          //displayMsg("ERR ECU");
          
          dpfON = false;
          
          nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
          myELM327.printError();
          obd_state = DPF_MASS;
          delay(5000);                            // Wait 2 seconds until we query again
      }
      break;
    }
    case DPF_MASS:
    {
      if (nb_query_state == SEND_COMMAND)         // We are ready to send a new command
      {
          msg = "DPF...";

          // if (!dpfON)
          // {
          //   displayMsg("Massa..");
          // }
      }

      massDPF = myELM327.processPID(34, 9260, 4, 4, 1, 0);  //22 242C DPF Mass
      nb_query_state = WAITING_RESP;          // Set the query state so we are waiting for response
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)    // Our response is fully received, let's get our data
      {   
          Serial.print("DPF Mass: ");   
          Serial.println(massDPF);   
          D = myELM327.responseByte_0;
          C = myELM327.responseByte_1;
          B = myELM327.responseByte_2;
          A = myELM327.responseByte_3;

          massVal = ((C*256)+D)/100.0;
          sprintf(sMass, "%.2f", massVal);
          displayMass(sMass);
          nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
          obd_state = DPF_STATE;
          delay(5000);                            // Wait 2 seconds until we query again
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {                                           // If state == ELM_GETTING_MSG, response is not yet complete. Restart the loop.
          displayMsg("ERR ECU");
          
          nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
          myELM327.printError();
          obd_state = DPF_STATE;
          delay(5000);                            // Wait 2 seconds until we query again
      }
      break;
    }
  }
}

void displayAmbientnode() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(4, 20);
  u8g2.print("INIT");
  u8g2.sendBuffer();
}

void displayDPF(String msg) {
  // int size = msg.length();
  
  // int posx = 35 - size;
  // int posy = 20;
  // if posx < 0
  //   posx = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB18_tr);

  int posx = ((LCDWidth - (u8g2.getStrWidth(msg.c_str()))) / 2);
  int posy = 25;

  Serial.printf("LCDWidth=%i StrLen=%i Posx=%i\n", LCDWidth, u8g2.getStrWidth(msg.c_str()), posx);

  u8g2.setCursor(posx, posy);
  u8g2.print(msg);
  u8g2.sendBuffer();
}

void displayAlert()
{
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawTriangle(36, 2, 2, 38, 70, 38);
  u8g2.sendBuffer();
  u8g2.setDrawColor(0);
  u8g2.drawBox(34, 6, 4, 24);
  u8g2.drawBox(34, 32, 4, 4);
  u8g2.sendBuffer();
  u8g2.setDrawColor(1);
}

void displayMsg(String msg) {
  // int size = msg.length();
  
  // int posx = 35 - size;
  // int posy = 20;
  // if posx < 0
  //   posx = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB12_tr);

  int posx = ((LCDWidth - (u8g2.getStrWidth(msg.c_str()))) / 2);
  int posy = 25;

  Serial.printf("LCDWidth=%i StrLen=%i Posx=%i\n", LCDWidth, u8g2.getStrWidth(msg.c_str()), posx);

  u8g2.setCursor(posx, posy);
  u8g2.print(msg);
  u8g2.sendBuffer();
}

void displayMass(String msg) {
  // int size = msg.length();
  
  // int posx = 35 - size;
  // int posy = 20;
  // if posx < 0
  //   posx = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);

  int posx = ((LCDWidth - (u8g2.getStrWidth(msg.c_str()))) / 2);
  int posy = 25;

  u8g2.setCursor(posx, posy);
  u8g2.print(msg);
  u8g2.sendBuffer();
}

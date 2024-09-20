//06/09/2024: v1 lettura messaggio 22 2056 DPF Mode (1=normale, 5=rigenerazione, 7=attesa) e gestione tramite led

#include "BluetoothSerial.h"
#include "ELMduino.h"

BluetoothSerial SerialBT;
#define ELM_PORT   SerialBT
#define DEBUG_PORT Serial

ELM327 myELM327;

uint32_t rpm = 0;
bool dpfON = false;

int nb_query_state = SEND_COMMAND; // Set the inital query state ready to send a command
int dpfstate = 0;

String msg;

// *********   PINS   *************
#define GPIO_BT 19
#define GPIO_DPF 18

uint8_t address[6]  = {0x00, 0x0D, 0x18, 0x3A, 0x67, 0x89};
//00:0D:18:3A:67:89 Vecchio OBDII

void setup()
{
  pinMode(GPIO_BT, OUTPUT);
  pinMode(GPIO_DPF, OUTPUT);


//digitalWrite(GPIO_BT, HIGH);
//digitalWrite(GPIO_DPF, HIGH);

delay(5000);

  String s;
  Serial.begin(115200);
  delay(100);
  
  //DEBUG_PORT.begin(115200);
  //ELM_PORT.setPin("1234");
  ELM_PORT.begin("ArduHUD", true);

  for (int i =0; i < 10; i++)
  {
    digitalWrite(GPIO_BT, !digitalRead(GPIO_BT));
    delay(100);
  }
  digitalWrite(GPIO_BT, LOW);
  
  if (!ELM_PORT.connect(address))  //OBDII
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 1");
    while(1);
  }

  for (int i =0; i < 5; i++)
  {
    digitalWrite(GPIO_BT, !digitalRead(GPIO_BT));
    delay(100);
  }

  digitalWrite(GPIO_BT, LOW);

  if (!myELM327.begin(ELM_PORT, true, 2000))
  {
    Serial.println("Couldn't connect to OBD scanner - Phase 2");
    while (1);
  }

  digitalWrite(GPIO_BT, HIGH);

  Serial.println("Connected to ELM327");
}

void loop()
{
    if (nb_query_state == SEND_COMMAND)         // We are ready to send a new command
    {
        msg = "DPF...";

        if (!dpfON)
        {
          digitalWrite(GPIO_DPF, HIGH);
          delay(100);
          digitalWrite(GPIO_DPF, LOW);
        }
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
          digitalWrite(GPIO_DPF, HIGH);
          Serial.println("DPF ON");   
        }
        else
        {
          dpfON = false;
          digitalWrite(GPIO_DPF, LOW);
          Serial.println("DPF OFF");   
        }
          
        nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
        delay(2000);                            // Wait 2 seconds until we query again
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {                                           // If state == ELM_GETTING_MSG, response is not yet complete. Restart the loop.
        digitalWrite(GPIO_DPF, LOW);
        dpfON = false;
        
        nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
        myELM327.printError();
        delay(2000);                            // Wait 2 seconds until we query again
    }
}

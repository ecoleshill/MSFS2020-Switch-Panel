/*****************************************************************************
 * Date:  Jan 9, 2023
 * Author: (James) Elliott Coleshill
 * Puurpose:  The purpose of this sketch is to provide control logic for 20
 *            digital switches on my MSFS2020 home simulator.   The software
 *            Monitors the values of the siwtches and sends the corresponding
 *            PRESS/RELEASE SPAD.next commands.
 *            
 *            This code is design to work inconjunction with the open source
 *            vJoy controller software.   Thus, SPAD.next must have a 20 switch
 *            vJoy controller configured for the aircraft being operated
 *****************************************************************************/
#include <CmdMessenger.h>   //SPAD.next serial command messenger handler

// This is the list of recognized commands. These can be commands that can either be sent
// or received.
// In order to receive, attach a callback function to these events
enum
{
  kRequest = 0, // Request from SPAD.neXt
  kCommand = 1, // Command to SPAD.neXt
  kEvent = 2, // Events from SPAD.neXt
  kDebug = 3, // Debug strings to SPAD.neXt Logfile
  kSimCommand = 4, // Send Event to Simulation
  kLed = 10, // CMDID for exposed data to SPAD.neXt
  kHeading = 11, // CMDID for data updates from SPAD.neXt
};

//This structure is used to keep track of the state and configuration of the
//20 physical switches wired into the panel
struct Switch
{
  int Number;                     //Digital Switch Number
  String Str = "";                //The string name for the button on the vJoy
  unsigned short int State;       //Current State of the Switch
};
Switch mySwitches[20];            //The 20 switches configuration and state

CmdMessenger messenger(Serial);   //Listens on serial connection for messages from Spad.next

bool isReady = false;

//This function is used to attached funtions with command ID callbacks from Spad.next
void attachCommandCallbacks()
{
  // Attach callback methods
  messenger.attach(onUnknownCommand);
  messenger.attach(kRequest  , onIdentifyRequest);
}

// ------------------- CALLBACK FUNCTIONS ----------------------------- //

// Called when a received command has no attached function
void onUnknownCommand()
{
  messenger.sendCmd(kDebug,"UNKNOWN COMMAND");
}

// Callback function to respond to indentify request. This is part of the
// Auto connection handshake.
void onIdentifyRequest()
{
  char *szRequest = messenger.readStringArg();

  if (strcmp(szRequest, "INIT") == 0) {
    messenger.sendCmdStart(kRequest);
    messenger.sendCmdArg("SPAD");
    // Unique Device ID: Change this!
    messenger.sendCmdArg(F("{abff8305-1ee6-4c4c-bccc-b271b53d03b3}"));
    // Device Name for UI
    messenger.sendCmdArg("Panel Switches");
    messenger.sendCmdEnd();
    //return;
  }

  if (strcmp(szRequest, "PING") == 0) {
    messenger.sendCmdStart(kRequest);
    messenger.sendCmdArg("PONG");
    messenger.sendCmdArg(messenger.readInt32Arg());
    messenger.sendCmdEnd();
    //return;
  }

  if (strcmp(szRequest, "CONFIG") == 0) {
    // tell SPAD.neXT we are done with config
    messenger.sendCmd(kRequest, "CONFIG");
    isReady = true;
  }
}

// -------------------- STD ARDUINO FUNCTIONS ------------------------- //

void setup() {
  // initialize all the digital pins for input and set their current state
  mySwitches[0].Number = 41;
  mySwitches[1].Number = 43;
  mySwitches[2].Number = 39;
  mySwitches[3].Number = 37;
  mySwitches[4].Number = 35;
  mySwitches[5].Number = 29;
  mySwitches[6].Number = 31;
  mySwitches[7].Number = 23;
  mySwitches[8].Number = 26;
  mySwitches[9].Number = 25;
  mySwitches[10].Number = 53;
  mySwitches[11].Number = 51;
  mySwitches[12].Number = 49;
  mySwitches[13].Number = 47;
  mySwitches[14].Number = 45;
  mySwitches[15].Number = 28;
  mySwitches[16].Number = 30;
  mySwitches[17].Number = 22;
  mySwitches[18].Number = 50;  //24
  mySwitches[19].Number = 27;

  // initialize serial communication:
  Serial.begin(115200);

  //Configure all the pins for INPUT and get their inital state
  for(int x=0; x<20; x++)
  {    
    pinMode(mySwitches[x].Number, INPUT);
    mySwitches[x].State = digitalRead(mySwitches[x].Number);
    mySwitches[x].Str = "BUTTON_" + (String)(x+1);
  }
   
  attachCommandCallbacks();
}

//This function generates and send the SPAD.next serial command based on the input
//String Button -- The vJoy button to command
//String State -- RELEASE/PRESS
void SendSPADCmd(String Button, String State)
{
    messenger.sendCmdStart(kCommand);
    messenger.sendCmdArg(F("EMULATE"));
    messenger.sendCmdArg(F("1234:BEAD:0"));
    messenger.sendCmdArg(Button);
    messenger.sendCmdArg(State);
    messenger.sendCmdEnd();

    //Serial.print("\n");
}

void loop() {
  messenger.feedinSerialData();     //Process incoming serial data, and perform callbacks

  //Loop through all the switches.  If different, update the state, determine the change
  //create and send the SPAD.next command
  for(int x = 0; x < 20; x++)
  {
    int ReadValue = digitalRead(mySwitches[x].Number);
    if(ReadValue != mySwitches[x].State)
    {
      mySwitches[x].State = ReadValue;
      if(ReadValue == 0)
        SendSPADCmd(mySwitches[x].Str, "RELEASE");
      else
        SendSPADCmd(mySwitches[x].Str, "PRESS");
    }
    delay(50);      //short delay to help prevent bouncing
  }
}

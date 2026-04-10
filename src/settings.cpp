//esp-stun modded
#include "display.h"
#include <GyverButton.h>
#include "CONFIG.h"
//#include "ArduinoOTA.h"
#include "menu/settings.h"
#include "Org_01.h"
#include "Org_01.h"
#include "Org_01.h"

extern DisplayType display;
extern GButton buttonUp;
extern GButton buttonDown;
extern GButton buttonOK;
extern GButton buttonBack;
extern bool inMenu;
extern byte currentMenu;
extern byte settingsMenuIndex;
extern byte standbyTimeoutIndex;
extern unsigned long standbyTimeoutMs;
extern byte colorSelectionIndex;
extern const unsigned long standbyTimeoutOptionsMs[];
extern const char* standbyTimeoutLabels[];
extern const byte STANDBY_OPTION_COUNT;
extern const char* colorOptions[];
extern const byte COLOR_OPTION_COUNT;
extern void applyColorScheme();
extern void saveConfig();
extern void OLED_printMenu(DisplayType &display, byte menuIndex);
extern void resetActivityTimer();
//serial
String incoming = "";
bool inited = false;

enum SettingsDetail : byte { SETTINGS_NONE, SETTINGS_COLOR, SETTINGS_CONNECT, SETTINGS_STANDBY, SETTINGS_ABOUT };

static SettingsDetail currentDetail = SETTINGS_NONE;
static byte standbySelectionIndex = 0;
static byte colorSelectionWorking = 0;
static bool colorNeedRedraw = true;
static bool standbyNeedRedraw = true;
static bool aboutNeedRedraw = true;
static bool connectNeedRedraw = true;

void exitSettingsDetail() {
  currentDetail = SETTINGS_NONE;
  displaySettingsMenu(display, settingsMenuIndex);
  colorNeedRedraw = true;
  standbyNeedRedraw = true;
  aboutNeedRedraw = true;
}

void renderColorSetting() {
  colorNeedRedraw = false;
  byte index = colorSelectionWorking;
  byte next = (index + 1) % COLOR_OPTION_COUNT;
  byte prev = (index + COLOR_OPTION_COUNT - 1) % COLOR_OPTION_COUNT;

  int16_t x1, y1;
  uint16_t w, h;

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);
  display.getTextBounds(colorOptions[prev], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 7);
  display.print(colorOptions[prev]);

  display.setTextSize(2);
  display.getTextBounds(colorOptions[index], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 25);
  display.print(colorOptions[index]);

  display.setTextSize(1);
  display.getTextBounds(colorOptions[next], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 50);
  display.print(colorOptions[next]);

  display.setCursor(2, 30);
  display.print(">");
  display.setCursor(120, 30);
  display.print("<");

  display.display();
}

void renderAboutSetting() {
  aboutNeedRedraw = false;
  display.clearDisplay();
  display.setTextColor(1);
  display.setTextSize(2);
  display.setTextWrap(false);
  display.setCursor(5, 5);
  display.print("ESP-STUN-a");

  display.setCursor(77, 24);
  display.print(FIRMWARE);
  display.setTextSize(1);
  display.setCursor(53, 40);
  display.print("based on 0.7");

  display.setCursor(5, 55);
  display.print("github.com/FOXIK-Lab");
  display.drawBitmap(11, 2, image_Teapot_bits, 63, 64, 1);
  display.drawBitmap(0, 22, image_FOXIK_bits, 32, 32, 1);
  display.display();
}

void renderStandbySetting(byte index) {
  standbyNeedRedraw = false;
  byte next = (index + 1) % STANDBY_OPTION_COUNT;
  byte prev = (index + STANDBY_OPTION_COUNT - 1) % STANDBY_OPTION_COUNT;

  int16_t x1, y1;
  uint16_t w, h;

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);
  display.getTextBounds(standbyTimeoutLabels[prev], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 7);
  display.print(standbyTimeoutLabels[prev]);

  display.setTextSize(2);
  display.getTextBounds(standbyTimeoutLabels[index], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 25);
  display.print(standbyTimeoutLabels[index]);

  display.setTextSize(1);
  display.getTextBounds(standbyTimeoutLabels[next], 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 50);
  display.print(standbyTimeoutLabels[next]);

  display.setCursor(2, 30);
  display.print(">");
  display.setCursor(120, 30);
  display.print("<");

  display.display();
}

void handleColorDetail(bool upClick, bool downClick, bool okClick, bool backClick) {
  if (colorNeedRedraw) {
    renderColorSetting();
  }

  if (upClick || downClick) {
    colorSelectionWorking = colorSelectionWorking == 0 ? 1 : 0;
    colorNeedRedraw = true;
  }

  if (okClick) {
    colorSelectionIndex = colorSelectionWorking;
    applyColorScheme();
    saveConfig();
    exitSettingsDetail();
    return;
  }

  if (backClick) {
    exitSettingsDetail();
    return;
  }
}

void handleAboutDetail(bool okClick, bool backClick) {
  if (aboutNeedRedraw) {
    renderAboutSetting();
  }

  if (backClick) {
    exitSettingsDetail();
  }
}

  

void renderConnectedScreen(){
  display.setCursor(66, 17);
  display.print("CONNECTED!");
  display.drawBitmap(0, 10, image_Connected_bits, 118, 59, 1);
  display.display();
}
void renderInfoScreen() {
    display.setFont(NULL);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("This mode need for");
    display.setCursor(0, 8);
    display.print("update firmware,");
    display.setCursor(0, 16);
    display.print("install plugins,");
    display.setCursor(0, 24);
    display.print("use official and");
    display.setCursor(0, 32);
    display.print("3rd party software");
    display.display();
}
void renderConnectSetting() {
display.clearDisplay();
  if (inited == true){
    renderConnectedScreen();
  }
  else {
display.drawBitmap(0, 18, image_Device_bits, 83, 29, 1);
display.drawBitmap(89, 20, image_Cable_bits, 44, 20, 1);
//display.drawBitmap(4, 32, image_Miniwifi_bits, 12, 11, 1); //later
  display.setCursor(7, 56);
  display.print("WAITING FOR CONNECT");


display.display();


//ArduinoOTA.begin();
//ArduinoOTA.handle();
  }


  while (true) {
    if (inited == false)
    Serial.println("init");
    delay(500);

    while (Serial.available()) {
      char c = Serial.read();
      incoming += c;

      if (incoming.indexOf("inited") != -1) {
        inited = true;
        Serial.println("connected");
        Serial.println(DISPLAY);
        renderInfoScreen;
        return;
      }
    }

    if (incoming.length() > 50) {
      incoming = "";
    }
  }   
}


void renderUpdateScreen(){
display.clearDisplay();
display.setTextSize(2);
display.setTextWrap(false);
display.setFont(&Org_01);
display.setCursor(2, 11);
display.println("ESP-STUN");

display.setTextSize(1);
display.setCursor(59, 21);
display.println("UPDATE MODE");
display.setCursor(35, 43);
//display.println("HOLD BOOT!");
display.setFont();
display.setCursor(14, 43);
display.println("DON'T  DISCONNECT");

display.setCursor(32, 54);
display.println("YOUR DEVICE");



}

void handleConnectDetail(bool okClick, bool backClick){
  static bool showInfoScreen = false;
    if (connectNeedRedraw) {
        renderConnectSetting();
        if (!showInfoScreen) {
            renderConnectSetting(); 
        } else {
          renderInfoScreen();
    }
    connectNeedRedraw = false;
  }
  if (okClick) {
        showInfoScreen = !showInfoScreen; 
        
        if (showInfoScreen) {
           renderInfoScreen();
        } else {
            renderConnectSetting();
        }
  };
    if (backClick) {
        showInfoScreen = false; 
        display.setFont(NULL);
        exitSettingsDetail();
        return;
    }
}
void handleStandbyDetail(bool upClick, bool downClick, bool okClick, bool backClick) {
  if (standbyNeedRedraw) {
    renderStandbySetting(standbySelectionIndex);
  }

  if (upClick) {
    standbySelectionIndex = (standbySelectionIndex + STANDBY_OPTION_COUNT - 1) % STANDBY_OPTION_COUNT;
    standbyNeedRedraw = true;
  }
  if (downClick) {
    standbySelectionIndex = (standbySelectionIndex + 1) % STANDBY_OPTION_COUNT;
    standbyNeedRedraw = true;
  }
  if (okClick) {
    standbyTimeoutIndex = standbySelectionIndex;
    standbyTimeoutMs = standbyTimeoutOptionsMs[standbyTimeoutIndex];
    resetActivityTimer();
    saveConfig();
    exitSettingsDetail();
    return;
  }
  if (backClick) {
    exitSettingsDetail();
    return;
  }
}

void enterSettingsDetail(byte menuIndex) {
  if (menuIndex == 0) {
    currentDetail = SETTINGS_COLOR;
    colorNeedRedraw = true;
    colorSelectionWorking = colorSelectionIndex;
    renderColorSetting();
  } else if (menuIndex == 1) {
    currentDetail = SETTINGS_CONNECT;
    connectNeedRedraw = true;
    renderConnectSetting();
  } else if (menuIndex == 2) {
    currentDetail = SETTINGS_STANDBY;
    standbySelectionIndex = standbyTimeoutIndex;
    standbyNeedRedraw = true;
    renderStandbySetting(standbySelectionIndex);
  } else if (menuIndex == 3) {
    currentDetail = SETTINGS_ABOUT;
    aboutNeedRedraw = true;
    renderAboutSetting(); 
  }
}

void handleSettingsSubmenu() {
  buttonUp.tick();
  buttonDown.tick();
  buttonOK.tick();
  buttonBack.tick();

  bool upClick = buttonUp.isClick();
  bool downClick = buttonDown.isClick();
  bool okClick = buttonOK.isClick();
  bool backClick = buttonBack.isClick();

  if (currentDetail == SETTINGS_COLOR) {
    handleColorDetail(upClick, downClick, okClick, backClick);
    return;
  } else if (currentDetail == SETTINGS_CONNECT) {
    handleConnectDetail(okClick, backClick);
    return;
  } else if (currentDetail == SETTINGS_STANDBY) {
    handleStandbyDetail(upClick, downClick, okClick, backClick);
    return;
  } else if (currentDetail == SETTINGS_ABOUT) {
    handleAboutDetail(okClick, backClick);
    return;
  }

  if (upClick) {
    settingsMenuIndex = (settingsMenuIndex + SETTINGS_MENU_ITEM_COUNT - 1) % SETTINGS_MENU_ITEM_COUNT;
    displaySettingsMenu(display, settingsMenuIndex);
  }
  if (downClick) {
    settingsMenuIndex = (settingsMenuIndex + 1) % SETTINGS_MENU_ITEM_COUNT;
    displaySettingsMenu(display, settingsMenuIndex);
  }
  if (okClick) {
    enterSettingsDetail(settingsMenuIndex);
  }
  if (backClick) {
    inMenu = true;
    OLED_printMenu(display, currentMenu);
  }
}

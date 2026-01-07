// === DISPLAY FUNCTIONS ===
void drawMainMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("TUBE DISTORTION");
  
  // Indica se bypass è attivo
  if (bypassEnabled) {
    display.setCursor(95, 0);
    display.print("[BYP]");
  }
  
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  int startItem = max(0, menuSelection - 3);
  for (int i = 0; i < 4 && (startItem + i) < numMenuItems; i++) {
    int itemIndex = startItem + i;
    display.setCursor(4, 14 + i * 12);
    
    if (itemIndex == menuSelection) {
      display.print("> ");
      display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    } else {
      display.print("  ");
    }
    
    display.print(menuItems[itemIndex]);
    display.setTextColor(SH110X_WHITE);
  }
  
  display.display();
}

void drawEditVolume() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("MASTER VOLUME");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print((int)(baseOutputVolume * 100));
  display.print("%");
  
  int barWidth = (int)(baseOutputVolume * 110);
  display.setTextSize(1);
  display.drawRect(5, 42, 118, 10, SH110X_WHITE);
  display.fillRect(6, 43, barWidth, 8, SH110X_WHITE);
  
  display.display();
}

void drawEditDrive() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("DRIVE");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(driveAmount, 2);
  
  display.setTextSize(1);
  int barWidth = (int)((driveAmount / 5.0f) * 110);
  display.drawRect(5, 42, 118, 10, SH110X_WHITE);
  display.fillRect(6, 43, barWidth, 8, SH110X_WHITE);
  
  display.setCursor(5, 54);
  if (driveAmount < 1.0f) display.print("Clean");
  else if (driveAmount < 2.0f) display.print("Crunch");
  else if (driveAmount < 3.0f) display.print("Overdrive");
  else display.print("Heavy");
  
  display.display();
}

void drawStats() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("STATISTICS");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setCursor(5, 14);
  display.print("CPU: ");
  display.print(AudioProcessorUsage(), 1);
  display.print("%");
  
  display.setCursor(5, 24);
  display.print("CPU Max: ");
  display.print(AudioProcessorUsageMax(), 1);
  display.print("%");
  
  display.setCursor(5, 34);
  display.print("RAM: ");
  display.print(AudioMemoryUsage());
  display.print("/");
  display.print(AudioMemoryUsageMax());
  
  display.setCursor(5, 44);
  display.print("Bypass: ");
  display.print(bypassEnabled ? "ON" : "OFF");
  
  display.setCursor(5, 54);
  display.print("Press BACK to reset");
  
  display.display();
}

void drawEditWetDry() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("WET / DRY MIX");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setCursor(5, 18);
  display.print("Wet: ");
  display.print((int)(wetAmount * 100));
  display.println("%");
  
  display.setCursor(5, 28);
  display.print("Dry: ");
  display.print((int)(dryAmount * 100));
  display.println("%");
  
  int wetBar = (int)(wetAmount * 110);
  display.drawRect(5, 40, 118, 8, SH110X_WHITE);
  display.fillRect(6, 41, wetBar, 6, SH110X_WHITE);
  
  int dryBar = (int)(dryAmount * 110);
  display.drawRect(5, 52, 118, 8, SH110X_WHITE);
  display.fillRect(6, 53, dryBar, 6, SH110X_WHITE);
  
  display.display();
}

void drawEditTone() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("TONE");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(toneFreq);
  display.setTextSize(1);
  display.print("Hz");
  
  int barWidth = (int)(((toneFreq - 6000) / 9000.0f) * 110);
  display.drawRect(5, 42, 118, 10, SH110X_WHITE);
  display.fillRect(6, 43, barWidth, 8, SH110X_WHITE);
  
  display.setCursor(5, 54);
  if (toneFreq < 9000) display.print("Dark");
  else if (toneFreq < 12000) display.print("Medium");
  else display.print("Bright");
  
  display.display();
}

void drawEditPreset() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("HARMONIC PRESET");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 25);
  display.print(presetNames[currentPreset]);
  
  display.setTextSize(1);
  display.setCursor(5, 50);
  display.print("Turn to change");
  
  display.display();
}

void drawToggleBypass() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("BYPASS MODE");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(3);
  display.setCursor(20, 25);
  if (bypassEnabled) {
    display.print("ON");
  } else {
    display.print("OFF");
  }
  
  display.setTextSize(1);
  display.setCursor(5, 52);
  if (bypassEnabled) {
    display.print("Clean signal pass");
  } else {
    display.print("Effect active");
  }
  
  display.display();
}

void drawToggleAutoComp() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("AUTO COMPENSATION");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(20, 25);
  if (autoVolumeCompensation) {
    display.print("ON");
  } else {
    display.print("OFF");
  }
  
  if (autoVolumeCompensation) {
    display.setTextSize(1);
    display.setCursor(10, 50);
    display.print("Comp: ");
    display.print((int)(calculateVolumeCompensation(driveAmount) * 100));
    display.print("%");
  }
  
  display.display();
}
void drawDelay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("DELAY");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(String(ritardo));
  display.setTextSize(1);
  display.print("ms");
  
  int barWidth = (int)((ritardo / (float)DELAY_MAX) * 110);
  display.drawRect(5, 42, 118, 10, SH110X_WHITE);
  display.fillRect(6, 43, barWidth, 8, SH110X_WHITE);
  
  /*display.setCursor(5, 54);
  if (toneFreq < 9000) display.print("Dark");
  else if (toneFreq < 12000) display.print("Medium");
  else display.print("Bright");*/
  
  display.display();
}

void drawLevel() {
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("Level Input");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(String(Level));
  display.setCursor(10, 30);
  display.setTextSize(1);
  display.setCursor(10, 50);
  display.print("Peak: ");
  display.display();
  float picco = 0.0f;
  int campioni = 0;

  if(trova_picco(picco ,campioni)){
  display.print(picco,2);  
  display.display();
  }
}


void draw_INPUT_GAIN_L(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("INPUT_GAIN_L");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(inputGain_L,1);


  
  int barWidth = (int)((inputGain_L / (float)INPUT_GAIN_MAX) * 110);
  display.drawRect(5, 42, 118, 10, SH110X_WHITE);
  display.fillRect(6, 43, barWidth, 8, SH110X_WHITE);
  
  display.display();
}

void draw_INPUT_GAIN_R(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("INPUT_GAIN_R");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(inputGain_R,1);


  
  int barWidth = (int)((inputGain_R / (float)INPUT_GAIN_MAX) * 110);
  display.drawRect(5, 42, 118, 10, SH110X_WHITE);
  display.fillRect(6, 43, barWidth, 8, SH110X_WHITE);
  
  display.display();
}
void draw_save_screen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  
  display.setCursor(0, 0);
  display.println("SAVE IN SLOT");
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(preset_view); 
  display.display();
}
void drawPresetScreen() {
  


 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Titolo
  display.setCursor(0, 0);
  display.print("SET: ");
  display.print(getPresetName(storage.presets[preset_view].record.current_Preset));
  if(storage.presets[preset_view].record.activ==true){
  display.println(" "+String(preset_view)+" V");
  }else{
  display.println(" "+String(preset_view));  
  }
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);

  // Parametri
  display.setCursor(0, 14);
  display.print("Drive: ");
  display.println(storage.presets[preset_view].record.drive_Mount, 2);

  display.setCursor(0, 26);
  display.print("Wet:   ");
  display.println(storage.presets[preset_view].record.wet_Amount, 2);

  display.setCursor(0, 38);
  display.print("Dry:   ");
  display.println(storage.presets[preset_view].record.dry_Amount, 2);

  display.setCursor(0, 50);
  display.print("Tone:  ");
  display.print(storage.presets[preset_view].record.tone_Freq);
  display.println(" Hz");

  display.display();
  
}

void updateDisplay() {
  switch (currentMenu) {
    case MENU_MAIN:{
      drawMainMenu();
      break;
    }  
    case MENU_EDIT_DRIVE:{
      drawEditDrive();
      break;
    }
    case MENU_EDIT_VOLUME:{
      drawEditVolume();
      break;
    }
    case MENU_EDIT_WETDRY:{
      drawEditWetDry();
      break;
    }
    case MENU_EDIT_TONE:{
      drawEditTone();
      break;
    }
    case MENU_EDIT_PRESET:{
      drawEditPreset();
      break;
    }
    case MENU_TOGGLE_BYPASS:{
      drawToggleBypass();
      break;
    }
    case MENU_TOGGLE_AUTOCOMP:{
      drawToggleAutoComp();
      break;
    }
    case MENU_CALIBRATE:{
      drawLevel();
      break;  
    }
    case MENU_INPUT_GAIN_L:{
      draw_INPUT_GAIN_L();
      break;
    }
    case MENU_INPUT_GAIN_R:{
      draw_INPUT_GAIN_R();
      break;    
    }
    case MENU_DELAY:{
      drawDelay();
      break;
    }
    case MENU_VIEW_PRESET:{
      drawPresetScreen();
      break;
    }
    case MENU_SAVE_PRESET:{
      draw_save_screen();
      break;
    }
    case MENU_STATS:{
      drawStats();
      break;
    }  
  }
}

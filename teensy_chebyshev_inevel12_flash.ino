#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Encoder.h>
#include "effect_delay_ext8.h"


// flash 
#define FLASH_MAGIC 0xA5B4C3D2
#define PRESET_FILE "presets.dat"
#define FLASH_CS_PIN 6
#define MAX_PRESETS 10  // Numero massimo di preset salvabili



// === CONFIGURAZIONE DISPLAY 1.3" ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// === PIN ROTARY ENCODER E TASTI ===
#define ENCODER_A 2
#define ENCODER_B 3
#define BTN_PUSH 4
#define BTN_CONFIRM 5
#define BTN_BACK 28
#define POWER_PIN 29

Encoder rotaryEncoder(ENCODER_A, ENCODER_B);

// === INPUT GAIN + PEAK ===
AudioMixer4        inputGainL;
AudioMixer4        inputGainR;
AudioAnalyzePeak   peakInL;
AudioAnalyzePeak   peakInR;
// === CANALE SINISTRO (L) ===
AudioInputI2S        audioInput;
AudioEffectWaveshaper waveshaperL;
AudioFilterBiquad    preFilterL;
AudioFilterBiquad    postFilterL;
AudioMixer4          wetDryMixerL;
AudioMixer4          bypassMixerL;  // NUOVO: Mixer per bypass

// === CANALE DESTRO (R) ===
AudioEffectWaveshaper waveshaperR;
AudioFilterBiquad    preFilterR;
AudioFilterBiquad    postFilterR;
AudioMixer4          wetDryMixerR;
AudioMixer4          bypassMixerR;  // NUOVO: Mixer per bypass

AudioMixer4          mixer_delay_L;
AudioMixer4          mixer_delay_R;
// Output
AudioOutputI2S       audioOutput;
AudioMixer4          mixerL;
AudioMixer4          mixerR;

// === AUDIO GRAPH CORRETTO ===
// === DELAY ===

AudioEffectDelayExternal8 delayExtL(AUDIO_MEMORY8_EXTMEM, 46000);
AudioEffectDelayExternal8 delayExtR(AUDIO_MEMORY8_EXTMEM, 46000);
//AudioEffectDelayExternal delayExtL;
//AudioEffectDelayExternal delayExtR;
// Input → Gain
AudioConnection pcInGainL(audioInput, 0, inputGainL, 0);
AudioConnection pcInGainR(audioInput, 1, inputGainR, 0);

// Peak detector
AudioConnection pcPeakL(inputGainL, 0, peakInL, 0);
AudioConnection pcPeakR(inputGainR, 0, peakInR, 0);

// Canale L - Distortion Chain
AudioConnection patchCord1(inputGainL, 0, preFilterL, 0);
AudioConnection patchCord2(preFilterL, 0, waveshaperL, 0);
AudioConnection patchCord3(waveshaperL, 0, postFilterL, 0);
AudioConnection patchCord4(postFilterL, 0, wetDryMixerL, 0);     // Wet
AudioConnection patchCord5(inputGainL, 0, wetDryMixerL, 1);      // Dry
AudioConnection patchCord6(wetDryMixerL, 0, bypassMixerL, 0);    // Processed
AudioConnection patchCord7(inputGainL, 0, bypassMixerL, 1);      // Direct

// Canale L - Delay Chain (INDIPENDENTE)
AudioConnection patchCord19(inputGainL, 0, delayExtL, 0);        // Input → Delay
AudioConnection patchCord31(inputGainL, 0, mixer_delay_L, 0);    // ✅ AGGIUNGI: Input → Dry
AudioConnection patchCord20(delayExtL, 0, mixer_delay_L, 1);     // Tap 1
AudioConnection patchCord21(delayExtL, 1, mixer_delay_L, 2);     // Tap 2
AudioConnection patchCord22(delayExtL, 2, mixer_delay_L, 3);     // Tap 3

// Mix Finale L (SEPARARE GLI INPUT!)
AudioConnection patchCord8(bypassMixerL, 0, mixerL, 0);          // Distortion
AudioConnection patchCord29(mixer_delay_L, 0, mixerL, 1);        // Delay
AudioConnection patchCord9(mixerL, 0, audioOutput, 0);           // Output

// Canale R - Distortion Chain
AudioConnection patchCord10(inputGainR, 0, preFilterR, 0);
AudioConnection patchCord11(preFilterR, 0, waveshaperR, 0);
AudioConnection patchCord12(waveshaperR, 0, postFilterR, 0);
AudioConnection patchCord13(postFilterR, 0, wetDryMixerR, 0);
AudioConnection patchCord14(inputGainR , 0, wetDryMixerR, 1);
AudioConnection patchCord15(wetDryMixerR, 0, bypassMixerR, 0);
AudioConnection patchCord16(inputGainR, 0, bypassMixerR, 1);

// Canale R - Delay Chain
AudioConnection patchCord24(inputGainR, 0, delayExtR, 0);        // Input → Delay
AudioConnection patchCord32(inputGainR, 0, mixer_delay_R, 0);    // ✅ AGGIUNGI: Input → Dry
AudioConnection patchCord25(delayExtR, 0, mixer_delay_R, 1);
AudioConnection patchCord26(delayExtR, 1, mixer_delay_R, 2);
AudioConnection patchCord27(delayExtR, 2, mixer_delay_R, 3);

// Mix Finale R
AudioConnection patchCord17(bypassMixerR, 0, mixerR, 0);         // Distortion
AudioConnection patchCord30(mixer_delay_R, 0, mixerR, 1);        // Delay
AudioConnection patchCord18(mixerR, 0, audioOutput, 1);






AudioControlSGTL5000 audioShield;

// === PARAMETRI AUDIO ===
#define MAX_ORDER 10
float inputGain_L = 1.0f;
float inputGain_R = 1.0f;

const float INPUT_GAIN_MIN = 0.0f;
const float INPUT_GAIN_MAX = 1.0f;

float alpha[MAX_ORDER];
float driveAmount = 0.05f;
float wetAmount = 0.70f;
float dryAmount = 0.30f;
float baseOutputVolume = 0.8f;
int toneFreq = 12000;
bool autoVolumeCompensation = false;
bool bypassEnabled = false;  // Stato bypass
bool shuttingDown = false;  // per spegnimento
bool Adjust_Level = false; // x fermare waveshape in fase di setup level
int ritardo = 0;            // ritardo in ms da applicare a delay
int Level = 8;
char nomi_preset[10][32];
// x calibrazione

bool calib_finish = false;
// regolazione gain 



const int MIN_LINE_LEVEL = 0;
const int MAX_LINE_LEVEL = 15;
const float CLIPPING_THRESHOLD = 0.95f;
const float LOW_SIGNAL_THRESHOLD = 0.40f;

#define WS_SIZE 16385
#define WS_CENTER 8192

DMAMEM float waveshape[WS_SIZE];// per calcolo waveshape in ram
DMAMEM float cheby0[WS_SIZE];
DMAMEM float cheby1[WS_SIZE];



// variabili per delay
const int DELAY_MIN = 0;
const int DELAY_MAX = 46000;   // ms (sicuro per extmem)
const int DELAY_STEP = 10;   // passo encoder

// === PRESETS ARMONICHE ===
enum HarmonicPreset {
  PRESET_TUBE,
  PRESET_TRANSISTOR,
  PRESET_FUZZ,
  PRESET_CLEAN,
  PRESET_CUSTOM,
  PRESET_OVERDRIVE,
  PRESET_DISTORTION,
  PRESET_BLUES,
  PRESET_METAL,
  PRESET_WARM_TUBE,
  PRESET_CRUNCH,
  PRESET_BRIGHT,
  PRESET_DARK,
  PRESET_OCTAVE,
  PRESET_SOFT_CLIP,
  PRESET_HARD_CLIP
};

HarmonicPreset currentPreset = PRESET_TUBE;
// RECORD SU FLASH
struct MeasurementRecord {
 bool activ;
 float drive_Mount;
 float wet_Amount;
 float dry_Amount;
 int tone_Freq;
 HarmonicPreset current_Preset;
 
};

struct Preset {
  char name[32];              // Nome del preset
  MeasurementRecord record;
  bool active;                // Se il preset è valido
  uint32_t checksum;
};


struct PresetStorage {
  uint32_t magic;
  int currentPreset;          // Indice del preset corrente
  Preset presets[MAX_PRESETS];
};

PresetStorage storage;
int activePreset = 0;  // Preset correntemente in uso
int preset_view = 0;


// === MENU E NAVIGAZIONE ===
enum MenuState {
  MENU_MAIN,      //
  MENU_EDIT_DRIVE,//0
  MENU_EDIT_VOLUME,//1
  MENU_EDIT_WETDRY,//2
  MENU_EDIT_TONE,  //3
  MENU_EDIT_PRESET,  //4 preset harmonic
  MENU_TOGGLE_AUTOCOMP,//6
  MENU_TOGGLE_BYPASS, //5    
  MENU_CALIBRATE,     //8
  MENU_DELAY,          //7
  MENU_STATS,         //9
  MENU_INPUT_GAIN_L,  //menu inputs 10
  MENU_INPUT_GAIN_R,  //            11
  MENU_VIEW_PRESET,   //12
  MENU_SAVE_PRESET,   //13
  MENU_SPEGNI         //14
};

MenuState currentMenu = MENU_MAIN;
int menuSelection = 0;
const int numMenuItems = 14;  

const char* menuItems[] = {
  "Drive",  //0
  "Volume", //1
  "Wet/Dry",//2
  "Tone",   //3
  "Harmonic", //4
  "Bypass",           // 5
  "Auto Comp",  //6
  "Delay",      //7
  "Level_IN",   //8
  "Inputs",     //9
  "Stats",      //10
  "Preset",     //11
  "save",       //12
  "OFF"         //13
};

const char* presetNames[] = {
  "Tube",
  "Transist",
  "Fuzz",
  "Clean",
  "Custom",
  "OVERDRIV",
  "DISTORT",
  "BLUES",
  "METAL",
  "WARM TUB",
  "CRUNCH",
  "BRIGHT",
  "DARK",
  "OCTAVE",
  "SOFT CLIP",
  "HARD CLIP"
};

// === GESTIONE PULSANTI ===
unsigned long lastButtonCheck = 0;
bool lastConfirmState = HIGH;
bool lastBackState = HIGH;
bool lastPushState = HIGH;
bool needWaveshapeRegen = false;

void shutdownAudio() {
  AudioNoInterrupts();     // 1️⃣ ferma DMA + processing audio
  asm("dsb");              // barriera memoria
  asm("isb");
  audioShield.disable();
}


void setupFilters() {
  preFilterL.setHighpass(0, 40, 0.5);
  preFilterR.setHighpass(0, 40, 0.5);
  postFilterL.setLowpass(0, toneFreq, 0.5);
  postFilterR.setLowpass(0, toneFreq, 0.5);
}

void updateToneFilter() {
  postFilterL.setLowpass(0, toneFreq, 0.5);
  postFilterR.setLowpass(0, toneFreq, 0.5);
}

void handleEncoder() {
  static long lastEncoderPos = 0;
  long newPos = rotaryEncoder.read() / 4;
  
  if (newPos != lastEncoderPos) {
    int delta = newPos - lastEncoderPos;
    lastEncoderPos = newPos;

    switch (currentMenu) {
      case MENU_MAIN:{
        // VolumeMenu();
       // updateWetDryMix();
        menuSelection += delta;
        if (menuSelection < 0) menuSelection = 0;
        if (menuSelection >= numMenuItems) menuSelection = numMenuItems - 1;
        break;
      }  
      case MENU_EDIT_DRIVE:{
        driveAmount += delta * 0.05f;
        if (driveAmount < 0.05f) driveAmount = 0.05f;
        if (driveAmount > 5.0f) driveAmount = 5.0f;
        needWaveshapeRegen = true;
        if (autoVolumeCompensation) updateOutputVolume();
        break;
       }
      case MENU_EDIT_VOLUME:{
        baseOutputVolume += delta * 0.05f;
        if (baseOutputVolume < 0.1f) baseOutputVolume = 0.1f;
        if (baseOutputVolume > 1.0f) baseOutputVolume = 1.0f;
        updateOutputVolume();
        break;
       }  
      case MENU_EDIT_WETDRY:{
        wetAmount += delta * 0.05f;
        if (wetAmount < 0.5f) wetAmount = 0.5f;
        if (wetAmount > 1.0f) wetAmount = 1.0f;
        dryAmount = 1.0f - wetAmount;
        updateWetDryMix();
        break;
       }  
      case MENU_EDIT_TONE:{
        toneFreq += delta * 500;
        if (toneFreq < 6000) toneFreq = 6000;
        if (toneFreq > 15000) toneFreq = 15000;
        updateToneFilter();
        needWaveshapeRegen = true;
        break;
       }  
      case MENU_EDIT_PRESET:{
        int newPreset = (int)currentPreset + delta;
        if (newPreset < 0) newPreset = 0;
        if (newPreset > 15) newPreset = 15;
        if (newPreset != currentPreset) {
          loadPreset((HarmonicPreset)newPreset);
        }
        break;
       }  
      case MENU_TOGGLE_AUTOCOMP:{
        //autoVolumeCompensation = (newPos % 2 == 0);
        static bool lastState = autoVolumeCompensation;
            if (delta != 0) {
                autoVolumeCompensation = (delta > 0);
                if (autoVolumeCompensation != lastState) {
                    updateOutputVolume();
                    updateDisplay();
                    lastState = autoVolumeCompensation;
                }
          }
        //updateOutputVolume();
        break;
       }
      case MENU_TOGGLE_BYPASS:{
          bypassEnabled = !bypassEnabled;
          updateBypass();
        break;
      }
      case MENU_CALIBRATE: {
           Level += delta * 1;

            if (Level < 0) Level = 0;
            if (Level > 15) Level = 15;

           Adjust_Level = true;
          
         
        break;
       }
       case MENU_DELAY: {
           ritardo += delta * DELAY_STEP;

            if (ritardo < DELAY_MIN) ritardo = DELAY_MIN;
            if (ritardo > DELAY_MAX) ritardo = DELAY_MAX;

  
          updateDelayTime();
         
        break;
       }
        case MENU_STATS:{
   
        break;
        }
        case MENU_INPUT_GAIN_L: {
            inputGain_L += delta * 0.05f;
            if (inputGain_L < INPUT_GAIN_MIN) inputGain_L = INPUT_GAIN_MIN;
            if (inputGain_L > INPUT_GAIN_MAX) inputGain_L = INPUT_GAIN_MAX;
            updateInputGain();
        break;
        }
        case MENU_INPUT_GAIN_R: {
            inputGain_R += delta * 0.05f;
            if (inputGain_R < INPUT_GAIN_MIN) inputGain_R = INPUT_GAIN_MIN;
            if (inputGain_R > INPUT_GAIN_MAX) inputGain_R = INPUT_GAIN_MAX;
            updateInputGain();
         break;
        }
        case MENU_VIEW_PRESET: {
             preset_view += delta * 1;

            if (preset_view < 0) preset_view = 0;
            if (preset_view >= MAX_PRESETS) preset_view = MAX_PRESETS - 1;
            driveAmount = storage.presets[preset_view].record.drive_Mount;
            wetAmount = storage.presets[preset_view].record.wet_Amount;
            dryAmount = storage.presets[preset_view].record.dry_Amount ;
            toneFreq = storage.presets[preset_view].record.tone_Freq;
            currentPreset = storage.presets[preset_view].record.current_Preset;
            loadPreset(currentPreset);
            updateWetDryMix();
            //needWaveshapeRegen = true;
            
         break;
        }
         case MENU_SAVE_PRESET: {
             preset_view += delta * 1;

            if (preset_view < 0) preset_view = 0;
            if (preset_view >= MAX_PRESETS) preset_view = MAX_PRESETS - 1;
            
         break;
         }
   

    }
    
    updateDisplay();
  }
}

void handleButtons() {
  if (millis() - lastButtonCheck < 50) return;
  lastButtonCheck = millis();

  bool confirmState = digitalRead(BTN_CONFIRM);
  bool backState = digitalRead(BTN_BACK);
  bool pushState = digitalRead(BTN_PUSH);

  if (confirmState == LOW && lastConfirmState == HIGH) {
    if (currentMenu == MENU_MAIN) {
      switch (menuSelection) {
        case 0: currentMenu = MENU_EDIT_DRIVE; break;
        case 1: currentMenu = MENU_EDIT_VOLUME; break;
        case 2: currentMenu = MENU_EDIT_WETDRY; break;
        case 3: currentMenu = MENU_EDIT_TONE; break;
        case 4: currentMenu = MENU_EDIT_PRESET; break;
        case 5: currentMenu = MENU_TOGGLE_BYPASS;break;
        case 6: currentMenu = MENU_TOGGLE_AUTOCOMP;
          //autoVolumeCompensation = !autoVolumeCompensation;
          //updateOutputVolume();
          break;
        case 7: currentMenu = MENU_DELAY; break;
        case 8: currentMenu = MENU_CALIBRATE; break;
        case 9: currentMenu =  MENU_INPUT_GAIN_L; break;
        case 10: currentMenu = MENU_STATS; break;
        case 11: currentMenu = MENU_VIEW_PRESET; break;
        case 12: currentMenu = MENU_SAVE_PRESET; break;
        case 13: currentMenu = MENU_SPEGNI; 
             //digitalWrite(POWER_PIN,LOW);
             shuttingDown = true;
          break;
          
      }
   }
    if (currentMenu == MENU_TOGGLE_AUTOCOMP) {
    //autoVolumeCompensation = !autoVolumeCompensation;
        updateOutputVolume();
        updateDisplay();
        delay(200);
        return;
     }
   
     if (currentMenu == MENU_INPUT_GAIN_L) {
         currentMenu = MENU_INPUT_GAIN_R;
         updateDisplay();
         delay(150);
          return;
      }
      if (currentMenu == MENU_INPUT_GAIN_R) {
          currentMenu = MENU_INPUT_GAIN_L;  // oppure resta su R
          updateDisplay();
           delay(150);
           return;
       }
      if (currentMenu == MENU_SAVE_PRESET) {
              updateDisplay();
              delay(150);
              return;
      }
    
      
    
   
  updateDisplay();
  delay(200);
}  
  if (currentMenu == MENU_SAVE_PRESET && pushState == LOW) {
         for (int i = 0; i < MAX_PRESETS; i++) {
                        storage.presets[i].record.activ = false;
                  }
                  storage.presets[preset_view].record.activ = true;
                  storage.presets[preset_view].record.drive_Mount = driveAmount;
                  storage.presets[preset_view].record.wet_Amount = wetAmount;
                  storage.presets[preset_view].record.dry_Amount = dryAmount;
                  storage.presets[preset_view].record.tone_Freq = toneFreq;
                  storage.presets[preset_view].record.current_Preset = currentPreset ;
                  storage.presets[preset_view].checksum = calculateChecksum(storage.presets[preset_view].record);
                  saveAllPresets();
                  display.setCursor(10, 40);
                  display.setTextSize(1);
                  display.print("salvataggio ok"); 
                  display.display();
                  delay(1000);
                  currentMenu = MENU_MAIN;                 
                  updateDisplay();
          
             return;
   }
   
  if (currentMenu == MENU_VIEW_PRESET && pushState == LOW) {
             for (int i = 0; i < MAX_PRESETS; i++) {
                        storage.presets[i].record.activ = false;
                  }
                  storage.presets[preset_view].record.activ = true;
                  driveAmount = storage.presets[preset_view].record.drive_Mount;
                  wetAmount = storage.presets[preset_view].record.wet_Amount;
                  dryAmount = storage.presets[preset_view].record.dry_Amount ;
                  toneFreq = storage.presets[preset_view].record.tone_Freq;
                  currentPreset = storage.presets[preset_view].record.current_Preset;
                  storage.presets[preset_view].checksum = calculateChecksum(storage.presets[preset_view].record);
                  saveAllPresets();
                  updateWetDryMix();
              needWaveshapeRegen = true;
              return;
   }
  if (backState == LOW && lastBackState == HIGH) {
    if (currentMenu == MENU_STATS) {
      AudioProcessorUsageMaxReset();
      AudioMemoryUsageMaxReset();
    }
    currentMenu = MENU_MAIN;
    
    if (currentMenu == MENU_INPUT_GAIN_L || currentMenu == MENU_INPUT_GAIN_R) {
          currentMenu = MENU_MAIN;
          updateDisplay();
          delay(200);
          return;
     }
    
    updateDisplay();
    delay(200);
    
 }
 if (pushState == LOW && lastPushState == HIGH) {
    currentMenu = MENU_MAIN;
    updateDisplay();
    delay(200);
 }
  
  lastConfirmState = confirmState;
  lastBackState = backState;
  lastPushState = pushState;
}
  


void setup() {
 // Serial.begin(115200);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN,HIGH);
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  delay(100);
  Wire.begin();
  display.begin(0x3C, true);
 // SerialFlash.remove(PRESET_FILE);
//  erase_flash_log();
  delay(100);
 
  if (!initFlash()) {
    //Serial.println("ERRORE: Flash non inizializzata!");
    //return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 30);
    display.println("FLASH KO");
    display.display();
    
    delay(3000);
    
    
  }

  loadAllPresets();

  
  bool profilo_attivo = true;  // cerco il profilo preferito in flash
  int i = 0;
  while (profilo_attivo){ 
   
    if (storage.presets[i].record.activ==true) {
    driveAmount = storage.presets[i].record.drive_Mount;
    wetAmount = storage.presets[i].record.wet_Amount;
    dryAmount = storage.presets[i].record.dry_Amount ;
    toneFreq = storage.presets[i].record.tone_Freq;
    currentPreset = storage.presets[i].record.current_Preset;
    profilo_attivo=false;
    loadPreset(currentPreset);
    activePreset = i;
    }
    i++;
  }
  
  
 
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 15);
  display.println("TUBE DISTORTION");
  display.setCursor(20, 30);
  display.println("Chebyshev DSP");
  display.setCursor(15, 45);
  display.println("Initializing...");
  display.display();
  
  pinMode(BTN_CONFIRM, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_PUSH, INPUT_PULLUP);
  
  delay(200);
  AudioMemory(180);  // Aumentato per i mixer aggiuntivi
  Level = 8;
  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_LINEIN);
  
  audioShield.volume(0.7);
  
  audioShield.lineInLevel(Level);
  audioShield.lineOutLevel(29);
  audioShield.adcHighPassFilterDisable();
  delay(500);
  
  //updateInputAutoGain();
  setupFilters();
  updateWetDryMix();
  updateBypass();  // Imposta bypass iniziale (OFF)
  
  //Serial.println("Loading tube preset...");
  loadPreset(PRESET_TUBE);
  
  display.setCursor(15, 55);
  display.println("Generating...");
  display.display();
  
  generateChebyshevWaveshape();
 // === SETUP MIXER DELAY (ENTRAMBI I CANALI) ===
  
  delay(250);
  mixer_delay_L.gain(0, 1.0f);
  mixer_delay_L.gain(1, 0.0f);
  mixer_delay_L.gain(2, 0.0f);
  mixer_delay_L.gain(3, 0.0f);

  mixer_delay_R.gain(0, 1.0f);  // 
  mixer_delay_R.gain(1, 0.0f);  // 
  mixer_delay_R.gain(2, 0.0f);  //
  mixer_delay_R.gain(3, 0.0f);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("WAITING FOR AUDIO");
  display.setCursor(0, 30);
  display.println("Press BACK to skip");
  display.display();
  //updateOutputVolume();
  elapsedMillis inizio;
   bool flag = true;
   inizio = 0;
   
  while (flag) {
    // Check pulsante BACK per skip
    if (digitalRead(BTN_BACK) == LOW) {
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("Skipped - Using");
      display.setCursor(0, 35);
      display.println("default level 5");
      display.display();
      delay(1500);
      Level = 8;
      audioShield.lineInLevel(Level);
      inputGainL.gain(0, 1.0f);
      inputGainR.gain(0, 1.0f);
      calib_finish = true;
      currentMenu = MENU_MAIN;
      updateDisplay();
    }
    if (inizio>=2000){
      flag = false; // out cicle
    }
 
  }
 
  
  //Serial.println("Tube Distortion Ready!");
  //Serial.println("Bypass: OFF (Effect active)");
}

unsigned long lastDisplayUpdate = 0;
unsigned long lastWaveshapeCheck = 0;

void loop() {
  
  if (calib_finish == false){ // calibrazione primo avvio
      bool flag = true;
      while (flag) {
                if (autoCalibrateLiveInputSafe()==true){
                  flag=false;
                }
      }
    calib_finish = true;
    elapsedMillis inizio;
    inizio = 0;
    while(inizio<=2000){
    }
    updateDisplay();
  }
  
  if (shuttingDown) {
    shutdownAudio();
    delay(1000);
    digitalWrite(POWER_PIN,LOW);
  }
  //updateInputAutoGain();
  handleEncoder();
  handleButtons();
  if (Adjust_Level == false) {
  if ((needWaveshapeRegen && millis() - lastWaveshapeCheck > 200)&&(calib_finish)) {
    lastWaveshapeCheck = millis();
    needWaveshapeRegen = false;
    Serial.println("Regenerating waveshape...");
    generateChebyshevWaveshape();
  }
  }else{
    audioShield.lineInLevel(Level);
    Adjust_Level = false;
  }
if (currentMenu == MENU_STATS && millis() - lastDisplayUpdate > 500) {
lastDisplayUpdate = millis();
updateDisplay();
}
if (currentMenu == MENU_VIEW_PRESET  && millis() - lastDisplayUpdate > 500) {
lastDisplayUpdate = millis();
updateDisplay();
}
}

bool trova_picco(float &maxPeak ,int &validSamples){
    elapsedMillis inizio;
    validSamples = 0;
    const int SAMPLES_TO_SKIP = 20; // Salta i primi 20 campion
    maxPeak = 0.0f;
  
    bool signalDetected = false;
    int samplesRead = 0; // per calcolo picco iniziale di avvio
    inizio = 0;
    // Controlla se c'è segnale
    while (inizio<=1200){
    
    if (peakInL.available()) {
        float p = peakInL.read();
        samplesRead++;
    
        // Ignora i primi campioni
        if (samplesRead <= SAMPLES_TO_SKIP) continue;
    
        if (p > 0.01f && p < 0.99f) {  // AGGIUNGI LIMITE SUPERIORE
            if (p > maxPeak) maxPeak = p;
                validSamples++;
                signalDetected = true;
            }
  }
  if (peakInR.available()) {
        float p = peakInR.read();
        samplesRead++;
    
        // Ignora i primi campioni
        if (samplesRead <= SAMPLES_TO_SKIP) continue;
    
        if (p > 0.01f && p < 0.99f) {  // AGGIUNGI LIMITE SUPERIORE
            if (p > maxPeak) maxPeak = p;
                validSamples++;
                signalDetected = true;
        }
   }
  
   } //  inizio
   if (signalDetected){
     return true; 
   }else{
     return false;
   }
}
bool autoCalibrateLiveInputSafe() {

   
    
  float picco = 0.0f;
  int campioni = 0;
 
  // Se non c'è segnale dopo il timeout
  if (!trova_picco(picco ,campioni)) {
    display.clearDisplay();
    display.setCursor(0, 15);
    display.println("NO AUDIO DETECTED");
    display.setCursor(0, 30);
    display.println("Using safe");
    display.setCursor(0, 45);
    display.println("default: Level 5");
    display.display();
    delay(1000);
    
    audioShield.lineInLevel(8);
    inputGainL.gain(0, 1.0f);
    inputGainR.gain(0, 1.0f);
    return true;
  }
 // SEGNALE RILEVATO - Procedi con calibrazione
 //delay(200);


  // Verifica che ci siano abbastanza campioni validi
  if (campioni < 10) {
    display.clearDisplay();
    display.setCursor(0, 15);
    display.println("Signal too weak");
    display.setCursor(0, 30);
    display.println("Using level 8");
    display.display();
    Level = 8;
    audioShield.lineInLevel(8);
    inputGainL.gain(0, 1.0f);
    inputGainR.gain(0, 1.0f);
    return true;
  }else{
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Audio detected!");
    display.setCursor(0, 25);
    display.println("Calibrating...");
    display.setCursor(0, 35);
    display.print("Level "+String(Level));
    display.print(" Peak:");
    display.print(picco,2);
    display.display();
  }
  bool gain_changed = false;
  bool  signal_correct = false;
  
  // CASO 1: Troppo clipping - Riduci gain per la PROSSIMA sessione
  if (picco > CLIPPING_THRESHOLD) {
    if (Level < MAX_LINE_LEVEL) {
      Level--;
      gain_changed = true;
      if (Level < 0) Level=15;
      //"⚠️  CLIPPING rilevato!"
      //"   Gain ridotto: "
      //  "   Ripeti la misura per risultati accurati"
     
    } else {
      //  CLIPPING - Gain già al minimo!";
      //"   Riduci il livello del segnale in ingresso";
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("Riduci livello");
      display.setCursor(0, 25);
      display.println("ingresso");
      display.display();
    }
  }
  // CASO 2: Segnale troppo basso - Aumenta gain per la PROSSIMA sessione
  else if (picco < LOW_SIGNAL_THRESHOLD) {
    if (Level > MIN_LINE_LEVEL) {
      Level++;
      gain_changed = true;
       if (Level > 15) Level=0;
      //"📉 Segnale molto basso"
      //"   Gain aumentato: "
      //"   Ripeti la misura per risultati ottimali"
     
    } else {
      //"📉 Segnale basso - Gain già al massimo!"
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("Aumenta livello");
      display.setCursor(0, 25);
      display.println("ingresso");
      display.display();
    }
  }
  // CASO 3: Segnale OK
  else {
    //"✓ Livello segnale ottimale")
    //"  Gain corrente: "
    signal_correct = true;
  }

  // Applica il nuovo gain (per la PROSSIMA acquisizione)
  if (gain_changed) {
    audioShield.lineInLevel(Level);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Audio detected!");
    display.setCursor(0, 25);
    display.println("Calibrating...");
    display.setCursor(0, 45);
    display.print("Level "+String(Level));
    display.print(" Peak:");
    display.print(picco,2);
    display.display();
    // IMPORTANTE: Livello input ottimale per evitare clipping
    // Valori: 0=3.12V, 1=2.63V, 2=2.22V, 3=1.87V, 4=1.58V (default), 5=1.33V
    /*float voltage_range = 3.12f - (current_line_level * 0.1875f);
    Serial.print("  Range input: ~");
    Serial.print(voltage_range, 2);
    Serial.println("V");
    Serial.println();*/
  }
 
  


 if (signal_correct) {
 
  
  // Mostra risultati
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CALIBRATION OK");
  display.drawLine(0, 10, 128, 10, SH110X_WHITE);
  
  display.setCursor(0, 15);
  display.print("Peak: ");
  display.print(picco, 3);  
  display.setCursor(0, 28);
  display.print("Samples: ");
  display.print(campioni);
  
  display.setCursor(0, 41);
  display.print("Level: ");
  display.print(Level);
 
  
  display.setCursor(0, 54);
  if (picco > 0.95f) {
    display.print("WARN: High signal!");
  } else if (picco > 0.40f) {
    display.print("Perfect!");
  } else {
    display.print("OK");
  }
  
  display.display();

  // === SETUP MIXER FINALE ===
  inputGainL.gain(0, 1.0f);
  inputGainR.gain(0, 1.0f);
  mixerL.gain(0, baseOutputVolume);  // Distortion
  mixerL.gain(1, 0.0);  // Delay chain (ha già il dry!)


  mixerR.gain(0, baseOutputVolume);
  mixerR.gain(1, 0.0);
  
  return true;   //signal correct
 }else{
  return false;
 }
 
}

// === PRESET CONFIGURATIONS ===
void loadPreset(HarmonicPreset preset) {
  currentPreset = preset;
   // EFFECT: 100% processato, 0% diretto
   
  switch(preset) {
    case PRESET_TUBE:
      alpha[0] = 0.8f;
      alpha[1] = 1.5f;
      alpha[2] = 0.3f;
      alpha[3] = 0.8f;
      alpha[4] = 0.1f;
      alpha[5] = 0.4f;
      alpha[6] = 0.05f;
      alpha[7] = 0.2f;
      alpha[8] = 0.02f;
      alpha[9] = 0.1f;
      break;
      
    case PRESET_TRANSISTOR:
      alpha[0] = 1.0f;
      alpha[1] = 1.0f;
      alpha[2] = 0.8f;
      alpha[3] = 0.8f;
      alpha[4] = 0.6f;
      alpha[5] = 0.6f;
      alpha[6] = 0.4f;
      alpha[7] = 0.4f;
      alpha[8] = 0.2f;
      alpha[9] = 0.2f;
      break;
      
    case PRESET_FUZZ:
      alpha[0] = 2.0f;
      alpha[1] = 2.5f;
      alpha[2] = 2.0f;
      alpha[3] = 1.5f;
      alpha[4] = 1.2f;
      alpha[5] = 1.0f;
      alpha[6] = 0.8f;
      alpha[7] = 0.6f;
      alpha[8] = 0.4f;
      alpha[9] = 0.3f;
      break;
      
    case PRESET_CLEAN:
      alpha[0] = 0.1f;
      alpha[1] = 0.2f;
      alpha[2] = 0.05f;
      alpha[3] = 0.1f;
      alpha[4] = 0.02f;
      alpha[5] = 0.05f;
      alpha[6] = 0.01f;
      alpha[7] = 0.02f;
      alpha[8] = 0.005f;
      alpha[9] = 0.01f;
      break;
      
    case PRESET_CUSTOM:
      alpha[0] = 1.2f;
      alpha[1] = 1.0f;
      alpha[2] = 0.8f;
      alpha[3] = 0.6f;
      alpha[4] = 0.4f;
      alpha[5] = 0.3f;
      alpha[6] = 0.2f;
      alpha[7] = 0.15f;
      alpha[8] = 0.1f;
      alpha[9] = 0.05f;
      break;
  

   case PRESET_OVERDRIVE:
      // Overdrive tipo Tube Screamer: mid boost, armoniche pari
      // Suono caldo e mid-forward, ottimo per lead
      alpha[0] = 1.5f;   // 2° armonica forte
      alpha[1] = 0.8f;   // 3° armonica moderata
      alpha[2] = 1.2f;   // 4° armonica (mid boost)
      alpha[3] = 0.4f;   // 5° armonica
      alpha[4] = 0.8f;   // 6° armonica
      alpha[5] = 0.2f;   // 7° armonica
      alpha[6] = 0.4f;   // 8° armonica
      alpha[7] = 0.1f;   // 9° armonica
      alpha[8] = 0.2f;   // 10° armonica
      alpha[9] = 0.05f;  // 11° armonica
      break;
      
    case PRESET_DISTORTION:
      // Distortion tipo Boss DS-1: aggressivo, molto saturo
      // Armoniche alte prominenti, suono metal/hard rock
      alpha[0] = 1.8f;
      alpha[1] = 2.0f;
      alpha[2] = 1.5f;
      alpha[3] = 1.8f;
      alpha[4] = 1.2f;
      alpha[5] = 1.5f;
      alpha[6] = 0.9f;
      alpha[7] = 1.0f;
      alpha[8] = 0.6f;
      alpha[9] = 0.7f;
      break;
      
    case PRESET_BLUES:
      // Blues breaker: armoniche pari dominanti, dolce e dinamico
      // Suono vintage anni '60, risponde al tocco
      alpha[0] = 1.3f;   // 2° armonica
      alpha[1] = 0.5f;   // 3° armonica bassa (meno aggressive)
      alpha[2] = 0.9f;   // 4° armonica
      alpha[3] = 0.3f;   // 5° armonica
      alpha[4] = 0.6f;   // 6° armonica
      alpha[5] = 0.2f;   // 7° armonica
      alpha[6] = 0.4f;   // 8° armonica
      alpha[7] = 0.1f;   // 9° armonica
      alpha[8] = 0.2f;   // 10° armonica
      alpha[9] = 0.05f;  // 11° armonica
      break;
      
    case PRESET_METAL:
      // High gain moderno: saturo, armoniche alte, tight
      // Suono djent/modern metal
      alpha[0] = 2.2f;
      alpha[1] = 2.8f;
      alpha[2] = 2.5f;
      alpha[3] = 2.0f;
      alpha[4] = 1.8f;
      alpha[5] = 1.5f;
      alpha[6] = 1.2f;
      alpha[7] = 1.0f;
      alpha[8] = 0.8f;
      alpha[9] = 0.6f;
      break;
      
    case PRESET_WARM_TUBE:
      // Valvolare ultra-caldo: solo armoniche pari (2°, 4°, 6°, 8°)
      // Suono Hi-Fi, jazz, estremamente musicale
      alpha[0] = 2.0f;   // 2° armonica MOLTO forte
      alpha[1] = 0.1f;   // 3° armonica quasi assente
      alpha[2] = 1.5f;   // 4° armonica forte
      alpha[3] = 0.05f;  // 5° armonica quasi assente
      alpha[4] = 1.0f;   // 6° armonica
      alpha[5] = 0.02f;  // 7° armonica quasi assente
      alpha[6] = 0.6f;   // 8° armonica
      alpha[7] = 0.01f;  // 9° armonica quasi assente
      alpha[8] = 0.3f;   // 10° armonica
      alpha[9] = 0.005f; // 11° armonica quasi assente
      break;
      
    case PRESET_CRUNCH:
      // Classic rock crunch: bilanciato, dinamico
      // Suono AC/DC, Led Zeppelin, rhythm guitar
      alpha[0] = 1.4f;
      alpha[1] = 1.2f;
      alpha[2] = 1.0f;
      alpha[3] = 0.9f;
      alpha[4] = 0.7f;
      alpha[5] = 0.6f;
      alpha[6] = 0.4f;
      alpha[7] = 0.3f;
      alpha[8] = 0.2f;
      alpha[9] = 0.15f;
      break;
      
    case PRESET_BRIGHT:
      // Bright/Treble boost: enfasi su armoniche alte
      // Suono brillante, cutting, ottimo per single coil
      alpha[0] = 0.5f;   // 2° armonica bassa
      alpha[1] = 0.8f;   // 3° armonica
      alpha[2] = 0.4f;   // 4° armonica
      alpha[3] = 1.2f;   // 5° armonica ALTA
      alpha[4] = 0.3f;   // 6° armonica
      alpha[5] = 1.5f;   // 7° armonica MOLTO ALTA
      alpha[6] = 0.2f;   // 8° armonica
      alpha[7] = 1.8f;   // 9° armonica MOLTO ALTA
      alpha[8] = 0.15f;  // 10° armonica
      alpha[9] = 2.0f;   // 11° armonica MASSIMA
      break;
      
    case PRESET_DARK:
      // Dark/Bass boost: solo armoniche basse, scuro e pesante
      // Suono doom, stoner, bass guitar
      alpha[0] = 2.5f;   // 2° armonica MASSIMA
      alpha[1] = 2.0f;   // 3° armonica
      alpha[2] = 1.5f;   // 4° armonica
      alpha[3] = 1.0f;   // 5° armonica
      alpha[4] = 0.5f;   // 6° armonica
      alpha[5] = 0.2f;   // 7° armonica bassa
      alpha[6] = 0.1f;   // 8° armonica molto bassa
      alpha[7] = 0.05f;  // 9° armonica quasi assente
      alpha[8] = 0.02f;  // 10° armonica quasi assente
      alpha[9] = 0.01f;  // 11° armonica quasi assente
      break;
      
    case PRESET_OCTAVE:
      // Octave fuzz: enfasi sulla 2° armonica (ottava)
      // Suono octaver naturale, psichedelico
      alpha[0] = 3.0f;   // 2° armonica (ottava) MASSIMA
      alpha[1] = 0.5f;   // 3° armonica
      alpha[2] = 2.0f;   // 4° armonica (doppia ottava)
      alpha[3] = 0.3f;   // 5° armonica
      alpha[4] = 1.0f;   // 6° armonica (tripla ottava)
      alpha[5] = 0.2f;   // 7° armonica
      alpha[6] = 0.5f;   // 8° armonica
      alpha[7] = 0.1f;   // 9° armonica
      alpha[8] = 0.3f;   // 10° armonica
      alpha[9] = 0.05f;  // 11° armonica
      break;
      
    case PRESET_SOFT_CLIP:
      // Soft clipping: compressione dolce, armoniche basse
      // Suono compressor-like, Nashville, country
      alpha[0] = 0.8f;
      alpha[1] = 0.6f;
      alpha[2] = 0.4f;
      alpha[3] = 0.3f;
      alpha[4] = 0.2f;
      alpha[5] = 0.15f;
      alpha[6] = 0.1f;
      alpha[7] = 0.08f;
      alpha[8] = 0.05f;
      alpha[9] = 0.03f;
      break;
      
    case PRESET_HARD_CLIP:
      // Hard clipping: square wave-like, aggressivo
      // Suono sintetico, fuzz estremo
      alpha[0] = 3.0f;
      alpha[1] = 3.0f;
      alpha[2] = 2.8f;
      alpha[3] = 2.8f;
      alpha[4] = 2.5f;
      alpha[5] = 2.5f;
      alpha[6] = 2.0f;
      alpha[7] = 2.0f;
      alpha[8] = 1.5f;
      alpha[9] = 1.5f;
      break;
  }
  
  needWaveshapeRegen = true;
}
void updateInputGain(){
   inputGainL.gain(0, inputGain_L);
   inputGainR.gain(0, inputGain_R);
}

// === BYPASS FUNCTION ===
void updateBypass() {
  if (bypassEnabled) {
    // BYPASS: 100% segnale diretto, 0% processato
 
    mixer_delay_L.gain(0, 1.0f); // input diretto
    mixer_delay_L.gain(1, 0.0f);
    mixer_delay_L.gain(2, 0.0f);
    mixer_delay_L.gain(3, 0.0f);
    mixer_delay_R.gain(0, 0.0f); // input diretto
    mixer_delay_R.gain(1, 0.0f);
    mixer_delay_R.gain(2, 0.0f);
    mixer_delay_R.gain(3, 0.0f);
    bypassMixerL.gain(0, 0.0f);  // Processed OFF
    bypassMixerL.gain(1, baseOutputVolume);  // Direct ON
    bypassMixerR.gain(0, 0.0f);
    bypassMixerR.gain(1, baseOutputVolume);
  } else {
    // EFFECT: 100% processato, 0% diretto
   
    bypassMixerL.gain(0, baseOutputVolume);  // Processed ON
    bypassMixerL.gain(1, 0.0f);  // Direct OFF
    bypassMixerR.gain(0, baseOutputVolume);
    bypassMixerR.gain(1, 0.0f);
  }
}

// === GENERAZIONE WAVESHAPE CON CHEBYSHEV ===
void generateChebyshevWaveshape()
{
 const float invSize = 2.0f / (WS_SIZE - 1);


    // T0 e T1
    for (int i = 0; i < WS_SIZE; i++) {
      float x = 2.0f * (float)i / (WS_SIZE - 1) - 1.0f;

        cheby0[i] = 1.0f;          // T0
        cheby1[i] = x;      // T1
        waveshape[i] = cheby1[i];  // fondamentale
    }

    // Ordini superiori
    const int maxOrder = min(MAX_ORDER, (int)(sizeof(alpha) / sizeof(float)) + 1);
float drive = driveAmount;
for (int k = 2; k <= maxOrder; k++) {

    for (int i = 0; i < WS_SIZE; i++) {
        float x = i * invSize - 1.0f;
        float Tn = 2.0f * x * cheby1[i] - cheby0[i];
        waveshape[i] += alpha[k - 2] * Tn * drive;

        cheby0[i] = cheby1[i];
        cheby1[i] = Tn;
    }

    drive *= driveAmount;   // progressione sicura
}

    // Rimozione DC reale
    float dc = 0.0f;
    for (int i = 0; i < WS_SIZE; i++) dc += waveshape[i];
    dc /= (float)WS_SIZE;

    for (int i = 0; i < WS_SIZE; i++) waveshape[i] -= dc;

    // Peak normalization (NO RMS per wet/dry)
    float maxAbs = 0.0f;
    for (int i = 0; i < WS_SIZE; i++) {
        float a = fabsf(waveshape[i]);
        if (a > maxAbs) maxAbs = a;
    }

    if (maxAbs > 0.0001f) {
        float gain = 0.95f / maxAbs;  // headroom
        for (int i = 0; i < WS_SIZE; i++) {
            waveshape[i] *= gain;
        }
    }

    // Upload LUT
    waveshaperL.shape(waveshape, WS_SIZE);
    waveshaperR.shape(waveshape, WS_SIZE);
}
// === AUDIO FUNCTIONS ===
float calculateVolumeCompensation(float drive) {
  if (drive < 1.0f) {
    return 1.0f;
  } else if (drive < 3.0f) {
    return 1.0f / (1.0f + (drive - 1.0f) * 0.15f);
  } else if (drive < 6.0f) {
    return 1.0f / (1.0f + (drive - 1.0f) * 0.2f);
  } else {
    return 1.0f / (1.0f + (drive - 1.0f) * 0.25f);
  }
}

void updateOutputVolume() {
  float compensation = autoVolumeCompensation ? calculateVolumeCompensation(driveAmount) : 1.0f;
  float finalVolume = baseOutputVolume * compensation;
  mixerL.gain(0, finalVolume);
  mixerR.gain(0, finalVolume);
}

void VolumeMenu() {
  
    mixer_delay_L.gain(0,  baseOutputVolume); // input diretto
    mixer_delay_L.gain(1, 0.0f);
    mixer_delay_L.gain(2, 0.0f);
    mixer_delay_L.gain(3, 0.0f);
    mixer_delay_R.gain(0, baseOutputVolume); // input diretto
    mixer_delay_R.gain(1, 0.0f);
    mixer_delay_R.gain(2, 0.0f);
    mixer_delay_R.gain(3, 0.0f);
    mixerR.gain(0, baseOutputVolume);
    mixerR.gain(1, 0.0f);
    mixerL.gain(0, baseOutputVolume);
    mixerL.gain(1, 0.0f);   
}

void updateDelayTime() {

  
  if (ritardo>0){
    
    mixerR.gain(0, 0.0f);
    mixerR.gain(1, baseOutputVolume);
    mixerL.gain(0, 0.0f);
    mixerL.gain(1, baseOutputVolume);

    mixer_delay_L.gain(0, 0.0);   // Dry
    mixer_delay_L.gain(1, 1.0);   // Tap 1 OFF
    mixer_delay_L.gain(2, 1.0);
    mixer_delay_L.gain(3, 1.0);
    
    mixer_delay_R.gain(0, 0.0);
    mixer_delay_R.gain(1, 1.0);
    mixer_delay_R.gain(2, 1.0);
    mixer_delay_R.gain(3, 1.0);
    delayExtL.delay(1, ritardo);
    delayExtR.delay(1, ritardo);
  }else{
    VolumeMenu();   
  } 
 
  //delayExtL.delay(0, ritardo);
  
  //delayExtR.delay(0, ritardo);

 // delayExtR.update();
 // delayExtL.update();
}

void updateWetDryMix() {
  /*float wetGain = wetAmount;
  float dryGain = 1.0f - wetAmount;
  
  // Compensazione per mantenere volume costante
  float totalPower = sqrt(wetGain * wetGain + dryGain * dryGain);
  float compensation = 1.0f / totalPower;
  
  wetDryMixerL.gain(0, wetGain * compensation);
  wetDryMixerL.gain(1, dryGain * compensation);
  
  wetDryMixerR.gain(0, wetGain * compensation);
  wetDryMixerR.gain(1, dryGain * compensation);*/
  float wetGain=0.0f;
  float dryGain=0.0f;
  if (!wetAmount == 0.0f){
   
  // Usa curve trigonometriche per mantenere energia costante
  float wetNorm = (wetAmount - 0.5f) / 0.5f;  // Normalizza 0.5-1.0 → 0.0-1.0
  // Curve di crossfade (0° a 90°)
  float angle = wetNorm * (PI / 2.0f);  // 0 a π/2
   wetGain = sinf(angle);          // Da 0 a 1
   dryGain = cosf(angle);          // Da 1 a 0
  
  }
  
  // Applica i gain
  wetDryMixerL.gain(0, wetGain);
  wetDryMixerL.gain(1, dryGain);
  
  wetDryMixerR.gain(0, wetGain);
  wetDryMixerR.gain(1, dryGain);
  //wetAmount = wetGain;
  //dryAmount = dryGain;*/
}

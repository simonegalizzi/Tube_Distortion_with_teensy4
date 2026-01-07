// SALVATAGGIO PARAMETRI SU FLASH w25q128fv



// Calcola checksum
uint32_t calculateChecksum(const MeasurementRecord& rec) {
  uint32_t sum = 0;
  const uint8_t* ptr = (const uint8_t*)&rec;
  for (size_t i = 0; i < sizeof(MeasurementRecord); i++) {
    sum += ptr[i];
  }
  return sum;
}

// Inizializza la flash
/*
bool initFlash() {
  if (!SerialFlash.begin(FLASH_CS_PIN)) {
    
    return false;
  }
  
  uint8_t id[5];
  SerialFlash.readID(id);
  // FLASH ID
  
  if (id[0] == 0xEF && id[1] == 0x40 && id[2] == 0x18) {
    return true;
  }
  return true;
}*/
// Converti enum in stringa per debug
const char* getPresetName(HarmonicPreset preset) {
  switch (preset) {
    case PRESET_TUBE: return "Tube";
    case PRESET_TRANSISTOR: return "Transistor";
    case PRESET_FUZZ: return "Fuzz";
    case PRESET_CLEAN: return "Clean";
    case PRESET_CUSTOM: return "Custom";
    case PRESET_OVERDRIVE: return "Overdrive";
    case PRESET_DISTORTION: return "Distortion";
    case PRESET_BLUES: return "Blues";
    case PRESET_METAL: return "Metal";
    case PRESET_WARM_TUBE: return "Warm Tube";
    case PRESET_CRUNCH: return "Crunch";
    case PRESET_BRIGHT: return "Bright";
    case PRESET_DARK: return "Dark";
    case PRESET_OCTAVE: return "Octave";
    case PRESET_SOFT_CLIP: return "Soft Clip";
    case PRESET_HARD_CLIP: return "Hard Clip";
    default: return "Unknown";
  }
}
bool initFlash() {
  unsigned long startTime = millis();
  const unsigned long TIMEOUT = 2000;  // 2 secondi di timeout
  
  // Mostra sul display che stiamo provando
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Init Flash...");
  display.display();
  
  if (!SerialFlash.begin(FLASH_CS_PIN)) {
    return false;
  }
  
  // Verifica timeout
  if (millis() - startTime > TIMEOUT) {
    return false;
  }
  
  unsigned char id[5];
  SerialFlash.readID(id);
  
  // Verifica timeout
  if (millis() - startTime > TIMEOUT) {
    return false;
  }
  
  if (id[0] == 0xFF || id[0] == 0x00) {
    return false;
  }
  
  return true;
}

// Salva tutti i preset nella flash
bool saveAllPresets() {
  if (SerialFlash.exists(PRESET_FILE)) {
    SerialFlash.remove(PRESET_FILE);
    while (!SerialFlash.ready()) delay(1);
  }
  
  if (!SerialFlash.create(PRESET_FILE, sizeof(PresetStorage))) {
    //"Errore: impossibile creare file preset";
    return false;
  }
  
  SerialFlashFile file = SerialFlash.open(PRESET_FILE);
  if (!file) {
    //"Errore: impossibile aprire file preset"
    return false;
  }
  
  storage.magic = FLASH_MAGIC;
  storage.currentPreset = activePreset;
  
  file.write(&storage, sizeof(PresetStorage));
  file.close();
  
  //"Preset salvati nella flash"
  return true;
}

// Carica tutti i preset dalla flash
bool loadAllPresets() {
  if (!SerialFlash.exists(PRESET_FILE)) {
    //"Nessun preset trovato, inizializzo..."
    initializePresets();
    return false;
  }
  
  SerialFlashFile file = SerialFlash.open(PRESET_FILE);
  if (!file) {
    //"Errore apertura file preset"
    return false;
  }
  
  file.read(&storage, sizeof(PresetStorage));
  file.close();
  
  if (storage.magic != FLASH_MAGIC) {
    //"Dati preset non validi"
    initializePresets();
    return false;
  }
 
  activePreset = 0;
  //"Preset caricati dalla flash"
  return true;
}

// Inizializza i preset con valori di default
void initializePresets() {
  storage.magic = FLASH_MAGIC;
  storage.currentPreset = 0;
  
  for (int i = 1; i < MAX_PRESETS; i++) {
    storage.presets[i].active = false;
    //sprintf(storage.presets[i].name, "Preset %d", i + 1);
    storage.presets[i].record.activ = false;
    storage.presets[i].record.drive_Mount = 0.0f;
    storage.presets[i].record.wet_Amount = 0.0f;
    storage.presets[i].record.dry_Amount = 0.0f;
    storage.presets[i].record.tone_Freq = 0;
    storage.presets[i].record.current_Preset = 0;
    storage.presets[i].checksum = calculateChecksum(storage.presets[i].record);
  }
  /*
 float drive_Mmount
 float wet_Amount
 float dry_Amount
 int tone_Freq
 HarmonicPreset current_Preset
 */
  // Crea un preset di default
  storage.presets[0].active = true;
  strcpy(storage.presets[0].name, "Preset 1");
//  storage.presets[0].record.activ = true;
  storage.presets[0].record.activ = true;
  storage.presets[0].record.drive_Mount = driveAmount;
  storage.presets[0].record.wet_Amount = wetAmount;
  storage.presets[0].record.dry_Amount = dryAmount;
  storage.presets[0].record.tone_Freq = toneFreq;
  storage.presets[0].record.current_Preset = currentPreset;
  storage.presets[0].checksum = calculateChecksum(storage.presets[0].record);
  
  activePreset = 0;
  saveAllPresets();
}

// Salva un preset specifico
bool savePreset(int index, const char* name, const MeasurementRecord& record) {
  if (index < 0 || index >= MAX_PRESETS) {
    //"Errore: indice preset non valido"
    return false;
  }
  
  storage.presets[index].active = true;
  strncpy(storage.presets[index].name, name, 31);
  storage.presets[index].name[31] = '\0';
  storage.presets[index].record = record;
  storage.presets[index].checksum = calculateChecksum(record);
  
  /*Serial.print("Preset ");
  Serial.print(index);
  Serial.print(" '");
  Serial.print(name);
  Serial.println("' salvato");*/
  
  return saveAllPresets();
}

// Carica un preset specifico
bool loadPreset(int index, MeasurementRecord& record) {
  if (index < 0 || index >= MAX_PRESETS) {
    //"Errore: indice preset non valido"
    return false;
  }
  
  if (!storage.presets[index].active) {
    //"Errore: preset non attivo"
    return false;
  }
  
  uint32_t checksum = calculateChecksum(storage.presets[index].record);
  if (checksum != storage.presets[index].checksum) {
    //"Errore: checksum preset non valido"
    return false;
  }
  
  record = storage.presets[index].record;
  activePreset = index;
  storage.currentPreset = activePreset;
  
  /*Serial.print("Caricato preset ");
  Serial.print(index);
  Serial.print(": '");
  Serial.print(storage.presets[index].name);
  Serial.println("'");*/
  
  return true;
}
bool loadPreset_on(MeasurementRecord& record) {
  int index = 0;
  for (int i = 0; i < MAX_PRESETS; i++){ 
  
  if (!storage.presets[index].active) {
    //"Errore: preset non attivo"
    return false;
  }else{
    index = i;
  }
  }
  uint32_t checksum = calculateChecksum(storage.presets[index].record);
  if (checksum != storage.presets[index].checksum) {
    //"Errore: checksum preset non valido"
    return false;
  }
  
  record = storage.presets[index].record;
  activePreset = index;
  storage.currentPreset = activePreset;
  
  /*Serial.print("Caricato preset ");
  Serial.print(index);
  Serial.print(": '");
  Serial.print(storage.presets[index].name);
  Serial.println("'");*/
  
  return true;
}

// Cancella un preset
bool deletePreset(int index) {
  if (index < 0 || index >= MAX_PRESETS) {
    //"Errore: indice preset non valido"
    return false;
  }
  
  storage.presets[index].active = false;
  /*Serial.print("Preset ");
  Serial.print(index);
  Serial.println(" cancellato");*/
  
  return saveAllPresets();
}

// Lista tutti i preset disponibili
bool listPresets(int indice, char nome[32]) {
   
    if (storage.presets[indice].active) {
      strcpy(nome, storage.presets[indice].name);
      return true;
    }else{
      return false;
    }
}

// Mostra dettagli di un preset
void showPresetDetails(int index) {
  if (index < 0 || index >= MAX_PRESETS || !storage.presets[index].active) {
    //Serial.println("Preset non valido");
    return;
  }
  
  Preset& p = storage.presets[index];
  /*
 float drive_Mmount
 float wet_Amount
 float dry_Amount
 int tone_Freq
 HarmonicPreset current_Preset
 */
  Serial.println("\n=== DETTAGLI PRESET ===");
  Serial.print("Nome: "); Serial.println(p.name);
  Serial.print("attivo: "); Serial.println(p.record.activ);
  Serial.print("Drive: "); Serial.print(p.record.drive_Mount);
  Serial.print("wet: "); Serial.println(p.record.wet_Amount);
  Serial.print("dry: "); Serial.print(p.record.dry_Amount);
  Serial.print("tone: "); Serial.print(p.record.tone_Freq); 
  Serial.print("preset "); Serial.print(getPresetName(p.record.current_Preset)); 
  Serial.println("=======================\n");
  
}

// Ottieni il preset corrente
MeasurementRecord getCurrentPreset() {
  return storage.presets[activePreset].record;
}

/*void erase_flash_log() {
  if (!flash_available) return;
  SerialFlash.remove(PRESET_FILE);
  flash_record_count = 0;
  //Serial.println("Log cancellato");
}

void setup() {
  Serial.begin(9600);
  delay(2000);
  
  Serial.println("=== Teensy Audio Shield + W25Q128FV ===");
  Serial.println("Sistema di gestione preset");
  
  if (!initFlash()) {
    Serial.println("ERRORE: Flash non inizializzata!");
    return;
  }
  
  loadAllPresets();
  listPresets();
  
  if (storage.presets[activePreset].active) {
    showPresetDetails(activePreset);
  }
  
  Serial.println("\nComandi disponibili:");
  Serial.println("  l - Lista preset");
  Serial.println("  s[n] - Seleziona preset n (es: s2)");
  Serial.println("  d[n] - Dettagli preset n (es: d1)");
  Serial.println("  n[n] nome - Nuovo preset (es: n3 Test)");
  Serial.println("  x[n] - Cancella preset n (es: x3)");
  Serial.println("  c - Mostra preset corrente");
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.length() == 0) return;
    
    char command = cmd.charAt(0);
    
    switch (command) {
      case 'l': // Lista preset
        listPresets();
        break;
        
      case 's': // Seleziona preset
        if (cmd.length() > 1) {
          int index = cmd.substring(1).toInt();
          MeasurementRecord record;
          if (loadPreset(index, record)) {
            Serial.println("Preset selezionato con successo");
            showPresetDetails(index);
            saveAllPresets();  // Salva il preset corrente
          }
        }
        break;
        
      case 'd': // Dettagli preset
        if (cmd.length() > 1) {
          int index = cmd.substring(1).toInt();
          showPresetDetails(index);
        }
        break;
        
      case 'n': { // Nuovo preset
        int spacePos = cmd.indexOf(' ');
        if (cmd.length() > 1 && spacePos > 0) {
          int index = cmd.substring(1, spacePos).toInt();
          String name = cmd.substring(spacePos + 1);
          
          // Crea un nuovo record con valori esempio
          MeasurementRecord newRecord;
          newRecord.timestamp = millis();
          newRecord.frequency = 1000.0 + random(-100, 100);
          newRecord.amplitude = 0.5;
          newRecord.thd = 0.001;
          newRecord.thdn = 0.002;
          newRecord.snr = 96.0;
          newRecord.noiseFloor = -90.0;
          newRecord.clipping = false;
          
          savePreset(index, name.c_str(), newRecord);
          listPresets();
        } else {
          Serial.println("Uso: n[numero] nome (es: n3 Mio Preset)");
        }
        break;
      }
        
      case 'x': // Cancella preset
        if (cmd.length() > 1) {
          int index = cmd.substring(1).toInt();
          deletePreset(index);
          listPresets();
        }
        break;
        
      case 'c': // Mostra preset corrente
        Serial.print("Preset corrente: ");
        Serial.println(activePreset);
        showPresetDetails(activePreset);
        break;
        
      default:
        Serial.println("Comando non riconosciuto");
        break;
    }
  }
  
  delay(10);
}*/

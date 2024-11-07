/*
 * Progetto: Six_In_Out
 * Autore: Italo Greco
 * Licenza: MIT License (vedi LICENSE per i dettagli).
 *
 *  Tensione di uscita controllata da un massimo di 
 *  sei ingressi digitali, più sei uscite digitali 
 *  controllate da WiFi. 
 *
**/

#include <Arduino.h>
#include <Wire.h>   
#include <ArduinoJson.h>          
#include <Button2.h>
#include <Hardware.h>
#include <Adafruit_GFX.h>         
#include <Adafruit_SSD1306.h>     
#include <Preferences.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#define SCREEN_WIDTH 128  //  Larghezza di visualizzazione in pixel
#define SCREEN_HEIGHT 64  //  Visualizza l'altezza in pixel
#define OLED_RESET    -1  //  Pin di ripristino del display (non utilizzato con I2C)

/*
#define DEBUG_ON  // Abilitazione alla depurazione

#ifdef DEBUG_ON
  Serial.println("Messaggio di depurazione...");
#endif
*/

const int OUT_DAC = 25; // Pin a cui è collegato il DAC sull'ESP32
const int IN_ADC = 35;  // Pin a cui è collegato il ADC sull'ESP32

const int KEY_DOWN = 39;  // I pin per pulsanti
const int KEY_UP = 36;

//Definisce i pin utilizzati come input
const int INPUT_1 = 12;
const int INPUT_2 = 13;
const int INPUT_3 = 14;
const int INPUT_4 = 15;
const int INPUT_5 = 16;
const int INPUT_6 = 17;
const int inputPins[] = {INPUT_1, INPUT_2, INPUT_3, INPUT_4, INPUT_5, INPUT_6, KEY_DOWN, KEY_UP};
const int numPinsFault = sizeof(inputPins) / sizeof(inputPins[0]);

//Definisce i pin utilizzati come output
const int OUTPUT_1 = 5;
const int OUTPUT_2 = 18;
const int OUTPUT_3 = 19;
const int OUTPUT_4 = 23;
const int OUTPUT_5 = 32;
const int OUTPUT_6 = 33;
const int outputPins[] = {OUTPUT_1, OUTPUT_2, OUTPUT_3, OUTPUT_4, OUTPUT_5, OUTPUT_6, OUT_DAC};
const int numPinsOutput = sizeof(outputPins) / sizeof(outputPins[0]);

volatile bool Initial = true;
volatile bool Normal = true;
volatile bool Setting = false;
volatile bool Fault = true;
volatile bool ValueFault = false;

volatile bool TimeOut = false;

uint8_t dacValue = 0;
uint8_t dacValueAnt = 0;
uint16_t valueADC = 0;
int sum = 0;
float voltage = 0.0;

int numbers[] = {1, 2, 3, 4, 5, 6};
int currentSelection = 5; // Indice del numero selezionato

uint8_t ValueRecupery[14] = {};

const int NUM_SAMPLES = 5;  // Numero di campionamento
int samples[NUM_SAMPLES];

const unsigned long SHORT_INTERVAL = 50;  // Intervallo corto in ms
const unsigned long LONG_INTERVAL = 80;   // Intervallo lungo in ms

// Costante per il tempo di inattività massimo
const unsigned long maximunDowntime = 10000;
unsigned long lastActionTime = 0; // Memorizza il tempo in millisecondi
                                  // dell'ultima volta che è stato premuto un pulsante
                                  
// Questa variabile memorizzerà lo stato corrente dei Checkboxes
bool statiCheckbox[] = {true, true, true, true, true, true};

const char* ssid = "name";
const char* password = "psw";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

Adafruit_SSD1306 disp(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Creiamo istanze Button2 per ciascun pulsante
Button2 keyDown;
Button2 keyUp;
// Variabili per gestire tempi di pressione prolungati
const unsigned long LONG_PRESS_TIME = 2000;

// Definire un namespace per Preferences
Preferences preferences;

void handleRoot() {
  const char* htmlTemplate = R"=====(
  <!DOCTYPE html>
  <html lang="it">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width">
    <title>App di Controllo - ESP32</title>
  </head>
  <body style='background-color: #EEEEEE; color: #003366;'>
    <h1>OUTPUT Controller</h1>
     
    <input type='checkbox' id='uscita1' onchange='inviaStato(1, this.checked)'> 
    <label for="uscita1">Uscita 1</label> <br>      
    <input type='checkbox' id='uscita2' onchange='inviaStato(2, this.checked)'>
    <label for="uscita2">Uscita 2</label> <br>
    <input type='checkbox' id='uscita3' onchange='inviaStato(3, this.checked)'> 
    <label for="uscita3">Uscita 3</label> <br>      
    <input type='checkbox' id='uscita4' onchange='inviaStato(4, this.checked)'>
    <label for="uscita4">Uscita 4</label> <br>
    <input type='checkbox' id='uscita5' onchange='inviaStato(5, this.checked)'>
    <label for="uscita5">Uscita 5</label> <br>
    <input type='checkbox' id='uscita6' onchange='inviaStato(6, this.checked)'>
    <label for="uscita6">Uscita 6</label> <br>

    <script>
      // Connettiti al server utilizzando WebSocket
      var websocket = connectWebSocket(); // Avvia la connessione WebSocket
      function connectWebSocket() {
        const socket = new WebSocket('ws://' + window.location.hostname + ':81');
        socket.onopen = function(event) {
          console.log('Connessione stabilita');
          // Richiedere lo stato iniziale dei checkboxes dal server
          inviaStato('getStatoIniziale', null);
        };
        socket.onclose = function(event) {
          console.log("Connessione WebSocket chiusa.");
        };
        socket.onerror = function(error) {
          console.error('Errore di connessione WebSocket:', error);
        };
        socket.onmessage = function(event) {
          var data = JSON.parse(event.data);
          if (Array.isArray(data.stati)) {
            for (var i = 0; i < data.stati.length; i++) {
              var checkbox = document.getElementById('uscita' + (i + 1));
              if (checkbox) {
                checkbox.checked = data.stati[i];
              }
            }
          }         
        };
        return socket;
      }
      function inviaStato(numero, stato) {
        if (websocket && websocket.readyState === WebSocket.OPEN) {
          websocket.send(JSON.stringify({numero: numero, stato: stato}));
          console.log('Send JSON ok');
        } else {
          console.error('WebSocket non è connesso.');
        }
      }
     </script>
  </body>
  </html>  
  )=====";
  server.send(200, "text/html", htmlTemplate);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_TEXT: {
      Serial.printf("Testo ricevuto: [%u] --  %s\n", num, payload);
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
      } else {
        // Gestire la richiesta dello stato iniziale
        if (jsonDoc.containsKey("richiesta") && jsonDoc["richiesta"] == "getStatoIniziale") {
          inviaStatoACliente(num);
        } 
        // Gestire l'aggiornamento dello stato dei checkboxes
        else if (jsonDoc.containsKey("numero") && jsonDoc.containsKey("stato")) {
          int numero = jsonDoc["numero"];
          bool stato = jsonDoc["stato"];

          // Aggiorna lo stato dei checkboxes          
          switch(numero) {
            case 1:
            digitalWrite(OUTPUT_1, stato ? HIGH : LOW);
            break;
            case 2:
            digitalWrite(OUTPUT_2, stato ? HIGH : LOW);
            break;
            case 3:
            digitalWrite(OUTPUT_3, stato ? HIGH : LOW);
            break;
            case 4:
            digitalWrite(OUTPUT_4, stato ? HIGH : LOW);
            break;
            case 5:
            digitalWrite(OUTPUT_5, stato ? HIGH : LOW);
            break;
            case 6:
            digitalWrite(OUTPUT_6, stato ? HIGH : LOW);
            break;
            case NULL:
            Serial.println("NULL di uscita.");
            break;
            default:
            Serial.println("Numero di uscita non valido.");
            break;
          }
          statiCheckbox[numero - 1] = stato;
          // Qui puoi aggiungere ulteriore logica per gestire il cambiamento di stato
          for (int n = 0; n < 6; n++) {
            Serial.println(statiCheckbox[n] ? "true" : "false");
          }
          // Chiama inviaStatoATutti() per propagare la modifica a tutti i client
          inviaStatoATutti();
          break;
        } 
      } 
    }
    case WStype_CONNECTED:
      Serial.printf("Cliente %u connesso\n", num);
      break;
    case WStype_DISCONNECTED:
      Serial.printf("Cliente %u disconnesso\n", num);
      break; 
  } 
}

// Invia lo stato al cliente che lo ha richiesto
void inviaStatoACliente(uint8_t clientNum) {
  StaticJsonDocument<200> jsonDoc;
  JsonArray statiArray = jsonDoc.createNestedArray("stati");
  for (int i = 0; i < 6; i++) {
    statiArray.add(statiCheckbox[i]);
  }
  String message;
  serializeJson(jsonDoc, message);
  // Invia il messaggio solo al client specificato
  webSocket.sendTXT(clientNum, message); 
}

void inviaStatoATutti() {
  // Crea un messaggio JSON con lo stato completo di tutti gli output
  StaticJsonDocument<200> jsonDoc;
  JsonArray statiArray = jsonDoc.createNestedArray("stati");
  for (int i = 0; i < 6; i++) {
    statiArray.add(statiCheckbox[i]);
  }
  String message;
  serializeJson(jsonDoc, message);
  // Invia il messaggio a tutti i client connessi
  webSocket.broadcastTXT(message);
}

void singleClick1(Button2& btn) {
  if (TimeOut) {
    // Reimposta il tempo in millisecondi dell'ultima azione quando si accede alla subroutine
    lastActionTime = millis();
  }
  if (Normal) {
    if (Setting) {
      if (dacValue >= 1 && dacValue <= ValueRecupery[1]) {
        ValueRecupery[0] = --dacValue;
      } else if (dacValue >= ValueRecupery[1]+2 && dacValue <= ValueRecupery[3]) {
        ValueRecupery[2] = --dacValue;
      } else if (dacValue >= ValueRecupery[3]+2 && dacValue <= ValueRecupery[5]) {
        ValueRecupery[4] = --dacValue;
      } else if (dacValue >= ValueRecupery[5]+2 && dacValue <= ValueRecupery[7]) {
        ValueRecupery[6] = --dacValue;
      } else if (dacValue >= ValueRecupery[7]+2 && dacValue <= ValueRecupery[9]) {
        ValueRecupery[8] = --dacValue;
      } else if (dacValue >= ValueRecupery[9]+2 && dacValue <= ValueRecupery[11]) {
        ValueRecupery[10] = --dacValue;
      } else if (dacValue >= ValueRecupery[11]+2 && dacValue <= 255) {
        ValueRecupery[12] = --dacValue;
      }
      dacWrite(OUT_DAC, dacValue);  // Invia dacValue all'uscita del DAC
    }
  } else {
    currentSelection--;
    if (currentSelection < 0) {
      currentSelection = 5;
    }
  }
}  

void singleClick2(Button2& btn) {
  if (TimeOut) {
    // Reimposta il tempo in millisecondi dell'ultima azione quando si accede alla subroutine
    lastActionTime = millis();
  }
  if (Normal) {
    if (Setting) {
      if (dacValue >= 0 && dacValue <= ValueRecupery[1]-1) {
        ValueRecupery[0] = ++dacValue;
      } else if (dacValue >= ValueRecupery[1]+1 && dacValue <= ValueRecupery[3]-1) {
        ValueRecupery[2] = ++dacValue;
      } else if (dacValue >= ValueRecupery[3]+1 && dacValue <= ValueRecupery[5]-1) {
        ValueRecupery[4] = ++dacValue;
      } else if (dacValue >= ValueRecupery[5]+1 && dacValue <= ValueRecupery[7]-1) {
        ValueRecupery[6] = ++dacValue;
      } else if (dacValue >= ValueRecupery[7]+1 && dacValue <= ValueRecupery[9]-1) {
        ValueRecupery[8] = ++dacValue;
      } else if (dacValue >= ValueRecupery[9]+1 && dacValue <= ValueRecupery[11]-1) {
        ValueRecupery[10] = ++dacValue;
      } else if (dacValue >= ValueRecupery[11]+1 && dacValue <= 254) {
        ValueRecupery[12] = ++dacValue;
      }
      dacWrite(OUT_DAC, dacValue);  // Invia dacValue all'uscita del DAC
    }
  } else {
    currentSelection++;
    if (currentSelection > 5) {
      currentSelection = 0;
    }
  }
}

// Calcolo della frammentazione dei valori DAC
void fragment(int n, byte *arr) {
  // Calcoliamo la nuova dimensione segmentata
  int segment = 2 * n;
  // Calcola l'incremento per suddividere l'intervallo utilizzando numeri interi
  int increment = 255 / segment; // Divisione intera troncata
  // Riempi l'array con i valori suddivisi in blocchi utilizzando il troncamento
  for (int i = 0; i <= segment; i++) {
    arr[i] = i * increment; // Qui viene eseguita la moltiplicazione dei numeri interi
  }
  // Regola l'ultimo valore in modo che sia esattamente 255
  arr[segment] = 255;
  // Riempi gli spazi rimanenti con 255 se la segmentazione <12
  for (int i = segment + 1; i < 13; i++) {
    arr[i] = 255;
  }
  // L'ultimo elemento dell'array e la quantita di pin che si decidera adoperare
  arr[13] = n;

  Scrivere_Valori();
}

void Leggere_Valori() {
  preferences.begin("Data", false); // Leggi l'array di byte dallo namespace "Data" con la chiave "ValueRecupery"
  size_t len = preferences.getBytes("ValueRecupery", ValueRecupery, sizeof(ValueRecupery));
  
  #ifdef DEBUG_ON
  // Verificare che sia stato letto correttamente
  if (len == sizeof(value_recupery)) {
    Serial.println("Array letto correttamente: ");
    for (size_t i = 0; i < len; i++) {
      Serial.println(value_recupery[i]);
    }
  } else {
    Serial.println("Errore durante la lettura dell'array");
  }
  #endif

  preferences.end();  
}

void Scrivere_Valori() {  
  // Avvia Preferences con lo namespace "Data"
  preferences.begin("Data", false);
  preferences.putBytes("ValueRecupery", ValueRecupery, sizeof(ValueRecupery));
  // Chiudere la connessione con NVS
  preferences.end();
}

/*
Funzione Callback da richiamare alla pressione del pulsante.
Quando entra per la prima volta Setting = true
e la seconda volta, impostando su false, scrive
i nuovi valori nell'array ValueRecupery[].
*/

void longClick1(Button2& btn) {
  TimeOut = true;
  if (Normal) {
    Setting = !Setting;
    if(!Setting) {
      Scrivere_Valori();
      TimeOut = false;
      lastActionTime = 0;
    }
  } else {
    switch(numbers[currentSelection]) {
      case 1:
      fragment(1, ValueRecupery);
      break;
    
      case 2:      
      fragment(2, ValueRecupery);
      break;
     
      case 3:  
      fragment(3, ValueRecupery);
      break;
     
      case 4:     
      fragment(4, ValueRecupery);
      break;
      
      case 5:   
      fragment(5, ValueRecupery);
      break;
     
      case 6:     
      fragment(6, ValueRecupery);
      break;
     
      default: break;                            
    }
    Leggere_Valori();
    Initial = true;
    Normal = true;
    TimeOut = false;
    lastActionTime = 0;
  }
}

void longClick2(Button2& btn) {
  TimeOut = true;
  Normal = false;
}

void IRAM_ATTR INT_FAULT(){
  Fault = true;
}

void setup() {
  Serial.begin(115200);
  
  // Inizializza i pin come ingressi GPIO 12, 13, 14, 15, 16, 17, KEY_DOWN e KEY_UP
  for (int i = 0; i < numPinsFault; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);
  }

  /*
  I pin GPIO da 34 a 39 sono GPI: solo pin di ingresso.
  Questi pin non hanno resistenze interne di pull-up o pull-down.
  Non possono essere utilizzati come uscite, quindi utilizza questi pin solo come ingressi:
  GPIO34, GPIO35, GPIO36, GPIO39
  pinMode(KEY_DOWN);  // INPUT_PULLDOWN o INPUT_PULLUP con resistenze essterne
  pinMode(KEY_UP);  // INPUT_PULLDOWN o INPUT_PULLUP con resistenze essterne
  */

  pinMode(IN_ADC, INPUT); 
  
  // Inizializza i pin come uscite GPIO 5, 18, 19, 23, 32, 33 e OUT_DAC
  for (int i = 0; i < numPinsOutput; i++) {
    pinMode(outputPins[i], OUTPUT);
  }

  // Mete in stato HIGH i pin uscite GPIO 5, 18, 19, 23, 32 e 33
  for (int i = 0; i < numPinsOutput - 1; i++) {
    digitalWrite(outputPins[i], HIGH);   
  }

  // Variabili locali per controllare il tempo di connessione WiFi
  unsigned long startAttemptTime = millis();  // Momento in cui inizia il tentativo di connessione
  unsigned long timeoutPeriod = 5000;         // Tempo di attesa massimo in millisecondi (5 secondi)

  // Prova a connetterti alla rete WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connessione alla rete Wi-Fi");
  Serial.println(ssid);

  // Prova a connetterti alla rete WiFi fino al raggiungimento del timeout
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeoutPeriod) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // Controlla se la connessione ha avuto successo
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connessione Wi-Fi stabilita");
    Serial.println("Indirizzo IP:");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Impossibile connettersi al WiFi entro il tempo specificato");
    // Qui puoi gestire il caso di mancata connessione, 
    // ad esempio provare a connetterti di nuovo in un secondo momento
  }

  // Percorso HTTP per servire la pagina HTML
  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Avvia Preferences con lo spazio dei nomi: "Data".
  preferences.begin("Data", false);
  size_t len = preferences.getBytes("ValueRecupery", ValueRecupery, sizeof(ValueRecupery));
  // Controlla se sono stati trovati dati
  if (len == 0) {
    // Se non sono stati trovati dati, utilizzare i valori predefiniti e salva l'array
    fragment(6, ValueRecupery); // valore di default   
    preferences.putBytes("ValueRecupery", ValueRecupery, sizeof(ValueRecupery));    
  }

  #ifdef DEBUG_ON
  // Stampa l'array ottenuto
  Serial.print("Array ottenuto: ");
  for (int i = 0; i < 14; i++) {
    Serial.print(ValueRecupery[i]);
    Serial.print(" ");
  }
  #endif

  // Chiudi la connessione con NVS
  preferences.end();

  keyDown.begin(KEY_DOWN);  // Inizializzare i pulsanti.
  keyUp.begin(KEY_UP);
  
  // Imposta il tempo di pressione prolungata per il button1 e button2
  keyDown.setLongClickTime(LONG_PRESS_TIME);
  keyUp.setLongClickTime(LONG_PRESS_TIME);

  // Configurare i handlers per button1
  keyDown.setTapHandler(singleClick1);
  keyDown.setLongClickDetectedHandler(longClick1);

  // Configurare i handlers per button2
  keyUp.setTapHandler(singleClick2);
  keyUp.setLongClickDetectedHandler(longClick2); 

  // Callback per tutti i Pin ingreso a CHANGE (GPIO 12, 13, 14, 15, 16 e 17)
  for (int i = 0; i < numPinsFault - 2; i++) {
    attachInterrupt(inputPins[i], INT_FAULT, CHANGE); // LOW/HIGH/FALLING/RISING/CHANGE
  }

  disp.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // inizializzare il display con l'indirizzo I2C 0x3C
  disp.clearDisplay();
  disp.display();
  
  analogReadResolution(12); // Leggi il valore del pin dell'ADC con risoluzione a 12 bit
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // Chiama il metodo loop() per ciascun pulsante nel ciclo principale
  keyDown.loop();
  keyUp.loop();

  if(Initial) {  
    Fault = true;
    Initial = false;
  }

  if(Fault) {
    dacValueAnt = dacValue;
    for (int i = 0; i < numPinsFault; i++) {
      delay(100);
      // Fare la somma dei pin che sono a livello basso
      if (digitalRead(inputPins[i]) == LOW) {
        sum += 1;      
      }
    }

    // Calcolare il valore DAC proporzionale al numero di pin a livelli bassi   
    switch(sum){
      case 0: dacValue = ValueRecupery[0]; break;
      case 1: dacValue = ValueRecupery[2]; break;
      case 2: dacValue = ValueRecupery[4]; break;
      case 3: dacValue = ValueRecupery[6]; break;
      case 4: dacValue = ValueRecupery[8]; break;
      case 5: dacValue = ValueRecupery[10]; break;
      case 6: dacValue = ValueRecupery[12]; break;
      default: break;
    }

    if (dacValue >= dacValueAnt) {     
      dacWrite(OUT_DAC, dacValue);
      dacValueAnt = 0;
      ValueFault = false;
    } else {
      ValueFault = true;
    }
    sum = 0;
    Fault = false;
  }

  /*
  Calcola il tempo trascorso dall'ultima volta 
  che è stato premuto un pulsante e se supera il massimo, 
  torna alla modalità NORMAL MODE, 
  salvando solo le ultime impostazioni effettuate, 
  senza intervenire sul numero di pin selezionati come ingressi.  
  */
  if (TimeOut) {
    unsigned long currentTime = millis();
    if (lastActionTime == 0) {
      lastActionTime = currentTime;
    }
    if (currentTime - lastActionTime > maximunDowntime) {
      Scrivere_Valori();      
      TimeOut = false;
      lastActionTime = 0;
      Normal = true;
      Setting = false;
    }
  }
    
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;

  if (ValueFault) {
    if (currentMillis - previousMillis >= LONG_INTERVAL) {
      --dacValueAnt;     
      if (dacValueAnt <= dacValue) {
        dacWrite(OUT_DAC, dacValue);  // Assicurati di non superare il valore obiettivo
        dacValueAnt = 0;
        previousMillis = 0;
        ValueFault = false;
      } else {
        dacWrite(OUT_DAC, dacValueAnt);
        previousMillis = currentMillis;
      }
    }
  }  
  
  static int sampleIndex = 0;
  static unsigned long lastShortIntervalTime = 0;

  // Controlla se gli intervalli di tempo per la raccolta dei campioni sono trascorsi
  if (currentMillis - lastShortIntervalTime >= SHORT_INTERVAL) {   
    if (sampleIndex < NUM_SAMPLES) {
      samples[sampleIndex] = analogRead(IN_ADC);
      sampleIndex++;
    } else {
      // Ordina campioni      
      for (int i = 0; i < NUM_SAMPLES - 1; i++) {
        for (int j = i + 1; j < NUM_SAMPLES; j++) {
          if (samples[j] < samples[i]) {
            int temp = samples[i];
            samples[i] = samples[j];
            samples[j] = temp;
          }
        }
      }      
      valueADC = samples[NUM_SAMPLES / 2];  // Prendi il campione centrale      
      sampleIndex = 0;  // Reimposta l'Index di campionamento dopo aver raggiunto il massimo
    }
    lastShortIntervalTime = currentMillis; // Aggiorna la tempistica dell'ultimo intervallo
  }

  //voltage = 1.54 * ((3.3 / 4095) * valueADC);  // Convertire il valore letto in 5 Volt con un riferimento di 3,3 Volt

  int valueMapped = map(valueADC, 0, 4095, 0, 5000);  // Mappa il valore letto a un valore equivalente a 5000 miliVolt
  voltage = (float)valueMapped / 1000.0;  // Converte valore a Volt

  disp.setTextSize(1);               
  disp.setTextColor(WHITE, BLACK);
  disp.clearDisplay();
  disp.setCursor(0, 0);
  if (Normal) {
    if (Setting) {
      disp.println("SETTING MODE:");  
    } else {
      disp.println("NORMAL MODE:");
    } 
    disp.println();
    disp.println("DAC to Volts -->");
    disp.setCursor(0, 29);
    disp.setTextSize(2);
    disp.print(voltage, 2);
    disp.println(" V");
    disp.setTextSize(1); 
    disp.println();
    disp.print("dacValue = ");
    disp.println(dacValue); 
    disp.display();
  } else {
    disp.setCursor(0, 0);
    disp.setTextSize(1); 
    disp.print("ACTUAL PINS = "); 
    disp.println(ValueRecupery[13]);    
    disp.println();
    disp.println("INPUT PINS ? -->");
    disp.println();
    for (int i = 0; i < 6; i++) {
      disp.print(numbers[i]);
      disp.print(" ");
    }    
    int cursorX = currentSelection * 12; // Regolare lo spazio in base alla fonte e alle dimensioni
    disp.setCursor(cursorX, 40);
    disp.print("^");
    disp.println();
    disp.println();
    disp.println("Up/Down for select");     
    disp.display();   
  }
}

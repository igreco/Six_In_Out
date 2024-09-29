
# Six_In_Out

Tensione di uscita controllata da un massimo di sei ingressi digitali, in più sei uscite digitali gestite tramite Wi-Fi.

Licenza:
Questo progetto è concesso secondo i termini della Licenza MIT, per i dettagli, vedere il file LICENSE.

Licenze di terze parti:
Questo progetto utilizza diverse librerie di terze parti, 
ognuna delle quali è soggetta alle proprie licenze. 
Per i dettagli, vedere il file THIRD_PARTY_LICENSES.md.

---

Six_In_Out è un programma per ESP32, che genera una tensione di uscita di controllo corrispondente a la quantità d'ingressi digitali attivi.
Indipendentemente sono presenti sei uscite digitali gestiti tramite l'ESP32 come Webserver (vedi dettagli sotto).

Per testarlo si utilizza un kit ESP32 su breadboard, opto accoppiatori 4N26, 
sia per gli ingressi che per le uscite (adattando così ingressi e uscite a 5 V, visto che l'ESP32 funziona a 3,3 V), un display SSD1306 tipo OLED, 2 pulsanti (N.A.), un amplificatore operazionale per adattare la uscita a 5 V, resistenze varie e cavi di interconnessione (vedere circuito Six_In_out.pdf).

Se le uscite non sono necessarie il tutto può essere ridotto al minimo con l'utilizzo dell'Atmega328P (vedi progetto Six_In).

Gli ingressi digitali sono normali a livello alto e attivi a livello basso, considerati in FAULT.

Nel caso di tutti gli ingressi a livello alto, 
la tensione in uscita sarà minima, ovvero 0,0 Volt (GND).

Nel caso di tutti gli ingressi a livello basso (tutti gli ingressi in FAULT), 
la tensione in uscita sarà massima, ovvero 3,3 Volt (VDD nel caso di ESP32).

In base al numero di voci in FAULT la tensione in uscita viene generata come segue:


| Ingressi in FAULT | Tensione di uscita generata |
|:-----------------:|-----------------------------|
| NESUNO            | 0,0 Volt (GND).             |
| 1                 | 1/6 VDD                     |
| 2                 | 2/6 VDD (1/3).              |
| 3                 | 3/6 VDD (1/2).              |
| 4                 | 4/6 VDD (2/3).              |
| 5                 | 5/6 VDD.                    |
| 6                 | 3,3 Volts (VDD).            |

La tensione di uscita generata puo essere variata in più o in meno 
tramite due pulsanti identificati come UP e DOWN, una sorta di trimmer digitale di regolazione.

Il display mostra lo stato in cui si trova l'ESP32, che può essere:
NORMAL MODE o SETTING MODE.

Nello stato NORMAL MODE, il display mostra il valore assoluto del DAC e l'uscita in Volt della tensione di controllo normalizzata da 0 a 5 Volt (valore misurato tramite l'ADC dell'ESP32).

Per passare allo stato SETTING MODE è necessario tenere premuto il pulsante DOWN per due secondi.
Nello stato SETTING MODE, i pulsanti svolgono le loro funzioni, 
potendo regolare verso l'alto o verso il basso la tensione di controllo dell'uscita e visualizzarla sul display SSD1306.

Generando i FAULT (un ingresso in FAULT, due ingressi in FAULT, tre... ecc.), è possibile effettuare una regolazione fine della tensione di controllo necessaria per ogni tipo di FAULT.

Una volta completate le correzioni, premendo nuovamente il pulsante DOWN per due secondi, l'ESP32 uscirà dallo stato SETTING MODE e si porterà a lo stato NORMAL MODE, facendo sì che tutte le impostazioni effettuate vengano memorizzate nella memoria non volatile (NVS), in questo modo le impostazioni rimarranno permanenti anche se VDD è disconnesso dall'ESP32.

Per ritornare ai valori iniziali (0, 1/6, 2/6, ecc.) o DEFAULT procedere come segue:
Dallo stato NORMAL MODE premere per due secondi, questa volta però, il tasto UP, 
il display cambia e mostrerà ora una riga con i valori da 1 a 6 e sotto di essi 
un cursore che si sposterà con i due tasti (con DOWN verso sinistra e UP verso la destra del display), posizionando il cursore sotto 6 ed uscendo dallo stato SETTING MODE con il tasto DOWN, tenendolo premuto per due secondi, le impostazioni dei valori della tensione di uscita torneranno a quelle di la tabella dei valori iniziali precedente.

Come si può vedere intuitivamente, è possibile scegliere il numero di input necessari da 1 a 6.

Gli ingressi non necessari verranno scartati dal circuito rimuovendo i relativi ponticelli (vedere circuito Six_In_Out.pdf).

I valori iniziali della tensione di uscita saranno secondo la seguente tabella:

| **VALORI INIZIALI** | **_NESSUN FAULT_** | **_1 FAULT_** | **_2 FAULT_** | **_3 FAULT_** | **_4 FAULT_** | **_5 FAULT_** | **_6 FAULT_** |
|:-------------------:|:------------------:|:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
|    **x 6 input**    |      _0 Volt_      |   _1/6 VDD_   |   _1/3 VDD_   |   _1/2 VDD_   |   _2/3 VDD_   |   _5/6 VDD_   |     _VDD_     |
|    **x 5 input**    |      _0 Volt_      |   _1/5 VDD_   |   _2/5 VDD_   |   _3/5 VDD_   |   _4/5 VDD_   |     _VDD_     |     _VDD_     |
|    **x 4 input**    |      _0 Volt_      |   _1/4 VDD_   |   _1/2 VDD_   |   _3/4 VDD_   |     _VDD_     |     _VDD_     |     _VDD_     |
|    **x 3 input**    |      _0 Volt_      |   _1/3 VDD_   |   _2/3 VDD_   |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |
|    **x 2 input**    |      _0 Volt_      |   _1/2 VDD_   |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |
|    **x 1 input**    |      _0 Volt_      |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |

Cioè, le tensioni di uscita vengono trattate in base al numero di pin di ingresso scelti e in base al numero di ingressi che sono in FAULT.

Il numero di pin in questione così come le loro impostazioni una volta memorizzati nella memoria non volatile 
rimangono permanenti finché non viene effettuata una nuova modifica con i due pulsanti e visualizzata sul display.

Il display e i due pulsanti possono essere un modulo separabile.
Una volta impostato l'ESP32 è possibile scollegare il modulo, 
rendendo più economica la produzione in quanto con un solo modulo si possono impostare altre schede e non essendo lasciati i pulsanti su di esso non permette di variare facilmente i valori per l'utente.

# Funzionamento degli ingressi in FAULT.

Ogni volta che viene generato un FAULT, la tensione di uscita viene generata rapidamente.
Quando la transizione avviene da FAULT a normale, l'uscita della tensione di controllo avverrà gradualmente attraverso una rampa fino a 3 secondi, 
producendo così un'uscita di tensione di controllo di tipo Soft Start.

# Operazione di uscita.

Le uscite ESP32, pin GPIO: 5, 18, 19, 23, 32 e 33 (normalmente in stato alto, attive in stato basso), sono gestite tramite una pagina HTML.

Sfruttando la possibilità di connessione WiFi del micro ed utilizzandolo come WebServer si può accedere ad una schermata in cui vengono visualizzati 6 checkbox che possono essere attivati o disattivati, ciò può essere fatto da tutti i client che possono connettersi alla stessa rete con la relativa password.

Attraverso WebSocket i client in questione potranno attivare o disattivare output casuali, vedendo tutti contemporaneamente cosa sta succedendo.
Ho potuto fare un test con due cellulari e un computer collegati contemporaneamente e tutto è andato bene.

Le uscite sono totalmente indipendenti dagli ingressi, nulla vieta che possano essere intrecciate e gestite tramite porte OR con gli ingressi precedentemente citati.

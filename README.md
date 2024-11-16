
# Six_In_Out

Tensione di uscita controllata da un massimo di sei ingressi digitali, in più sei uscite digitali gestite tramite Wi-Fi.

Licenza:
Questo progetto è concesso secondo i termini della Licenza MIT, per i dettagli, vedere il file LICENSE.

Licenze di terze parti:
Questo progetto utilizza diverse librerie di terze parti, 
ognuna delle quali è soggetta alle proprie licenze. 
Per i dettagli, vedere il file THIRD_PARTY_LICENSES.md.

---

Six_In_Out è un programma per ESP32 che genera una tensione in uscita in corrispondenza a la quantità d'ingessi digitali attivi, tra uno a sei come massimo.
Indipendentemente sono presenti sei uscite digitali gestiti tramite l'ESP32 come Webserver (vedi dettagli sotto).

Per testarlo si utilizza un kit ESP32 su breadboard, opto accoppiatori 4N26, 
sia per gli ingressi che per le uscite (adattando così ingressi e uscite a 5 V, visto che l'ESP32 funziona a 3,3 V), un display SSD1306 tipo OLED, 2 pulsanti (N.A.), un amplificatore operazionale per adattare la uscita a 5 V, resistenze varie e cavi di interconnessione (vedere circuito Six_In_out.pdf).

Se le uscite non sono necessarie il tutto può essere ridotto al minimo con l'utilizzo dell'Atmega328P (vedi progetto Six_In).

Il programma permette di settare la quantità degli ingressi digitali di controllo: da uno (minimo) a sei (massimo), chiamati Control_In.

I Control_In sono normali a livello alto e attivi a livello basso.

Nel caso di tutti i Control_In a livello alto, 
la tensione in uscita sarà minima, ovvero 0,0 Volt (GND).

Nel caso di tutti i Control_In a livello basso, 
la tensione in uscita sarà massima, ovvero 3,3 Volt (VDD nel caso di ESP32).

Una volta programmato il micro, parte con un settaggio di sei ingressi di controllo (sei Control_In), che è il modo per default, dopo vedremmo come vanno cambiati.

La seguente tabella in basso mostra come è generata la tensione in uscita in riferimento a la quantità di Control_In attivi.


| Control_In | Tensione di uscita nominale generata |
|:-----------------:|-----------------------------|
| NESSUNO            | 0,0 Volt (GND).             |
| UNO                 | 1/6 VDD                     |
| DUE                 | 1/3 VDD (2/6).              |
| TRE                 | 1/2 VDD (3/6).              |
| QUATRO                 | 2/3 VDD (4/6).              |
| CINQUE                 | 5/6 VDD.                    |
| SEI                 | VDD (3,3 Volts).            |

La tensione di uscita generata può essere variata in più o in meno 
tramite due pulsanti identificati come UP e DOWN, una sorta di trimmer digitale di regolazione.

Il display mostra lo stato in cui si trova l'ESP32, che può essere:
NORMAL MODE o SETTING MODE.

Nello stato NORMAL MODE, il display mostra il valore assoluto del DAC e l'uscita in Volt della tensione di controllo normalizzata da 0 a 5 Volt (valore misurato tramite l'ADC dell'ESP32).

Per passare allo stato SETTING MODE è necessario tenere premuto il pulsante DOWN per due secondi.
Nello stato SETTING MODE, i pulsanti svolgono le loro funzioni, 
potendo regolare verso l'alto o verso il basso la tensione di controllo dell'uscita e visualizzarla sul display SSD1306. Mettendo a massa ogni Control_In necessari si vedrà sul display la tensione nominale generata e si potrà agire come detto prima, andando a scatti ogni volta che si pigia i pulsanti.

Una volta completate le correzioni, premendo nuovamente il pulsante DOWN per due secondi, l'ESP32 uscirà dallo stato SETTING MODE e si porterà a lo stato NORMAL MODE, facendo sì che tutte le impostazioni effettuate vengano memorizzate nella memoria non volatile (NVS), in questo modo le impostazioni rimarranno permanenti anche se VDD è disconnesso dall'ESP32.

Da far notare che nello stato SETTING MODE, si dalla ultima volta che si ha pigiato su un pulsante passa 60 secondi senza aver fatto il passaggio a lo stato NORMAL MODE tenendo premuto nuovamente il pulsante DOWN per due secondi, il ESP32 torna in automatico a lo stato NORMAL MODE facendo sì che tutte le impostazioni effettuate fino questo instante vengano memorizzate nella memoria non volatile (NVS), in questo modo le impostazioni fatte non si perdono e si potrà riprendere quelle mancanti in futuro.

Se invece dallo stato NORMAL MODE si preme per due secondi, questa volta però, il tasto UP, 
il display cambia e mostrerà ora una riga con i valori da 1 a 6 e sotto di essi 
un cursore che si sposterà con i due tasti (con DOWN verso sinistra e UP verso la destra del display), posizionando il cursore sotto la quantità che si vuole di Control_In ed uscendo dallo stato SETTING MODE con il tasto DOWN, tenendolo premuto per due secondi, le impostazioni della quantità dei Control_In cambierà a la scelta fatta e i valori della tensione di uscita sarà come indicati nella prossima tabella.
Nuovamente se, sin dalla ultima volta che si ha pigiato su un pulsante passa 60 secondi senza aver fatto il passaggio a lo stato NORMAL MODE tenendo premuto nuovamente il pulsante DOWN per due secondi, il ESP32 torna in automatico a lo stato NORMAL MODE, questa volta pero non cambierà la scelta della quantità di Control_In, sarà compito del utente a farlo solo pigiando per due secondi il pulsante DOWN, il resto rimane invariato. Cosi è possibile scegliere il numero di input necessari da 1 a 6.

Gli ingressi non necessari verranno scartati dal circuito rimuovendo i ponticelli su di essi (vedere circuito Six_In_Out.pdf). Se invece si decide di restare i ponticelli il comportamento sarà sulla quantità di Control_In attivi sul totale, per controllare la tensione di uscita. Ad esempio: scegliendo 3 Control_In come quantità e mantenendo tutti 6 ponticelli, qualunque de i sei ingressi sarà visto per il programma come attivo e facendo si che il valore di tensione varie accordo a la massima quantità di tre, finché non scenda a due o uno la uscita si fermerà al valore di tensione corrispondente a tre. 

I valori della tensione di uscita nominali saranno secondo la seguente tabella:

| **Control_In Attivi -->** | **_NESSUNO_** | **_UNO_** | **_DUE_** | **_TRE_** | **_QUATTRO_** | **_CINQUE_** | **_SEI_** |
|:-------------------:|:------------------:|:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|:-------------:|
|    **x 6 input**    |      _0 Volt_      |   _1/6 VDD_   |   _1/3 VDD_   |   _1/2 VDD_   |   _2/3 VDD_   |   _5/6 VDD_   |     _VDD_     |
|    **x 5 input**    |      _0 Volt_      |   _1/5 VDD_   |   _2/5 VDD_   |   _3/5 VDD_   |   _4/5 VDD_   |     _VDD_     |     _VDD_     |
|    **x 4 input**    |      _0 Volt_      |   _1/4 VDD_   |   _1/2 VDD_   |   _3/4 VDD_   |     _VDD_     |     _VDD_     |     _VDD_     |
|    **x 3 input**    |      _0 Volt_      |   _1/3 VDD_   |   _2/3 VDD_   |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |
|    **x 2 input**    |      _0 Volt_      |   _1/2 VDD_   |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |
|    **x 1 input**    |      _0 Volt_      |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |     _VDD_     |

Cioè, le tensioni di uscita vengono trattate in base al numero di Control_In di ingresso scelti e in base al numero di Control_In attivi.

Il numero Control_In in questione così come le loro impostazioni una volta memorizzati nella memoria non volatile 
rimangono permanenti finché non viene effettuata una nuova modifica con i due pulsanti e visualizzata sul display.

Il display e i due pulsanti possono essere un modulo separabile.
Una volta impostato l'ESP32 è possibile scollegare il modulo, 
rendendo più economica la produzione in quanto con un solo modulo si possono impostare altre schede e non essendo lasciati i pulsanti su di esso non permette di variare facilmente i valori per l'utente.

# Funzionamento degli ingressi Control_In.

Ogni volta che un Control_In viene attivato, la tensione di uscita viene generata rapidamente.
Quando la transizione sui Control_In avviene da attivato a normale, l'uscita della tensione di controllo avverrà gradualmente attraverso una rampa scendente fino a 3 secondi, 
producendo così un'uscita di tensione di controllo di tipo Soft Start.

# Operazione di uscita.

Le uscite ESP32, pin GPIO: 5, 18, 19, 23, 32 e 33 (normalmente in stato alto, attive in stato basso), sono gestite tramite una pagina HTML.

Sfruttando la possibilità di connessione WiFi del micro ed utilizzandolo come WebServer si può accedere ad una schermata in cui vengono visualizzati 6 checkbox che possono essere attivati o disattivati, ciò può essere fatto da tutti i client che possono connettersi alla stessa rete con la relativa password.

Attraverso WebSocket i client in questione potranno attivare o disattivare output casuali, vedendo tutti contemporaneamente cosa sta succedendo.
Ho potuto fare un test con due cellulari e un computer collegati contemporaneamente e tutto è andato bene.

Le uscite sono totalmente indipendenti dagli ingressi, nulla vieta che possano essere intrecciate e gestite tramite porte OR con gli ingressi precedentemente citati.

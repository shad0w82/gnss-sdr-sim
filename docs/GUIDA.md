# Guida all'uso di gnss-sdr-sim (GPS + Galileo)

Questa guida spiega come generare un segnale GPS L1 C/A e Galileo E1 con `gnss-sdr-sim` e come
trasmetterlo a un ricevitore GNSS con un front-end SDR (HackRF, bladeRF, ADALM-Pluto, LimeSDR,
USRP).

## Indice

1. [Come funziona](#1-come-funziona)
2. [Avvertenze](#2-avvertenze)
3. [Requisiti e compilazione](#3-requisiti-e-compilazione)
4. [Effemeridi](#4-effemeridi)
5. [Generare il segnale](#5-generare-il-segnale)
6. [Trasmettere con un front-end SDR](#6-trasmettere-con-un-front-end-sdr)
7. [Collegamento al ricevitore e livelli di potenza](#7-collegamento-al-ricevitore-e-livelli-di-potenza)
8. [Riferimento delle opzioni](#8-riferimento-delle-opzioni)
9. [Problemi frequenti](#9-problemi-frequenti)

---

## 1. Come funziona

`gnss-sdr-sim` lavora in due passi:

1. **Genera un file** di campioni I/Q in banda base. Per ogni satellite visibile dalla posizione
   scelta calcola distanza, Doppler, ritardi atmosferici, codice di spreading e messaggio di
   navigazione, poi somma tutti i satelliti in un unico segnale.
2. **Un programma dell'SDR trasmette il file** a 1575,42 MHz, la frequenza comune a GPS L1 e
   Galileo E1.

```
 RINEX (effemeridi) ─┐
 posizione/traiettoria ─┼─> gnss-sdr-sim ──> gpssim.bin ──> player SDR ──> attenuatore ──> ricevitore
 ora di inizio ───────┘
```

Il file contiene solo il segnale: il simulatore non aggiunge rumore termico. Il rumore lo
aggiungono l'SDR e il ricevitore.

Alcuni dettagli del segnale:

- **Doppler.** Il moto relativo tra satellite e ricevitore sposta sia la frequenza della portante
  sia la velocità del codice: il chip rate diventa 1,023 MHz × (1 + f_D/1575,42 MHz). Ogni 0,1 s
  la fase del codice viene ricalcolata dalla pseudodistanza, quindi codice e portante restano
  coerenti per tutta la simulazione. Nel Galileo le sottoportanti sono agganciate al codice e
  seguono lo stesso Doppler.
- **Modulazione Galileo E1.** CBOC(6,1,1/11): sottoportanti BOC(1,1) e BOC(6,1) con pesi
  √(10/11) e √(1/11), in fase su E1-B (dati) e in controfase su E1-C (pilota). I lobi della
  BOC(6,1) arrivano a 7,16 MHz, quindi la CBOC completa richiede una frequenza di campionamento di
  almeno 14,322 MHz (`-s 15000000`). Sotto questa soglia la componente BOC(6,1), che porta 1/11
  della potenza, viene omessa, come farebbe un front-end di quella banda; il simulatore lo segnala
  all'avvio. Per i ricevitori a banda stretta (circa 4 MHz, quasi tutti i chip commerciali) il
  segnale è lo stesso nei due casi.
  Ogni campione è la media della forma d'onda CBOC sulla durata del campione: il campionamento
  puntuale porterebbe in banda le armoniche delle sottoportanti (a 15 MHz una transizione della
  BOC(6,1) cade ogni 1,2 campioni) e disturberebbe il tracking dei ricevitori CBOC. La media
  attenua un po' la BOC(6,1): −2,5 dB a 15 MHz, −1,4 dB a 20 MHz, −0,6 dB a 30 MHz. Se SDR e disco
  lo permettono, 20–30 MHz danno la CBOC più fedele.
- **Tempo Galileo.** I satelliti Galileo seguono il Galileo System Time (GST), che differisce dal
  tempo GPS di pochi nanosecondi. Il simulatore legge l'offset GST−GPS (GGTO) dalla riga `GAGP`
  dell'header RINEX, ΔtSystems = A0G + A1G·(t − t0G + 604800·(WN − WN0G)), e lo applica al
  segnale: orbite e orologi Galileo sono calcolati in GST, la fase del codice e il messaggio I/NAV
  portano tempi GST, mentre il tempo del ricevitore resta il tempo GPS. Lo stesso GGTO è trasmesso
  nella parola 10 dell'I/NAV: un ricevitore GPS+Galileo deve applicarlo, oppure stimare l'offset
  tra i due sistemi, come con il cielo vero. Senza la riga `GAGP` GST e tempo GPS coincidono.

Il segnale è stato verificato con un ricevitore software indipendente (acquisizione, tracking,
decodifica I/NAV, soluzione ai minimi quadrati) su uno scenario statico di 34 s a Torino:

- tutte le pagine I/NAV degli 11 satelliti Galileo hanno il CRC corretto, a 5 e a 15 MHz;
- a 20 MHz lo spettro mostra i lobi della BOC(6,1) a ±6,1 MHz con il 6,8% della potenza Galileo;
- l'uscita GPS è identica byte per byte a quella di GPS-SDR-SIM originale.

Errore medio 3D della posizione:

| Campionamento | Solo GPS | Solo Galileo | GPS + Galileo con GGTO | GPS + Galileo senza GGTO |
|---|---|---|---|---|
| 5 MHz (parte BOC(1,1) della CBOC) | 8,4 cm | 7,0 cm | 4,1 cm | 10,9 cm |
| 15 MHz (CBOC completa, replica CBOC) | | 4,1 cm | 2,2 cm | 8,5 cm |

Quel giorno il GGTO valeva −0,32 ns (circa 10 cm): un ricevitore che lo ignora vede le
pseudodistanze Galileo spostate di tanto rispetto a quelle GPS.

## 2. Avvertenze

- **Non trasmettere in aria.** Emettere segnali sulle frequenze GNSS è vietato dalla legge e
  disturba tutti i ricevitori nei dintorni, compresi quelli usati per la sicurezza (aviazione,
  servizi di emergenza). Collega sempre l'SDR al ricevitore **via cavo, con attenuatori**
  (sezione 7), oppure lavora dentro una camera schermata.
- **Non superare i livelli di potenza indicati.** Un segnale troppo forte satura l'ingresso del
  ricevitore e può danneggiare un LNA o un'antenna attiva.
- **Separa il ricevitore dal segnale vero.** Se il ricevitore ha anche un'antenna reale collegata,
  vedrà i due segnali insieme e non darà un fix pulito.

## 3. Requisiti e compilazione

### Simulatore

Serve un compilatore C:

```bash
make
```

oppure

```bash
gcc gpssim.c -lm -O3 -o gnss-sdr-sim
```

Su Windows si compila con Visual Studio aggiungendo al progetto `gpssim.c` e `getopt.c`
(vedi README).

Per il download automatico delle effemeridi (`-D`) servono anche i comandi `curl` e `gzip`. Su
macOS e Linux sono già installati. Su Windows `curl` è incluso, mentre `gzip` va installato a
parte, per esempio con Git for Windows.

### Software dei front-end

| Front-end | Software per trasmettere | Formato del file |
|---|---|---|
| HackRF One | `hackrf_transfer` (pacchetto `hackrf`), oppure `player/hackplayer` | 8 bit (`-b 8`) |
| bladeRF | `bladeRF-cli` (pacchetto `bladeRF`), oppure `player/bladeplayer` | 16 bit (default) o 1 bit |
| ADALM-Pluto | `player/plutoplayer` (libiio, libad9361) | 16 bit (default) |
| LimeSDR | `player/limeplayer` (LimeSuite) | 1, 8 o 16 bit |
| USRP (UHD) | `gnss-sdr-sim-uhd.py` (GNU Radio) o `tx_samples_from_file` (esempi UHD) | 8 o 16 bit |

I player della cartella `player/` si compilano con `make <nome>` dentro `player/`, dopo aver
installato la libreria del dispositivo (istruzioni in `player/README.md`).

> **Importante:** `hackplayer`, `bladeplayer` e `plutoplayer` trasmettono a una frequenza di
> campionamento fissa di **2,6 MHz**. Con Galileo il simulatore usa 5 MHz per default: per questi
> player bisogna generare il file con `-s 2600000` (segnale Galileo un po' degradato), oppure
> trasmettere con `hackrf_transfer` o `bladeRF-cli`, che accettano qualsiasi frequenza.

## 4. Effemeridi

Le effemeridi dicono dove si trovano i satelliti e come vanno i loro orologi. Il simulatore le
legge da un file RINEX di navigazione.

### Download automatico (`-D`)

Con `-D` il simulatore scarica da solo il file del giorno dello scenario dall'archivio pubblico
BKG (nessuna registrazione) e lo salva nella cartella corrente:

```bash
./gnss-sdr-sim -D -l 45.07,7.66,250 -t 2026/10/30,10:00:00   # file del 30 ottobre 2026
./gnss-sdr-sim -D -l 45.07,7.66,250 -T now                   # effemeridi recenti, spostate all'ora attuale
./gnss-sdr-sim -D -l 45.07,7.66,250                          # file di oggi
```

- **Giorni passati:** scarica il file definitivo `BRDC00IGS_R_...` e lo riusa agli avvii successivi.
- **Giorno corrente:** scarica il file aggiornato in tempo reale `BRDC00WRD_S_...`, ogni volta di
  nuovo, perché cresce durante la giornata. Contiene effemeridi solo fino a circa un'ora prima:
  per uno scenario "adesso" usa `-T now`.

### File locale (`-e`)

- **RINEX 3 misto** (GPS + Galileo), per esempio `BRDC00IGS_R_AAAAGGG0000_01D_MN.rnx`, da
  [BKG](https://igs.bkg.bund.de/root_ftp/IGS/BRDC/) o da
  [CDDIS](https://cddis.nasa.gov/archive/gnss/data/daily/) (registrazione gratuita). `AAAA` è
  l'anno e `GGG` il giorno dell'anno. I file sono compressi: decomprimili con `gunzip`.
- **RINEX 2 GPS** (`brdcGGG0.AAn`): funziona come nella versione originale, solo GPS. Nel repo c'è
  `brdc0010.22n` come esempio.

```bash
./gnss-sdr-sim -e BRDC00IGS_R_20263000000_01D_MN.rnx -l 45.07,7.66,250 -t 2026/10/27,10:00:00
```

### Ora di inizio

- **`-t AAAA/MM/GG,hh:mm:ss`:** l'ora deve cadere dentro l'intervallo coperto dal file, altrimenti
  il simulatore risponde "Invalid start time" e stampa l'intervallo valido.
- **`-T AAAA/MM/GG,hh:mm:ss` o `-T now`:** sposta le effemeridi all'ora indicata. Il ricevitore
  "vede" quella data anche se il file è di un altro giorno. È l'opzione da usare quando il
  ricevitore deve mostrare la data e l'ora attuali.
- **Nessuna delle due:** lo scenario parte dalla prima effemeride del file.

## 5. Generare il segnale

### Esempi base

Posizione statica a Torino, GPS + Galileo, 5 minuti, con ritardo troposferico:

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 300 -a -o gpssim.bin
```

Lo stesso, ma per HackRF (8 bit):

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 300 -a -b 8 -o gpssim.bin
```

Solo GPS a 2,6 MHz, per i player a frequenza fissa:

```bash
./gnss-sdr-sim -D -T now -C G -l 45.07,7.66,250 -d 300 -a -o gpssim.bin
```

Solo Galileo:

```bash
./gnss-sdr-sim -D -T now -C E -l 45.07,7.66,250 -d 300 -a -o gpssim.bin
```

### Traiettorie (modalità dinamica)

Le traiettorie sono campionate a 10 Hz. Nel repo ci sono file di esempio:

```bash
./gnss-sdr-sim -e brdc0010.22n -u circle.csv -b 8          # ECEF x,y,z (con tempo in prima colonna)
./gnss-sdr-sim -e brdc0010.22n -x circle_llh.csv -b 8      # lat,lon,altezza
./gnss-sdr-sim -e brdc0010.22n -g triumphv3.txt -b 8       # stream NMEA GGA
```

La simulazione dura quanto il file della traiettoria, senza limiti; con `-d` la si può accorciare.
La quota dei file `-x` è **ellissoidica** (non sul livello del mare).

### Disegnare la traiettoria su una mappa

Nella cartella `tools` ci sono due strumenti che producono il file per `-x` (o `-u`) a partire da
un percorso. Danno esattamente lo stesso risultato.

**Pagina web `route_planner.html`.** Il browser non carica gli script della pagina da `file://`,
quindi va servita via HTTP:

```bash
python3 -m http.server 8000 --directory tools
```

Poi apri http://localhost:8000/route_planner.html e:

1. cerca il luogo per nome, oppure sposta la mappa;
2. scegli il tracciato: linea retta, strade (auto), ciclabile o a piedi;
3. clicca sulla mappa per aggiungere i punti; trascinali per spostarli; clicca su un punto per
   impostare la velocità del tratto successivo o per cancellarlo;
4. imposta movimento e quota (pannello a sinistra) e premi **Genera CSV**: la pagina mostra la
   traiettoria (tratteggio rosa), il riepilogo e il profilo di velocità, e scarica `route.csv`;
5. con **Salva GeoJSON** salvi il percorso per riaprirlo dopo (Importa) o per usarlo con lo script.

La pagina importa anche file GPX, KML e GeoJSON, per esempio percorsi disegnati con
[gpx.studio](https://gpx.studio) (che segue le strade e aggiunge le quote) o con Google My Maps
(esporta in KML).

**Script `route2motion.py`.** Fa la stessa conversione da riga di comando, utile per automatizzare
i test. Richiede solo Python 3, senza librerie aggiuntive.

```bash
python3 tools/route2motion.py percorso.gpx -o percorso.csv --speed 50
python3 tools/route2motion.py percorso.kml -o percorso.csv --speed 5 --height 250   # a piedi, quota fissa
python3 tools/route2motion.py route.geojson -o route.csv                             # salvato dalla pagina
python3 tools/route2motion.py percorso.gpx -o percorso.csv --duration 600            # dura 10 minuti
```

Poi:

```bash
./gnss-sdr-sim -D -T now -x percorso.csv -a -b 8
```

**Parametri principali** (uguali nella pagina e nello script):

| Parametro | Default | Significato |
|---|---|---|
| Velocità (`--speed`) | 50 km/h | Velocità di crociera; nella pagina anche per singolo tratto |
| Accelerazione (`--accel`) | 1,5 m/s² | Accelerazione e frenata massime |
| Accel. laterale (`--lat-accel`) | 2 m/s² | Limita la velocità in curva (valore indicativo, entro circa il 15%) |
| Arrotondamento (`--smooth`) | 20 m | Gli angoli del percorso diventano curve di questa dimensione |
| Sosta iniziale (`--hold-start`) | 60 s | Fermo all'inizio: dà tempo al ricevitore di fare il primo fix |
| Sosta finale (`--hold-end`) | 10 s | Fermo alla fine |
| Quota | terreno | Dal file del percorso, costante (`--height`) o dal modello del terreno |
| Antenna (`--agl`) | 0 m | Altezza dell'antenna sopra il suolo, per esempio 1,5 m su un'auto |

**Come funziona.** Il percorso viene ricampionato ogni metro e smussato, così gli angoli diventano
curve. La velocità viene limitata in curva e da accelerazione e frenata, e la traiettoria viene
campionata a 10 Hz. Le quote del terreno (Copernicus DEM, servizio Open-Meteo) e quelle dei file
GPX/KML sono sul livello del mare: gli strumenti le convertono in quote ellissoidiche con il geoide
EGM96 (in Italia la differenza è di circa 40–50 m).

**Servizi esterni.** Routing (server dimostrativi OSRM), ricerca dei luoghi (Nominatim) e quote del
terreno (Open-Meteo) sono servizi pubblici gratuiti con limiti d'uso: vanno bene per disegnare
percorsi, non per migliaia di richieste. Senza internet la pagina non mostra la mappa; lo script
funziona offline se il percorso ha già le quote, oppure con `--height` o `--no-dem`.

### Quanto è grande il file

| Configurazione | Velocità | 5 minuti |
|---|---|---|
| 5 MHz, 8 bit (HackRF) | 10 MB/s | circa 3 GB |
| 5 MHz, 16 bit (bladeRF, Pluto, USRP) | 20 MB/s | circa 6 GB |
| 15 MHz, 8 bit (HackRF, CBOC completa) | 30 MB/s | circa 9 GB |
| 15 MHz, 16 bit (bladeRF, USRP, CBOC completa) | 60 MB/s | circa 18 GB |
| 2,6 MHz, 8 bit | 5,2 MB/s | circa 1,6 GB |
| 2,6 MHz, 1 bit (`-b 1`, solo bladeplayer e limeplayer) | 0,65 MB/s | circa 0,2 GB |

### Cosa stampa il simulatore

All'avvio il simulatore stampa la tabella dei satelliti simulati; con `-v` la ristampa ogni 30 s:

```
G05  276.0  62.1  20655681.7   3.5   2.7
E08   57.4  68.2  23588862.3   3.5   2.1
```

Le colonne sono: satellite (G = GPS, E = Galileo), azimut e elevazione in gradi, distanza in metri,
ritardo ionosferico e ritardo troposferico in metri.

## 6. Trasmettere con un front-end SDR

In tutti gli esempi la frequenza di campionamento del player **deve essere uguale** a quella usata
per generare il file: 5 MHz con Galileo (default), 2,6 MHz per il solo GPS (default), oppure il
valore passato con `-s`. Se non coincidono, il ricevitore non aggancia nessun satellite.

### HackRF One

File a 8 bit:

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 300 -a -b 8
```

Trasmissione con `hackrf_transfer`, a 5 MHz:

```bash
hackrf_transfer -t gpssim.bin -f 1575420000 -s 5000000 -a 1 -x 0
```

- `-a 1` attiva l'amplificatore RF interno; `-x` è il guadagno TX in dB (0–47).
  Parti da `-x 0` e regola il livello con gli attenuatori (sezione 7).
- Con un file a 2,6 MHz (solo GPS, o `-s 2600000`) usa `-s 2600000`.
- Con un file a 15 MHz (CBOC completa, `-s 15000000 -b 8`) usa `-s 15000000`.
- `hackrf_transfer` trasmette il file una volta; con `-R` lo ripete a ciclo continuo, ma
  il tempo dei satelliti ricomincia da capo a ogni ripetizione e il ricevitore perde il fix.
- Con `player/hackplayer` (solo file a 2,6 MHz):

  ```bash
  ./hackplayer -t gpssim.bin
  ```

**Clock:** l'oscillatore standard dell'HackRF ha una tolleranza di circa 20 ppm, cioè fino a
30 kHz di errore a 1,5 GHz. Molti ricevitori agganciano lo stesso, ma la ricerca è più lenta e
alcuni falliscono. Per risultati affidabili usa un TCXO o un riferimento esterno a 10 MHz
(cartella `extclk/`).

### bladeRF

File a 16 bit (default):

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 300 -a
```

Script per `bladeRF-cli` (bladeRF prima generazione, x40/x115), da salvare come
`bladerf_5M.script`:

```
set frequency 1575.42M
set samplerate 5M
set bandwidth 5M
set txvga1 -25
cal lms
cal dc tx
tx config file=gpssim.bin format=bin
tx start
tx wait
```

```bash
bladeRF-cli -s bladerf_5M.script
```

Per la bladeRF 2.0 micro (xA4/xA9) frequenza, campionamento e guadagno si impostano per canale,
e i comandi `cal lms`, `cal dc tx` e `txvga1` (solo prima generazione) non vanno usati:

```
set frequency tx1 1575.42M
set samplerate tx1 5M
set bandwidth tx1 5M
set gain tx1 0
tx config file=gpssim.bin format=bin
tx start
tx wait
```

- Con un file a 2,6 MHz imposta `samplerate 2.6M` e `bandwidth 2.5M` (lo script `bladerf.script`
  del repo fa così). Con un file a 15 MHz (CBOC completa) imposta `samplerate 15M` e `bandwidth 15M`.
- Il guadagno (`txvga1` da −35 a −4 dB sulla prima generazione, `gain tx1` sulla 2.0) va regolato
  con gli attenuatori. Controlla l'intervallo valido sul tuo modello con `print gain`.
- `tx config` trasmette il file una volta; `repeat=0` lo ripete senza fine, con lo stesso problema
  di `hackrf_transfer -R` (il tempo ricomincia da capo).
- `bladeRF-cli` si aspetta valori nell'intervallo ±2047 e non li controlla. Con molti satelliti
  il simulatore può superarlo in rari campioni (meno dello 0,01% nelle prove con 23 satelliti);
  se serve, abbassa il livello con `-p` (potenza costante, per esempio `-p 64`).
- Con `player/bladeplayer` (solo file a 2,6 MHz, 16 o 1 bit):

  ```bash
  ./bladeplayer -f gpssim.bin -g -25
  ./bladeplayer -f gpssim.bin -b 1      # file generato con -b 1
  ```

### ADALM-Pluto

`plutoplayer` trasmette file a 16 bit a 2,6 MHz fissi, quindi il file va generato a 2,6 MHz:

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 300 -a -s 2600000
```

Il simulatore avvisa che sotto 4 MHz lo spettro di Galileo viene troncato; per il solo GPS
aggiungi `-C G` e l'avviso sparisce.

```bash
./plutoplayer -t gpssim.bin                  # Pluto raggiungibile come pluto.local
./plutoplayer -t gpssim.bin -a -40           # attenuazione TX -40 dB (da 0 a -80, default -20)
./plutoplayer -t gpssim.bin -n 192.168.2.1   # indirizzo IP
./plutoplayer -t gpssim.bin -u usb:1.2.5     # URI libiio (USB)
./plutoplayer -t gpssim.bin -b 3.0           # banda RF in MHz (da 1 a 5, default 3)
```

### LimeSDR

`limeplayer` legge lo stream da stdin e accetta qualsiasi frequenza di campionamento:

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 300 -a
./limeplayer -s 5000000 -b 16 -g 0.1 < gpssim.bin
```

- `-g` è il guadagno normalizzato da 0.0 a 1.0 (default 1.0, potenza massima): parti basso.
- Con un file a 1 bit (`-b 1` nel simulatore) usa `-b 1 -d 2047`, dove `-d` è la dinamica.
- `-c` sceglie il canale (0 o 1), `-a` l'antenna, `-i` il dispositivo se ce n'è più di uno.

### USRP (UHD)

Con lo script GNU Radio del repo, che ripete il file a ciclo continuo:

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 300 -a
./gnss-sdr-sim-uhd.py -t gpssim.bin -s 5000000 -b 16 -x 0
```

- `-b 8` per file generati con `-b 8`; `-x` è il guadagno TX; `-c external` usa un riferimento
  esterno a 10 MHz; `-a` passa argomenti al dispositivo (per esempio `-a "type=b200"`).
- La USRP2 richiede un decimatore pari di 100 MHz: 5 MHz (100/20) va bene, 2,6 MHz no (usa
  `-s 2500000` nel simulatore).

Con `tx_samples_from_file` degli esempi UHD:

```bash
tx_samples_from_file --file gpssim.bin --type short --rate 5000000 --freq 1575420000 --gain 0
```

### Generazione e trasmissione in contemporanea

Con `-o -` il simulatore scrive su stdout, e il file non serve più. La generazione deve essere più
veloce del tempo reale: su un portatile recente con circa 25 satelliti a 5 MHz lo è di circa due
volte e mezzo, ma su macchine più lente il player resta senza campioni e il segnale si interrompe.
A 15 MHz (CBOC completa) la generazione può essere più lenta del tempo reale: in quel caso genera
prima il file e poi trasmettilo.

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 3600 -a -b 8 -o - | hackrf_transfer -t - -f 1575420000 -s 5000000 -a 1 -x 0
```

```bash
./gnss-sdr-sim -D -T now -l 45.07,7.66,250 -d 3600 -a -o - | ./limeplayer -s 5000000 -b 16 -g 0.1
```

Il simulatore stampa i suoi messaggi su stderr, quindi non si mescolano ai campioni.

## 7. Collegamento al ricevitore e livelli di potenza

```
 SDR (TX) ── DC block ── attenuatore 50–60 dB ── ingresso antenna del ricevitore
```

- **DC block:** molti ricevitori portano sul connettore d'antenna la tensione per alimentare
  un'antenna attiva (3–5 V). Il DC block protegge l'uscita dell'SDR.
- **Attenuatori:** l'obiettivo è che ogni satellite arrivi al ricevitore con un livello realistico,
  circa −130 dBm, come all'uscita di un'antenna passiva. La potenza di uscita degli SDR varia molto
  con modello, frequenza e guadagno, quindi non esiste un valore fisso: parti con molta
  attenuazione e riducila gradualmente.
- **Come capire se il livello è giusto:** guarda il C/N0 riportato dal ricevitore, per esempio in
  u-center o nei messaggi NMEA GSV. Valori tra 40 e 50 dB-Hz sono realistici. Valori sopra 55
  dB-Hz indicano un segnale troppo forte: alcuni ricevitori segnalano jamming o spoofing.
- **Prima di partire:** fai un reset a freddo (cold start) del ricevitore, così non usa
  l'almanacco o la posizione memorizzati dal cielo vero.

Il primo fix arriva quando il ricevitore ha decodificato effemeridi e tempo, di solito dopo
30–60 s. Per tutto questo tempo il file deve continuare a essere trasmesso.

## 8. Riferimento delle opzioni

| Opzione | Significato |
|---|---|
| `-e <file>` | File RINEX di navigazione: RINEX 2 GPS, oppure RINEX 3 GPS/Galileo/misto |
| `-D` | Scarica da BKG le effemeridi del giorno dello scenario (al posto di `-e`) |
| `-C <sistemi>` | Costellazioni: `G` (GPS), `E` (Galileo), per esempio `GE`. Default: tutte quelle presenti nel file |
| `-l lat,lon,h` | Posizione statica in gradi e metri, per esempio `45.07,7.66,250` |
| `-c x,y,z` | Posizione statica ECEF in metri |
| `-u <file>` | Traiettoria ECEF a 10 Hz (CSV) |
| `-x <file>` | Traiettoria lat, lon, quota ellissoidica a 10 Hz (CSV), per esempio da `tools/route2motion.py` |
| `-g <file>` | Traiettoria da stream NMEA GGA |
| `-t <data,ora>` | Ora di inizio `AAAA/MM/GG,hh:mm:ss`, dentro l'intervallo del file |
| `-T <data,ora>` | Sposta le effemeridi all'ora indicata; `-T now` usa l'ora attuale |
| `-d <s>` | Durata in secondi (statica: 300 di default, massimo 86400; dinamica: tutto il file della traiettoria) |
| `-o <file>` | File di uscita (default `gpssim.bin`; `-` per stdout) |
| `-s <Hz>` | Frequenza di campionamento (default 2600000, 5000000 con Galileo; 15000000 per la CBOC completa) |
| `-b <bit>` | Formato I/Q: 1, 8 o 16 bit (default 16) |
| `-a` | Aggiunge il ritardo troposferico (modello Saastamoinen) |
| `-i` | Disattiva il ritardo ionosferico (scenari spaziali) |
| `-p [guadagno]` | Disattiva l'attenuazione con la distanza e tiene la potenza costante (1–128) |
| `-L wn,dn,dt` | Annuncia un salto di secondo intercalare futuro (settimana GPS, giorno, nuovo valore) |
| `-v` | Mostra i dettagli dei canali simulati |

## 9. Problemi frequenti

| Sintomo | Causa probabile e rimedio |
|---|---|
| Il ricevitore non vede nessun satellite | Frequenza di campionamento del player diversa da quella del file; formato sbagliato (8 bit per HackRF, 16 per gli altri); segnale troppo debole o troppo forte; alcuni ricevitori segnalano "antenna assente" se non sentono il carico di un'antenna attiva e vanno configurati per l'antenna passiva |
| Vede i satelliti ma non fa il fix | Aspetta almeno 60 s; fai un cold start; con HackRF l'errore del clock può essere troppo grande (usa un TCXO); con `hackrf_transfer -R` il file ricomincia da capo e il ricevitore perde il tempo |
| Il ricevitore segnala jamming o spoofing | Segnale troppo forte: aumenta l'attenuazione |
| Vede solo i satelliti GPS | File generato con `-C G` o da RINEX 2; ricevitore con Galileo disattivato nella configurazione (per esempio in u-center: UBX-CFG-GNSS); player a 2,6 MHz con ricevitori poco tolleranti |
| `ERROR: Invalid start time` | L'ora di `-t` non è coperta dal file: usa l'intervallo stampato, oppure `-T` |
| `ERROR: Failed to download the broadcast ephemerides` | Nessuna connessione, data futura, oppure file del giorno non ancora pubblicato: usa `-T now` o una data passata |
| Posizione con qualche metro di errore in quota | Il ricevitore corregge la troposfera ma il simulatore non la aggiunge: usa `-a` |
| `WARNING: Sampling frequency below 4 MHz truncates the Galileo E1 BOC(1,1) spectrum` | Con Galileo e `-s` sotto 4 MHz il segnale Galileo è degradato: usa 5 MHz, oppure `-C G` |

# Informe de Pràctica 7: Busos de Comunicació III (I2S)

**Autors:** Julio Lázaro Alcobendas i Gerard Rodríguez González
**Data:** 05 de Maig de 2026
**Repositori GitHub:** https://github.com/gedrar/Practica-7

---

# Exercici 1: Reproducció d'àudio AAC des de memòria interna (PROGMEM)

## 1. Objectius de la pràctica

L'objectiu d'aquest exercici és comprendre el funcionament del bus I2S i aplicar-lo per reproduir àudio digital des de la memòria de programa (PROGMEM) de l'ESP32-S3. S'utilitza el mòdul amplificador MAX98357A per convertir el senyal I2S digital a una senyal analògica apta per a un altaveu.

## 2. Especificacions (platformio.ini)

```ini
[env:esp32-s3-devkitc-1]
platform  = espressif32
board     = esp32-s3-devkitc-1
framework = arduino

monitor_speed = 115200
upload_speed  = 921600

lib_deps =
    earlephilhower/ESP8266Audio @ ^1.9.7

build_flags =
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
```

## 3. Desenvolupament i Arquitectura

El sistema utilitza la biblioteca `ESP8266Audio` d'Earle F. Philhower, compatible amb ESP32. La cadena de reproducció és:

```
PROGMEM (sampleaac.h)
    ↓  AudioFileSourcePROGMEM
Decodificador AAC
    ↓  AudioGeneratorAAC
Sortida I2S → MAX98357A → Altaveu
    ↓  AudioOutputI2S
```

Els pins I2S utilitzats per al MAX98357A:

| Senyal I2S | Pin ESP32-S3 | Pin MAX98357A |
|-----------|-------------|---------------|
| BCLK      | GPIO 4      | BCLK          |
| LRC (WS)  | GPIO 5      | LRC           |
| DOUT (SD) | GPIO 6      | DIN           |

## 4. Codi Principal (main.cpp)

```cpp
#include <Arduino.h>
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"

#define I2S_BCLK  4
#define I2S_LRC   5
#define I2S_DOUT  6

AudioFileSourcePROGMEM *fuente;
AudioGeneratorAAC       *decodificador;
AudioOutputI2S          *salida;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Practica 7 - Ejercicio 1 ===");
  Serial.println("Iniciando reproduccion AAC desde PROGMEM...");

  fuente = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));

  salida = new AudioOutputI2S();
  salida->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  salida->SetGain(0.125); // Volum: 0.0 (mut) – 4.0 (màxim)

  decodificador = new AudioGeneratorAAC();
  decodificador->begin(fuente, salida);

  Serial.println("Reproduccion iniciada.");
}

void loop() {
  if (decodificador->isRunning()) {
    if (!decodificador->loop()) {
      decodificador->stop();
      Serial.println("Reproduccion finalizada.");
    }
  } else {
    Serial.println("Generador de sonido detenido. Esperando 3 s...");
    delay(3000);

    fuente->seek(0, SEEK_SET); // Rebobina
    decodificador->begin(fuente, salida);
    Serial.println("Reiniciando reproduccion...");
  }
}
```

## 5. Funcionament del codi

Al `setup()`, es creen tres objectes:

- `AudioFileSourcePROGMEM`: apunta a l'array `sampleaac` emmagatzemat a la memòria de programa.
- `AudioOutputI2S`: configura el bus I2S amb els pins indicats i el guany de volum.
- `AudioGeneratorAAC`: descodificador AAC que conecta la font amb la sortida.

Al `loop()`, `decodificador->loop()` processa un fragment d'àudio per iteració. Quan el fitxer arriba al final (`isRunning()` retorna `false`), el programa espera 3 segons, rebobina la font amb `seek(0, SEEK_SET)` i reinicia la reproducció en bucle.

## 6. Sortida pel Monitor Sèrie

```
=== Practica 7 - Ejercicio 1 ===
Iniciando reproduccion AAC desde PROGMEM...
Reproduccion iniciada.
Reproduccion finalizada.
Generador de sonido detenido. Esperando 3 s...
Reiniciando reproduccion...
Reproduccion iniciada.
```

## 7. Diagrama de flux

```
Inici Programa
  ↓
Crear AudioFileSourcePROGMEM (sampleaac)
  ↓
Crear AudioOutputI2S (BCLK=4, LRC=5, DOUT=6, Gain=0.125)
  ↓
Crear AudioGeneratorAAC → begin(fuente, salida)
  ↓
Serial "Reproduccion iniciada"
  ↓
Bucle Infinit / loop()
  ↓
decodificador->isRunning()?
  Sí → decodificador->loop()
        → Retorna false? → stop() → "Finalizada"
  No  → delay(3000)
        → fuente->seek(0)
        → decodificador->begin()
        → "Reiniciando"
  ↻ (repeteix)
```

## 8. Preguntes de la pràctica

**Quina és la sortida pel port sèrie?** El programa imprimeix el missatge d'inici i, quan el fragment AAC emmagatzemat a PROGMEM finalitza, avisa per sèrie i reinicia la reproducció en bucle amb un delay de 3 segons entre repeticions.

**Explica el funcionament:** La biblioteca `ESP8266Audio` abstreu tota la complexitat del protocol I2S. El `decodificador->loop()` ha de cridar-se contínuament (és una màquina d'estats que processa l'àudio en petits fragments), per tant no s'ha d'usar `delay()` llarg dins del `loop()` mentre el so es reprodueix o es tallaria.

## 9. Conclusions

Aquest exercici ens ha introduït al bus I2S com a estàndard per a la transmissió d'àudio digital sense pèrdua de qualitat. La combinació ESP32-S3 + MAX98357A és una solució compacta i econòmica per afegir sortida d'àudio a qualsevol projecte. La biblioteca `ESP8266Audio` simplifica enormement la integració, oferint una interfície orientada a objectes clara.

---

# Exercici 2: Reproducció d'arxiu WAV des de targeta SD

## 1. Objectiu

Reproduir un arxiu d'àudio WAV emmagatzemat en una targeta microSD externa, llegint-lo via SPI i enviant-lo a l'altaveu via I2S a través del MAX98357A.

## 2. Especificacions (platformio.ini)

```ini
[env:esp32-s3-devkitc-1]
platform  = espressif32
board     = esp32-s3-devkitc-1
framework = arduino

monitor_speed = 115200
upload_speed  = 921600

lib_deps =
    https://github.com/schreibfaul1/ESP32-audioI2S.git#3.0.12
```

## 3. Desenvolupament i Arquitectura

En aquest exercici s'utilitzen dos busos de comunicació simultàniament:

- **SPI** → per llegir el fitxer WAV de la targeta SD.
- **I2S** → per enviar l'àudio decodificat al MAX98357A.

Assignació de pins:

| Perifèric | Senyal | Pin ESP32-S3 |
|-----------|--------|-------------|
| SD (SPI)  | CS     | GPIO 39     |
| SD (SPI)  | MOSI   | GPIO 35     |
| SD (SPI)  | MISO   | GPIO 37     |
| SD (SPI)  | SCK    | GPIO 36     |
| MAX98357A (I2S) | DOUT | GPIO 4  |
| MAX98357A (I2S) | BCLK | GPIO 6  |
| MAX98357A (I2S) | LRC  | GPIO 7  |

## 4. Codi Principal (main.cpp)

```cpp
#include <Arduino.h>
#include "Audio.h"
#include "SD.h"
#include "FS.h"

#define AUDIO_FILE "/audio.wav"

// Pins SPI per a la SD
#define SD_CS    39
#define SPI_MOSI 35
#define SPI_MISO 37
#define SPI_SCK  36

// Pins I2S per al MAX98357A
#define I2S_DOUT  4
#define I2S_BCLK  6
#define I2S_LRC   7

Audio audio;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Practica 7 - Ejercicio 2 ===");

  // Inicialitza SPI i SD
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR: No se pudo montar la tarjeta SD.");
    while (true) delay(1000);
  }
  Serial.println("Tarjeta SD montada correctamente.");

  // Configura I2S i volum
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(10); // 0 (mut) – 21 (màxim)

  // Verifica que el fitxer existeix
  if (!SD.exists(AUDIO_FILE)) {
    Serial.printf("ERROR: Archivo no encontrado: %s\n", AUDIO_FILE);
    while (true) delay(1000);
  }

  // Inicia reproducció
  audio.connecttoFS(SD, AUDIO_FILE);
  Serial.printf("Reproduciendo: %s\n", AUDIO_FILE);
}

void loop() {
  audio.loop(); // Ha de cridar-se contínuament
}

// Callbacks opcionals
void audio_info(const char *info)          { Serial.print("info       : "); Serial.println(info); }
void audio_id3data(const char *info)       { Serial.print("id3data    : "); Serial.println(info); }
void audio_eof_mp3(const char *info)       {
  Serial.println("Fin del archivo. Reiniciando en 3 s...");
  delay(3000);
  audio.connecttoFS(SD, AUDIO_FILE); // Bucle de reproducció
}
void audio_showstation(const char *info)   { Serial.print("station    : "); Serial.println(info); }
void audio_showstreaminfo(const char *info){ Serial.print("streaminfo : "); Serial.println(info); }
void audio_showstreamtitle(const char *info){ Serial.print("streamtitle: "); Serial.println(info); }
void audio_bitrate(const char *info)       { Serial.print("bitrate    : "); Serial.println(info); }
void audio_commercial(const char *info)    { Serial.print("commercial : "); Serial.println(info); }
void audio_icyurl(const char *info)        { Serial.print("icyurl     : "); Serial.println(info); }
void audio_lasthost(const char *info)      { Serial.print("lasthost   : "); Serial.println(info); }
void audio_eof_speech(const char *info)    { Serial.print("eof_speech : "); Serial.println(info); }
```

## 5. Funcionament del codi

Al `setup()`, s'inicialitza el bus SPI i es munta la targeta SD. Llavors es configuren els pins I2S de l'objecte `audio` i el nivell de volum. Abans de reproduir, es verifica que `audio.wav` existeix a l'arrel de la SD. `audio.connecttoFS()` connecta l'objecte d'àudio directament al sistema de fitxers de la SD i inicia la reproducció.

Al `loop()`, `audio.loop()` ha de cridar-se contínuament per mantenir el flux d'àudio. Si s'introduís un `delay()` llarg, el so es tallaria. El callback `audio_eof_mp3()` es crida automàticament quan el fitxer finalitza, reiniciant la reproducció en bucle.

## 6. Sortida pel Monitor Sèrie

```
=== Practica 7 - Ejercicio 2 ===
Tarjeta SD montada correctamente.
Reproduciendo: /audio.wav
info       : sampling rate: 44100  bits per sample: 16  num channels: 2
info       : format: WAV
bitrate    : 1411
info       : decode error
Fin del archivo. Reiniciando en 3 s...
Reproduciendo: /audio.wav
```

## 7. Diagrama de flux

```
Inici Programa
  ↓
SPI.begin(SCK=36, MISO=37, MOSI=35)
  ↓
SD.begin(CS=39)
  ↓
Error SD? → Aturar programa
  ↓
audio.setPinout(BCLK=6, LRC=7, DOUT=4)
audio.setVolume(10)
  ↓
SD.exists("/audio.wav")?
  No → Aturar programa
  Sí ↓
audio.connecttoFS(SD, "/audio.wav")
  ↓
Bucle Infinit / loop()
  ↓
audio.loop() → manté el flux d'àudio
  ↓ (callback quan acaba)
audio_eof_mp3() → delay(3000) → reconnectar
  ↻ (repeteix en bucle)
```

## 8. Diferències entre Exercici 1 i Exercici 2

| Característica | Exercici 1 (PROGMEM) | Exercici 2 (SD) |
|---------------|----------------------|-----------------|
| Font d'àudio | Array en memòria RAM | Fitxer WAV a SD |
| Biblioteca | ESP8266Audio | ESP32-audioI2S |
| Format | AAC | WAV (i MP3) |
| Capacitat màx. | Limitada per la RAM | Limitada per la SD |
| Maquinari extra | No (sol MAX98357A) | Sí (SD + MAX98357A) |
| Cas d'ús | Sons curts / efectes | Música llarga |

## 9. Preguntes de la pràctica

**Quina és la sortida pel port sèrie?** La biblioteca `ESP32-audioI2S` dispara callbacks automàticament amb informació del fitxer: sampling rate, bits per mostra, canals, bitrate i metadades ID3 si el fitxer en té. Quan el fitxer finalitza, es crida `audio_eof_mp3()` i es reinicia la reproducció.

**Explica el funcionament:** `audio.connecttoFS()` estableix un flux de dades des de la SD fins al decodificador intern. `audio.loop()` llegeix fragments del fitxer WAV, els descodifica i els envia al bus I2S, que el MAX98357A converteix a senyal analògica i amplifica per a l'altaveu.

## 10. Conclusions

Aquest exercici ens ha mostrat com integrar simultàniament els busos SPI i I2S per crear un reproductor d'àudio complet. La biblioteca `ESP32-audioI2S` és molt més potent que `ESP8266Audio`: suporta WAV, MP3, AAC, streaming per URL i ofereix un sistema de callbacks ric per monitorar l'estat de la reproducció. El resultat és un reproductor d'àudio de qualitat HiFi (44,1 kHz, 16 bits estèreo) en un microcontrolador del tamany d'un polze.

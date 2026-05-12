#include <Arduino.h>
#include "Audio.h"
#include "SD.h"
#include "FS.h"

// ── Nombre del archivo de audio en la SD ────────────────────
#define AUDIO_FILE  "/audio.wav"
// ────────────────────────────────────────────────────────────

// ── Pines SPI para el módulo SD ─────────────────────────────
#define SD_CS       39
#define SPI_MOSI    35
#define SPI_MISO    37
#define SPI_SCK     36
// ────────────────────────────────────────────────────────────

// ── Pines I2S para el MAX98357A ─────────────────────────────
#define I2S_DOUT    4   // Serial Data  → MAX98357A DIN
#define I2S_BCLK    6   // Bit Clock    → MAX98357A BCLK
#define I2S_LRC     7   // Word Select  → MAX98357A LRC
// ────────────────────────────────────────────────────────────

Audio audio;

void setup() {
    Serial.begin(115200);
    Serial.println("=== Practica 7 - Ejercicio 2 ===");

    // Inicializa bus SPI para la tarjeta SD
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    // Monta la tarjeta SD
    if (!SD.begin(SD_CS)) {
        Serial.println("ERROR: No se pudo montar la tarjeta SD.");
        Serial.println("Comprueba el cableado y el formato (FAT32).");
        while (true) delay(1000);   // Detiene la ejecución
    }
    Serial.println("Tarjeta SD montada correctamente.");

    // Configura pines I2S y volumen
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10);   // Rango: 0 (mudo) – 21 (máximo)

    // Verifica que el archivo exista antes de reproducir
    if (!SD.exists(AUDIO_FILE)) {
        Serial.printf("ERROR: Archivo no encontrado: %s\n", AUDIO_FILE);
        Serial.println("Copia el archivo WAV en la raiz de la SD.");
        while (true) delay(1000);
    }

    // Inicia reproducción
    audio.connecttoFS(SD, AUDIO_FILE);
    Serial.printf("Reproduciendo: %s\n", AUDIO_FILE);
}

void loop() {
    audio.loop();   // Debe llamarse continuamente para mantener el flujo de audio
}

// ── Callbacks opcionales: información por puerto serie ───────

void audio_info(const char *info) {
    Serial.print("info       : ");
    Serial.println(info);
}

void audio_id3data(const char *info) {   // Metadatos ID3
    Serial.print("id3data    : ");
    Serial.println(info);
}

void audio_eof_mp3(const char *info) {   // Fin de archivo
    Serial.print("eof_mp3    : ");
    Serial.println(info);
    Serial.println("Fin del archivo. Reiniciando en 3 s...");
    delay(3000);
    audio.connecttoFS(SD, AUDIO_FILE);   // Reproduce en bucle
}

void audio_showstation(const char *info) {
    Serial.print("station    : ");
    Serial.println(info);
}

void audio_showstreaminfo(const char *info) {
    Serial.print("streaminfo : ");
    Serial.println(info);
}

void audio_showstreamtitle(const char *info) {
    Serial.print("streamtitle: ");
    Serial.println(info);
}

void audio_bitrate(const char *info) {
    Serial.print("bitrate    : ");
    Serial.println(info);
}

void audio_commercial(const char *info) {   // Duración en segundos
    Serial.print("commercial : ");
    Serial.println(info);
}

void audio_icyurl(const char *info) {   // URL de la emisora
    Serial.print("icyurl     : ");
    Serial.println(info);
}

void audio_lasthost(const char *info) {   // URL del stream reproducido
    Serial.print("lasthost   : ");
    Serial.println(info);
}

void audio_eof_speech(const char *info) {
    Serial.print("eof_speech : ");
    Serial.println(info);
}

#include <Arduino.h>
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"

// ── Pines I2S ───────────────────────────────────────────────
#define I2S_BCLK  4
#define I2S_LRC   5
#define I2S_DOUT  6
// ────────────────────────────────────────────────────────────

AudioFileSourcePROGMEM *fuente;
AudioGeneratorAAC       *decodificador;
AudioOutputI2S          *salida;

void setup() {
    Serial.begin(115200);
    Serial.println("=== Practica 7 - Ejercicio 1 ===");
    Serial.println("Iniciando reproduccion AAC desde PROGMEM...");

    // Fuente: array en memoria de programa
    fuente = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));

    // Salida I2S
    salida = new AudioOutputI2S();
    salida->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    salida->SetGain(0.125);   // Volumen: 0.0 (mudo) – 4.0 (máximo)

    // Decodificador AAC
    decodificador = new AudioGeneratorAAC();
    decodificador->begin(fuente, salida);

    Serial.println("Reproduccion iniciada.");
}

void loop() {
    if (decodificador->isRunning()) {
        // Procesa el siguiente trozo de audio
        if (!decodificador->loop()) {
            decodificador->stop();
            Serial.println("Reproduccion finalizada.");
        }
    } else {
        // El decodificador se ha detenido: espera y reinicia
        Serial.println("Generador de sonido detenido. Esperando 3 s...");
        delay(3000);

        // Rebobina y vuelve a reproducir
        fuente->seek(0, SEEK_SET);
        decodificador->begin(fuente, salida);
        Serial.println("Reiniciando reproduccion...");
    }
}

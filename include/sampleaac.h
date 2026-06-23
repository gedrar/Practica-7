#ifndef SAMPLEAAC_H
#define SAMPLEAAC_H

#include <Arduino.h>

// Este es un ejemplo de cómo luce el array. 
// Debes asegurarte de que el nombre de la variable sea 'sampleaac'
const unsigned char sampleaac[] PROGMEM = {
    0x21, 0x12, 0x0A, 0x00, 0x00, 0x00, // ... miles de bytes más de tu archivo de audio
};

#endif // SAMPLEAAC_H
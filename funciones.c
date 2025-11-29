funciones.h
#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//pines
#define PIN_BOTON 18
#define PIN_HUMIDIFICADOR 23
#define PIN_VENTILADOR 19
#define PIN_BUZZER 15
#define PIN_SENSOR_NIVEL 32

//oled
#define OLED_ANCHO 128
#define OLED_ALTO 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

//tiempos
#define TIEMPO_DEBOUNCE 300UL
#define PERIODO_LECTURA 50UL
#define ALARMA_REPETICIONES 5

//enum de estados
enum EstadoSistema {
  ESTADO_INICIO,
  ESTADO_NEBULIZANDO,
  ESTADO_BLOQUEADO
};

//funciones
void setupSistema();
void loopSistema();
bool botonPresionado();
bool sensorSinAgua();
void iniciarNebulizacion();
void detenerNebulizacion();
void mostrarPantallaInicial();
void mostrarPantallaNebulizando();
void mostrarPantallaSinAgua();
void mostrarPantallaBloqueado();
void activarAlarmaNoBloqueante();
void alarmTick();

#endif
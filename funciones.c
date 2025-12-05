FUNCIONES.H

#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED
#define OLED_ANCHO 128
#define OLED_ALTO 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

// Pines
#define PIN_BOTON           18
#define PIN_HUMIDIFICADOR   23
#define PIN_VENTILADOR      19
#define PIN_BUZZER          15
#define PIN_SENSOR_NIVEL    32

// Tiempos
#define TIEMPO_DEBOUNCE 300UL
#define PERIODO_FSM     50UL
#define ALARMA_REPETICIONES 5

// Estados
enum EstadoSistema {
  ESTADO_INICIO,
  ESTADO_NEBULIZANDO,
  ESTADO_BLOQUEADO
};

extern EstadoSistema estadoActual;
extern Adafruit_SSD1306 oled;

extern bool humidificadorActivo;
extern bool bloqueoActivo;
extern bool nebulizandoActivo;

extern unsigned long ultimoTick;

// Prototipos
void inicializarHardware();
void leerBoton();
bool botonPresionado();
bool sensorSinAgua();

void controlarFSM();
void iniciarNebulizacion();
void detenerNebulizacion();

// Pantallas
void mostrarPantallaInicial();
void mostrarPantallaNebulizando();
void mostrarPantallaSinAgua();
void mostrarPantallaBloqueado();

// Animación
void animacionWarning();

// Alarma
void activarAlarma();
void actualizarAlarma();

#endif
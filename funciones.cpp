funciones.cpp
#include "funciones.h"

//variable global de estado
EstadoSistema estadoActual = ESTADO_INICIO;

//oled
Adafruit_SSD1306 oled(OLED_ANCHO, OLED_ALTO, &Wire, OLED_RESET);

//variables de debounce y temporizacion
static bool ultimoEstadoBoton = HIGH;
static unsigned long ultimoCambioBoton = 0ULL;
static unsigned long ultimoTick = 0ULL;

//variables de estado
static bool HUMIFICADOR_ACTIVO = false;
static bool NEBULIZANDO_ACTIVO = false;
static bool BLOQUEADO_ALARMA = false;

//alarma no bloqueante
static bool alarmaActiva = false;
static int alarmaPaso = 0;
static int alarmaContadorRep = 0;
static unsigned long alarmaNextMillis = 0ULL;

//setup del sistema
void setupSistema() {
  Serial.begin(115200);

  Wire.begin(21,22);
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);

  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_HUMIDIFICADOR, OUTPUT);
  pinMode(PIN_VENTILADOR, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_SENSOR_NIVEL, INPUT);

  digitalWrite(PIN_HUMIDIFICADOR, LOW);
  digitalWrite(PIN_VENTILADOR, LOW);
  noTone(PIN_BUZZER);

  mostrarPantallaInicial();

  ultimoCambioBoton = 0;
  ultimoTick = millis();
}

//loop principal no bloqueante
void loopSistema() {
  unsigned long ahora = millis();

  if (ahora - ultimoTick < PERIODO_LECTURA) {
    alarmTick();
    return;
  }
  ultimoTick = ahora;

  if (botonPresionado()) {
    if (BLOQUEADO_ALARMA) {
      BLOQUEADO_ALARMA = false;
      HUMIFICADOR_ACTIVO = false;
      NEBULIZANDO_ACTIVO = false;
      estadoActual = ESTADO_INICIO;
      mostrarPantallaInicial();
    } else {
      HUMIFICADOR_ACTIVO = !HUMIFICADOR_ACTIVO;
      if (HUMIFICADOR_ACTIVO) {
        estadoActual = ESTADO_NEBULIZANDO;
        iniciarNebulizacion();
        mostrarPantallaNebulizando();
      } else {
        detenerNebulizacion();
        estadoActual = ESTADO_INICIO;
        mostrarPantallaInicial();
      }
    }
  }

  if (!BLOQUEADO_ALARMA && HUMIFICADOR_ACTIVO && NEBULIZANDO_ACTIVO) {
    if (sensorSinAgua()) {
      activarAlarmaNoBloqueante();
      BLOQUEADO_ALARMA = true;
      HUMIFICADOR_ACTIVO = false;
      NEBULIZANDO_ACTIVO = false;
      detenerNebulizacion();
      estadoActual = ESTADO_BLOQUEADO;
      mostrarPantallaSinAgua();
    }
  }

  if (!HUMIFICADOR_ACTIVO && !NEBULIZANDO_ACTIVO && !BLOQUEADO_ALARMA) {
    if (estadoActual != ESTADO_INICIO) {
      estadoActual = ESTADO_INICIO;
      mostrarPantallaInicial();
    }
  }

  alarmTick();
}

//boton con debounce
bool botonPresionado() {
  bool lectura = digitalRead(PIN_BOTON);
  unsigned long ahora = millis();

  if (lectura == LOW && ultimoEstadoBoton == HIGH && (ahora - ultimoCambioBoton) > TIEMPO_DEBOUNCE) {
    ultimoCambioBoton = ahora;
    ultimoEstadoBoton = lectura;
    return true;
  }

  ultimoEstadoBoton = lectura;
  return false;
}

//sensor de nivel
bool sensorSinAgua() {
  int lectura = digitalRead(PIN_SENSOR_NIVEL);
  return (lectura == LOW);
}

//actuadores
void iniciarNebulizacion() {
  digitalWrite(PIN_HUMIDIFICADOR, HIGH);
  digitalWrite(PIN_VENTILADOR, HIGH);
  NEBULIZANDO_ACTIVO = true;
}

void detenerNebulizacion() {
  digitalWrite(PIN_HUMIDIFICADOR, LOW);
  digitalWrite(PIN_VENTILADOR, LOW);
  noTone(PIN_BUZZER);
  NEBULIZANDO_ACTIVO = false;
}

//pantallas
void mostrarPantallaInicial() {
  oled.clearDisplay();
  oled.setCursor(0,0);
  oled.println("Presione el boton");
  oled.println("para iniciar la");
  oled.println("nebulizacion");
  oled.display();
}

void mostrarPantallaNebulizando() {
  oled.clearDisplay();
  oled.setCursor(0,0);
  oled.println("Nebulizando...");
  oled.println("Proceso activo");
  oled.display();
}

void mostrarPantallaSinAgua() {
  oled.clearDisplay();
  oled.setCursor(0,0);
  oled.println("SIN AGUA!");
  oled.println("Recargar deposito");
  oled.println("y presionar boton");
  oled.display();
}

void mostrarPantallaBloqueado() {
  oled.clearDisplay();
  oled.setCursor(0,0);
  oled.println("Sistema Bloqueado");
  oled.println("Presione boton");
  oled.display();
}

//alarma no bloqueante
void activarAlarmaNoBloqueante() {
  alarmaActiva = true;
  alarmaPaso = 0;
  alarmaContadorRep = 0;
  alarmaNextMillis = millis();
}

void alarmTick() {
  if (!alarmaActiva) return;

  unsigned long ahora = millis();
  if (ahora < alarmaNextMillis) return;

  if (alarmaPaso == 0) {
    tone(PIN_BUZZER,3000);
    alarmaNextMillis = ahora + 350UL;
    alarmaPaso = 1;
  } else if (alarmaPaso == 1) {
    noTone(PIN_BUZZER);
    alarmaNextMillis = ahora + 100UL;
    alarmaPaso = 2;
  } else if (alarmaPaso == 2) {
    tone(PIN_BUZZER,1800);
    alarmaNextMillis = ahora + 250UL;
    alarmaPaso = 3;
  } else if (alarmaPaso == 3) {
    noTone(PIN_BUZZER);
    alarmaContadorRep++;
    if (alarmaContadorRep >= ALARMA_REPETICIONES) {
      alarmaActiva = false;
      alarmaPaso = 0;
      alarmaContadorRep = 0;
      noTone(PIN_BUZZER);
    } else {
      alarmaNextMillis = ahora + 300UL;
      alarmaPaso = 0;
    }
  }
}
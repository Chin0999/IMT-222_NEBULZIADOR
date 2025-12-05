funciones.cpp

#include "funciones.h"

// ---------------- OBJETO OLED ----------------
Adafruit_SSD1306 oled(OLED_ANCHO, OLED_ALTO, &Wire, OLED_RESET);

// ---------------- ESTADOS ----------------
EstadoSistema estadoActual = ESTADO_INICIO;

bool humidificadorActivo = false;
bool bloqueoActivo = false;
bool nebulizandoActivo = false;
unsigned long ultimoTick = 0;

// ---------------- BOTÓN ----------------
static bool ultimoEstadoBoton = HIGH;
static unsigned long ultimoCambio = 0;

// ---------------- ALARMA ----------------
static bool alarmaActiva = false;
static int alarmaPaso = 0;
static int alarmaReps = 0;
static unsigned long alarmaProx = 0;

// ---------------- ANIMACIÓN ----------------
unsigned long animacionProxFrame = 0;
int animacionPaso = 0;

// ------------------------------------------------------
// INICIALIZAR HARDWARE
// ------------------------------------------------------
void inicializarHardware() {
  Serial.begin(115200);

  Wire.begin(21, 22);
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

  ultimoTick = millis();
}

// ------------------------------------------------------
// BOTÓN
// ------------------------------------------------------
void leerBoton() {
  if (botonPresionado()) {

    if (bloqueoActivo) {
      bloqueoActivo = false;
      humidificadorActivo = false;
      nebulizandoActivo = false;
      estadoActual = ESTADO_INICIO;
      mostrarPantallaInicial();
      return;
    }

    humidificadorActivo = !humidificadorActivo;

    if (humidificadorActivo) {
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

bool botonPresionado() {
  unsigned long ahora = millis();
  bool lectura = digitalRead(PIN_BOTON);

  if (lectura == LOW && ultimoEstadoBoton == HIGH &&
      (ahora - ultimoCambio) > TIEMPO_DEBOUNCE) {

    ultimoCambio = ahora;
    ultimoEstadoBoton = lectura;
    return true;
  }

  ultimoEstadoBoton = lectura;
  return false;
}

// ------------------------------------------------------
// SENSOR NIVEL
// ------------------------------------------------------
bool sensorSinAgua() {
  return digitalRead(PIN_SENSOR_NIVEL) == LOW;
}

// ------------------------------------------------------
// FSM
// ------------------------------------------------------
void controlarFSM() {
  unsigned long ahora = millis();
  if (ahora - ultimoTick < PERIODO_FSM) return;
  ultimoTick = ahora;

  if (estadoActual == ESTADO_NEBULIZANDO &&
      humidificadorActivo && nebulizandoActivo) {

    if (sensorSinAgua()) {
      activarAlarma();
      bloqueoActivo = true;
      humidificadorActivo = false;
      nebulizandoActivo = false;

      detenerNebulizacion();
      estadoActual = ESTADO_BLOQUEADO;

      mostrarPantallaSinAgua();
    }
  }
}

// ------------------------------------------------------
// ACTUADORES
// ------------------------------------------------------
void iniciarNebulizacion() {
  digitalWrite(PIN_HUMIDIFICADOR, HIGH);
  digitalWrite(PIN_VENTILADOR, HIGH);
  nebulizandoActivo = true;
}

void detenerNebulizacion() {
  digitalWrite(PIN_HUMIDIFICADOR, LOW);
  digitalWrite(PIN_VENTILADOR, LOW);
  noTone(PIN_BUZZER);
  nebulizandoActivo = false;
}

// ------------------------------------------------------
// PANTALLAS
// ------------------------------------------------------
void mostrarPantallaInicial() {
  oled.clearDisplay();

  oled.setTextSize(2);
  oled.setCursor(5, 6);
  oled.print("BIENVENIDO");

  oled.setTextSize(1);
  oled.setCursor(10, 24);
  oled.print("Listo para iniciar");

  oled.setCursor(26, 33);
  oled.print("nebulizacion");

  oled.drawRect(2, 45, 121, 16, 1);
  oled.setCursor(12, 49);
  oled.print("PRESIONE EL BOTON");

  oled.display();
}

void mostrarPantallaNebulizando() {
  oled.clearDisplay();
  oled.setCursor(2, 2);
  oled.println("Nebulizando...");
  oled.println("Proceso activo");
  oled.display();
}

void mostrarPantallaSinAgua() {
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("SIN AGUA!");
  oled.println("Recargar deposito");
  oled.println("y presionar boton");
  oled.display();
}

void mostrarPantallaBloqueado() {
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Sistema Bloqueado");
  oled.println("Presione boton");
  oled.display();
}

// ------------------------------------------------------
// ANIMACIÓN – SIN BORRAR EL TEXTO
// ------------------------------------------------------
void animacionWarning() {
  if (estadoActual != ESTADO_BLOQUEADO) return;

  unsigned long ahora = millis();
  if (ahora < animacionProxFrame) return;

  // SOLO BORRA LA ZONA INFERIOR, NO EL TEXTO
  oled.fillRect(0, 32, 128, 32, BLACK);

  switch (animacionPaso) {
    case 0:
      oled.fillTriangle(64, 40, 44, 70, 84, 70, WHITE);
      break;

    case 1:
      oled.fillTriangle(64, 38, 45, 72, 83, 72, WHITE);
      break;

    case 2:
      oled.fillTriangle(64, 42, 42, 68, 86, 68, WHITE);
      break;
  }

  // Signo !
  oled.fillRect(62, 50, 4, 10, BLACK);
  oled.fillRect(62, 63, 4, 4, BLACK);

  oled.display();

  animacionPaso++;
  if (animacionPaso > 2) animacionPaso = 0;
  animacionProxFrame = ahora + 200;
}

// ------------------------------------------------------
// ALARMA
// ------------------------------------------------------
void activarAlarma() {
  alarmaActiva = true;
  alarmaPaso = 0;
  alarmaReps = 0;
  alarmaProx = millis();
}

void actualizarAlarma() {
  if (!alarmaActiva) return;

  unsigned long ahora = millis();
  if (ahora < alarmaProx) return;

  switch (alarmaPaso) {
    case 0:
      tone(PIN_BUZZER, 3000);
      alarmaProx = ahora + 350;
      alarmaPaso = 1;
      break;

    case 1:
      noTone(PIN_BUZZER);
      alarmaProx = ahora + 100;
      alarmaPaso = 2;
      break;

    case 2:
      tone(PIN_BUZZER, 1800);
      alarmaProx = ahora + 250;
      alarmaPaso = 3;
      break;

    case 3:
      noTone(PIN_BUZZER);
      alarmaReps++;

      if (alarmaReps >= ALARMA_REPETICIONES) {
        alarmaActiva = false;
      } else {
        alarmaProx = ahora + 300;
        alarmaPaso = 0;
      }
      break;
  }
}
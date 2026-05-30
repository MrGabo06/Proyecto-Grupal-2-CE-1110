// =======================
// L293D
// =======================
const int ENA = 9;   // PWM
const int IN1 = 7;
const int IN2 = 8;

// =======================
// Botones de pisos
// Cada boton va entre pin y GND
// =======================
const int botonesPiso[5] = {
  2,  // Piso 1
  3,  // Piso 2
  4,  // Piso 3
  5,  // Piso 4
  6   // Piso 5
};

// =======================
// Potenciometro
// =======================
const int potPin = A0;

// =======================
// Finales de carrera
// COM -> GND
// NO  -> pin
// =======================
const int switchAbajoPin = 11;
const int switchArribaPin = 12;

// =======================
// Referencias de pisos
// Ajustar con tus valores reales
// =======================
const int pisoRef[5] = {
  90,  // Piso 1
  120,   // Piso 2
  180,  // Piso 3
  250,  // Piso 4
  350   // Piso 5
};

// =======================
// PID
// =======================
float Kp = 0.7;
float Ki = 0.0;
float Kd = 0.25;

float integral = 0.0;
float errorAnterior = 0.0;

unsigned long lastPidTime = 0;
const unsigned long Ts_ms = 50;

// =======================
// Motor
// =======================
const int PWM_MIN = 90;          // velocidad minima normal
const int PWM_MAX = 180;         // velocidad maxima reducida
const int PWM_CERCA = 75;        // velocidad cuando esta cerca del piso
const int PWM_LIBERACION = 150;  // velocidad para liberar final de carrera

const int TOLERANCIA = 5;        // error aceptable para detenerse
const int ZONA_LENTA = 30;       // si el error es menor a esto, baja velocidad

// =======================
// Estado
// =======================
int pisoObjetivo = -1;
bool enMovimiento = false;
bool liberandoFinal = false;

int ultimoPWM = 0;
int ultimaDireccion = 0; // -1 menor, 0 stop, 1 mayor

unsigned long lastPrintTime = 0;
const unsigned long printInterval = 200;

// Debounce botones
bool ultimoEstadoBoton[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long ultimoTiempoBoton[5] = {0, 0, 0, 0, 0};
const unsigned long debounceDelay = 80;

void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(switchAbajoPin, INPUT_PULLUP);
  pinMode(switchArribaPin, INPUT_PULLUP);

  for (int i = 0; i < 5; i++) {
    pinMode(botonesPiso[i], INPUT_PULLUP);
  }

  detenerMotor();

  Serial.println("Sistema PID listo");
  Serial.println("Botones:");
  Serial.println("D2 = piso 1");
  Serial.println("D3 = piso 2");
  Serial.println("D4 = piso 3");
  Serial.println("D5 = piso 4");
  Serial.println("D6 = piso 5");
  Serial.println("Tambien puede escribir 1,2,3,4,5 por Serial");
}

void loop() {
  leerBotones();
  leerSerial();

  int posicionActual = leerPotPromediado();

  bool switchAbajo = digitalRead(switchAbajoPin) == LOW;
  bool switchArriba = digitalRead(switchArribaPin) == LOW;

  imprimirEstado(posicionActual, switchAbajo, switchArriba);

  // Finales de carrera:
  // Si se presiona uno, se mueve en direccion contraria
  // hasta que se libere. Luego se detiene.
  if (manejarFinalesCarrera(switchAbajo, switchArriba)) {
    return;
  }

  if (!enMovimiento || pisoObjetivo == -1) {
    detenerMotor();
    return;
  }

  ejecutarPID(posicionActual);
}

// =======================
// Finales de carrera
// =======================
bool manejarFinalesCarrera(bool switchAbajo, bool switchArriba) {
  if (switchAbajo || switchArriba) {
    enMovimiento = false;
    pisoObjetivo = -1;
    resetPID();

    if (!liberandoFinal) {
      detenerMotor();
      delay(80);
      liberandoFinal = true;
      Serial.println("Final de carrera activado: liberando en direccion contraria");
    }

    if (switchAbajo && !switchArriba) {
      moverHaciaPotMayor(PWM_LIBERACION);
    } 
    else if (switchArriba && !switchAbajo) {
      moverHaciaPotMenor(PWM_LIBERACION);
    } 
    else {
      detenerMotor();
    }

    return true;
  }

  if (liberandoFinal) {
    detenerMotor();
    liberandoFinal = false;
    Serial.println("Final de carrera liberado: motor detenido");
    delay(200);
    return true;
  }

  return false;
}

// =======================
// Lectura de botones
// =======================
void leerBotones() {
  for (int i = 0; i < 5; i++) {
    bool lectura = digitalRead(botonesPiso[i]);

    if (lectura != ultimoEstadoBoton[i]) {
      ultimoTiempoBoton[i] = millis();
      ultimoEstadoBoton[i] = lectura;
    }

    if ((millis() - ultimoTiempoBoton[i]) > debounceDelay) {
      if (lectura == LOW) {
        seleccionarPiso(i + 1);

        while (digitalRead(botonesPiso[i]) == LOW) {
          delay(10);
        }

        ultimoEstadoBoton[i] = HIGH;
      }
    }
  }
}

// =======================
// Lectura Serial
// =======================
void leerSerial() {
  if (Serial.available() > 0) {
    char entrada = Serial.read();

    if (entrada >= '1' && entrada <= '5') {
      seleccionarPiso(entrada - '0');
    }

    while (Serial.available() > 0) {
      Serial.read();
    }
  }
}

void seleccionarPiso(int piso) {
  pisoObjetivo = piso;
  enMovimiento = true;
  liberandoFinal = false;
  resetPID();

  Serial.print("Nuevo piso objetivo: ");
  Serial.print(pisoObjetivo);
  Serial.print(" | Referencia: ");
  Serial.println(pisoRef[pisoObjetivo - 1]);
}

// =======================
// PID
// =======================
void ejecutarPID(int posicionActual) {
  unsigned long ahora = millis();

  if (ahora - lastPidTime < Ts_ms) {
    return;
  }

  float dt = (ahora - lastPidTime) / 1000.0;
  lastPidTime = ahora;

  int referencia = pisoRef[pisoObjetivo - 1];
  float error = referencia - posicionActual;

  if (abs(error) <= TOLERANCIA) {
    detenerMotor();
    enMovimiento = false;
    pisoObjetivo = -1;
    resetPID();

    Serial.println("Llego al piso objetivo");
    return;
  }

  integral += error * dt;
  integral = constrain(integral, -300.0, 300.0);

  float derivada = (error - errorAnterior) / dt;
  errorAnterior = error;

  float salidaPID = Kp * error + Ki * integral + Kd * derivada;

  aplicarSalidaMotor(salidaPID, abs((int)error));
}

void aplicarSalidaMotor(float salidaPID, int errorAbs) {
  int pwm;

  if (errorAbs <= ZONA_LENTA) {
    pwm = PWM_CERCA;
  } else {
    pwm = abs((int)salidaPID);
    pwm = constrain(pwm, PWM_MIN, PWM_MAX);
  }

  if (salidaPID > 0) {
    moverHaciaPotMayor(pwm);
  } else {
    moverHaciaPotMenor(pwm);
  }
}

// =======================
// Potenciometro
// =======================
int leerPotPromediado() {
  long suma = 0;

  for (int i = 0; i < 10; i++) {
    suma += analogRead(potPin);
    delay(2);
  }

  return suma / 10;
}

// =======================
// Motor
// =======================
void moverHaciaPotMayor(int pwm) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, pwm);

  ultimoPWM = pwm;
  ultimaDireccion = 1;
}

void moverHaciaPotMenor(int pwm) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, pwm);

  ultimoPWM = pwm;
  ultimaDireccion = -1;
}

void detenerMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  ultimoPWM = 0;
  ultimaDireccion = 0;
}

// =======================
// Utilidades
// =======================
void resetPID() {
  integral = 0.0;
  errorAnterior = 0.0;
  lastPidTime = millis();
}

void imprimirEstado(int posicionActual, bool switchAbajo, bool switchArriba) {
  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();

    Serial.print("Pot: ");
    Serial.print(posicionActual);

    Serial.print(" | Piso objetivo: ");
    Serial.print(pisoObjetivo);

    if (pisoObjetivo >= 1 && pisoObjetivo <= 5) {
      int ref = pisoRef[pisoObjetivo - 1];

      Serial.print(" | Ref: ");
      Serial.print(ref);

      Serial.print(" | Error: ");
      Serial.print(ref - posicionActual);
    }

    Serial.print(" | PWM: ");
    Serial.print(ultimoPWM);

    Serial.print(" | Dir: ");

    if (ultimaDireccion == 1) {
      Serial.print("POT MAYOR");
    } else if (ultimaDireccion == -1) {
      Serial.print("POT MENOR");
    } else {
      Serial.print("STOP");
    }

    Serial.print(" | SW abajo: ");
    Serial.print(switchAbajo ? "ON" : "OFF");

    Serial.print(" | SW arriba: ");
    Serial.println(switchArriba ? "ON" : "OFF");
  }
}
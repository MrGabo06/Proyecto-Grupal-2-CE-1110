// L293D
const int ENA = 9;   // PWM
const int IN1 = 7;
const int IN2 = 8;

// Botones de pisos
const int botonesPiso[5] = {
  2,  // Piso 1
  3,  // Piso 2
  4,  // Piso 3
  5,  // Piso 4
  6   // Piso 5
};

// Potenciometro
const int potPin = A0;

// Finales de carrera
const int switchAbajoPin = 11;
const int switchArribaPin = 12;

// Referencias de pisos
const int pisoRef[5] = {
  290,  // Piso 1
  350,  // Piso 2
  450,  // Piso 3
  530,  // Piso 4
  600   // Piso 5
};

// PID
float Kp = 0.7;
float Ki = 0.0;
float Kd = 0.25;

float integral = 0.0;
float errorAnterior = 0.0;
bool primerCicloPID = true;

float ultimoError = 0.0;
float ultimoP = 0.0;
float ultimoI = 0.0;
float ultimoD = 0.0;
float ultimaSalidaPID = 0.0;

unsigned long lastPidTime = 0;
const unsigned long Ts_ms = 50;

// Motor
const int PWM_MIN = 90;
const int PWM_MAX = 180;
const int PWM_CERCA = 75;

// Motor bajando
const int PWM_BAJANDO_MIN = 90;
const int PWM_BAJANDO_MAX = 130;
const int PWM_BAJANDO_CERCA = 85;

// Liberacion
const int PWM_LIBERACION = 150;

// Control
const int TOLERANCIA = 5;
const int ZONA_LENTA = 20;

// Estado
int pisoObjetivo = -1;
bool enMovimiento = false;
bool liberandoFinal = false;

int ultimoPWM = 0;
int ultimaDireccion = 0; // -1 menor, 0 stop, 1 mayor

// Pruebas
int numeroPrueba = 0;
unsigned long tiempoInicioMovimiento = 0;

unsigned long lastPrintTime = 0;
const unsigned long printInterval = 200;

// Debounce
bool ultimaLecturaBoton[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool estadoEstableBoton[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
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

  Serial.println();
  Serial.println("CSV_RESULTADOS_PID:");
  Serial.println("prueba,piso_objetivo,referencia,posicion_final,error_final,tiempo_ms,Kp,Ki,Kd,Ts_ms,resultado");
}

void loop() {
  leerBotones();
  leerSerial();

  int posicionActual = leerPotPromediado();

  bool switchAbajo = digitalRead(switchAbajoPin) == LOW;
  bool switchArriba = digitalRead(switchArribaPin) == LOW;

  imprimirEstado(posicionActual, switchAbajo, switchArriba);

  if (manejarFinalesCarrera(switchAbajo, switchArriba)) {
    return;
  }

  if (!enMovimiento || pisoObjetivo == -1) {
    detenerMotor();
    return;
  }

  ejecutarPID(posicionActual);
}

// Finales de carrera
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
    } else if (switchArriba && !switchAbajo) {
      moverHaciaPotMenor(PWM_BAJANDO_MAX);
    } else {
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

// Botones
void leerBotones() {
  for (int i = 0; i < 5; i++) {
    bool lectura = digitalRead(botonesPiso[i]);

    if (lectura != ultimaLecturaBoton[i]) {
      ultimoTiempoBoton[i] = millis();
      ultimaLecturaBoton[i] = lectura;
    }

    if ((millis() - ultimoTiempoBoton[i]) > debounceDelay) {
      if (lectura != estadoEstableBoton[i]) {
        estadoEstableBoton[i] = lectura;

        if (estadoEstableBoton[i] == LOW) {
          seleccionarPiso(i + 1);
        }
      }
    }
  }
}

// Serial
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

// Piso objetivo
void seleccionarPiso(int piso) {
  pisoObjetivo = piso;
  enMovimiento = true;
  liberandoFinal = false;

  numeroPrueba++;
  tiempoInicioMovimiento = millis();

  resetPID();

  Serial.print("Nuevo piso objetivo: ");
  Serial.print(pisoObjetivo);
  Serial.print(" | Referencia: ");
  Serial.println(pisoRef[pisoObjetivo - 1]);
}

// PID
void ejecutarPID(int posicionActual) {
  unsigned long ahora = millis();

  if (ahora - lastPidTime < Ts_ms) {
    return;
  }

  float dt = (ahora - lastPidTime) / 1000.0;
  lastPidTime = ahora;

  int referencia = pisoRef[pisoObjetivo - 1];
  float error = referencia - posicionActual;
  int errorAbs = abs((int)error);

  ultimoError = error;

  if (errorAbs <= TOLERANCIA) {
    unsigned long tiempoTotal = millis() - tiempoInicioMovimiento;

    registrarResultadoPID(
      numeroPrueba,
      pisoObjetivo,
      referencia,
      posicionActual,
      errorAbs,
      tiempoTotal,
      "CORRECTO"
    );

    detenerMotor();
    enMovimiento = false;
    pisoObjetivo = -1;
    resetPID();

    Serial.println("Llego al piso objetivo");
    return;
  }

  integral += error * dt;
  integral = constrain(integral, -300.0, 300.0);

  float derivada = 0.0;

  if (primerCicloPID) {
    derivada = 0.0;
    primerCicloPID = false;
  } else {
    derivada = (error - errorAnterior) / dt;
  }

  errorAnterior = error;

  ultimoP = Kp * error;
  ultimoI = Ki * integral;
  ultimoD = Kd * derivada;

  float salidaPID = ultimoP + ultimoI + ultimoD;
  ultimaSalidaPID = salidaPID;

  aplicarSalidaMotor(salidaPID, errorAbs);
}

// Salida motor
void aplicarSalidaMotor(float salidaPID, int errorAbs) {
  int pwm;

  if (salidaPID > 0) {
    if (errorAbs <= ZONA_LENTA) {
      pwm = PWM_CERCA;
    } else {
      pwm = abs((int)salidaPID);
      pwm = constrain(pwm, PWM_MIN, PWM_MAX);
    }

    moverHaciaPotMayor(pwm);
  } else {
    if (errorAbs <= ZONA_LENTA) {
      pwm = PWM_BAJANDO_CERCA;
    } else {
      pwm = abs((int)salidaPID);
      pwm = constrain(pwm, PWM_BAJANDO_MIN, PWM_BAJANDO_MAX);
    }

    moverHaciaPotMenor(pwm);
  }
}

// Registro CSV
void registrarResultadoPID(
  int prueba,
  int piso,
  int referencia,
  int posicionFinal,
  int errorFinal,
  unsigned long tiempoMs,
  const char* resultado
) {
  Serial.println();
  Serial.println("RESULTADO_PID:");
  Serial.println("prueba,piso_objetivo,referencia,posicion_final,error_final,tiempo_ms,Kp,Ki,Kd,Ts_ms,resultado");

  Serial.print(prueba);
  Serial.print(",");
  Serial.print(piso);
  Serial.print(",");
  Serial.print(referencia);
  Serial.print(",");
  Serial.print(posicionFinal);
  Serial.print(",");
  Serial.print(errorFinal);
  Serial.print(",");
  Serial.print(tiempoMs);
  Serial.print(",");
  Serial.print(Kp, 3);
  Serial.print(",");
  Serial.print(Ki, 3);
  Serial.print(",");
  Serial.print(Kd, 3);
  Serial.print(",");
  Serial.print(Ts_ms);
  Serial.print(",");
  Serial.println(resultado);
  Serial.println();
}

// Potenciometro
int leerPotPromediado() {
  long suma = 0;

  for (int i = 0; i < 20; i++) {
    suma += analogRead(potPin);
    delay(2);
  }

  return suma / 20;
}

// Movimiento mayor
void moverHaciaPotMayor(int pwm) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, pwm);

  ultimoPWM = pwm;
  ultimaDireccion = 1;
}

// Movimiento menor
void moverHaciaPotMenor(int pwm) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, pwm);

  ultimoPWM = pwm;
  ultimaDireccion = -1;
}

// Parada
void detenerMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  ultimoPWM = 0;
  ultimaDireccion = 0;
}

// Reset PID
void resetPID() {
  integral = 0.0;
  errorAnterior = 0.0;
  primerCicloPID = true;
  lastPidTime = millis();

  ultimoError = 0.0;
  ultimoP = 0.0;
  ultimoI = 0.0;
  ultimoD = 0.0;
  ultimaSalidaPID = 0.0;
}

// Monitor serial
void imprimirEstado(int posicionActual, bool switchAbajo, bool switchArriba) {
  if (!enMovimiento && !liberandoFinal) {
    return;
  }

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

      Serial.print(" | P: ");
      Serial.print(ultimoP);

      Serial.print(" | I: ");
      Serial.print(ultimoI);

      Serial.print(" | D: ");
      Serial.print(ultimoD);

      Serial.print(" | PID: ");
      Serial.print(ultimaSalidaPID);
    }

    Serial.print(" | PWM: ");
    Serial.print(ultimoPWM);

    Serial.print(" | Dir: ");

    if (ultimaDireccion == 1) {
      Serial.print("POT MAYOR");
    } else if (ultimaDireccion == -1) {
      Serial.print("POT MENOR / BAJANDO");
    } else {
      Serial.print("STOP");
    }

    Serial.print(" | SW abajo: ");
    Serial.print(switchAbajo ? "ON" : "OFF");

    Serial.print(" | SW arriba: ");
    Serial.println(switchArriba ? "ON" : "OFF");
  }
}
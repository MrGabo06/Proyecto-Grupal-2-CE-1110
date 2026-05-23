const int ENA = 9;
const int IN1 = 7;
const int IN2 = 8;

const int switch1Pin = 11;
const int switch2Pin = 12;

const int VELOCIDAD = 180;

// Direcciones
const int DERECHA = 1;
const int IZQUIERDA = -1;

int direccionActual = DERECHA;

bool lastSwitch1 = false;
bool lastSwitch2 = false;

void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(switch1Pin, INPUT_PULLUP);
  pinMode(switch2Pin, INPUT_PULLUP);

  Serial.println("Motor con cambio de direccion por switches");
}

void loop() {
  bool switch1Pressed = digitalRead(switch1Pin) == LOW;
  bool switch2Pressed = digitalRead(switch2Pin) == LOW;

  // Si cualquier switch está presionado, detener motor
  if (switch1Pressed || switch2Pressed) {
    detenerMotor();

    if (switch1Pressed && !lastSwitch1) {
      Serial.println("Switch 1 presionado: motor detenido");
    }

    if (switch2Pressed && !lastSwitch2) {
      Serial.println("Switch 2 presionado: motor detenido");
    }
  } 
  else {
    // Si el switch 1 se acaba de soltar, cambia a izquierda
    if (lastSwitch1 == true && switch1Pressed == false) {
      direccionActual = IZQUIERDA;
      Serial.println("Switch 1 liberado: girando izquierda");
      delay(100);
    }

    // Si el switch 2 se acaba de soltar, cambia a derecha
    if (lastSwitch2 == true && switch2Pressed == false) {
      direccionActual = DERECHA;
      Serial.println("Switch 2 liberado: girando derecha");
      delay(100);
    }

    girarMotor(direccionActual);
  }

  lastSwitch1 = switch1Pressed;
  lastSwitch2 = switch2Pressed;
}

void girarMotor(int direccion) {
  if (direccion == DERECHA) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } 
  else if (direccion == IZQUIERDA) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }

  analogWrite(ENA, VELOCIDAD);
}

void detenerMotor() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
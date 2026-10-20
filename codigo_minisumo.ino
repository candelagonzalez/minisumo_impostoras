// DEFINICIÓN DE PINES

// Driver de motores TB6612FNG
const int PWMA = 3;  // Velocidad Motor Izquierdo
const int AIN1 = 4;  // Dirección 1 Motor Izq
const int AIN2 = 5;  // Dirección 2 Motor Izq
const int PWMB = 6;  // Velocidad Motor Derecho
const int BIN1 = 7;  // Dirección 1 Motor Der
const int BIN2 = 8;  // Dirección 2 Motor Der
const int STBY = 9;  // Pin Standby del driver

// Sensor de Distancia (Sharp GP2Y0A21YK0F)
const int SENSOR_OPONENTE = A0; 
const int UMBRAL_ATAQUE = 350; // A mayor valor, más cerca está el oponente (rango 0-1023)

// Sensores de Línea (QRE1113) 
const int LINEA_IZQ = A1;
const int LINEA_DER = A2;
const int UMBRAL_BLANCO = 200; // Valor que indica que ha tocado el borde blanco del dohyo

// Interruptor Basculante
const int INTERRUPTOR_INICIO = 2;

// MÁQUINA DE ESTADOS
enum Estado { ESPERANDO, CUENTA_ATRAS, BUSCANDO, ATACANDO, EVADIENDO };
Estado estadoActual = ESPERANDO;

void setup() {
  Serial.begin(9600);
  
  // Configuración de pines de motores
  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH); // Activar driver
  
  // Configuración de sensores e interruptor
  pinMode(INTERRUPTOR_INICIO, INPUT_PULLUP); 
  pinMode(SENSOR_OPONENTE, INPUT);
  pinMode(LINEA_IZQ, INPUT);
  pinMode(LINEA_DER, INPUT);
}

void loop() {
  // Leemos el estado del interruptor (LOW significa encendido por el INPUT_PULLUP)
  bool robotEncendido = (digitalRead(INTERRUPTOR_INICIO) == LOW);

  if (!robotEncendido) {
    estadoActual = ESPERANDO;
  } else if (estadoActual == ESPERANDO && robotEncendido) {
    estadoActual = CUENTA_ATRAS;
  }

  // LÓGICA DE ESTADOS
  switch (estadoActual) {
    
    case ESPERANDO:
      detenerMotores();
      break;

    case CUENTA_ATRAS:
      // Regla obligatoria de 5 segundos de espera en Minisumo
      delay(5000); 
      estadoActual = BUSCANDO;
      break;

    case BUSCANDO:
    case ATACANDO:
    case EVADIENDO:
      // 1. Prioridad absoluta: No salirse del dohyo
      int valorIzq = analogRead(LINEA_IZQ);
      int valorDer = analogRead(LINEA_DER);
      
      if (valorIzq < UMBRAL_BLANCO || valorDer < UMBRAL_BLANCO) {
        estadoActual = EVADIENDO;
      } 
      // 2. Si no hay peligro, buscar oponente
      else {
        int distanciaOponente = analogRead(SENSOR_OPONENTE);
        if (distanciaOponente > UMBRAL_ATAQUE) {
          estadoActual = ATACANDO;
        } else {
          estadoActual = BUSCANDO;
        }
      }
      ejecutarAccion(estadoActual);
      break;
  }
}

// FUNCIONES DE ACCIÓN
void ejecutarAccion(Estado estado) {
  if (estado == EVADIENDO) {
    // Retroceder rápido y girar para no caer
    moverMotores(-150, -150);
    delay(300);
    moverMotores(150, -150); // Girar sobre sí mismo
    delay(400);
  } 
  else if (estado == ATACANDO) {
    // Ir a máxima velocidad hacia adelante para empujar con la cuña
    moverMotores(255, 255);
  } 
  else if (estado == BUSCANDO) {
    // Girar lentamente buscando al oponente
    moverMotores(100, -100);
  }
}

// CONTROLADOR DE MOTORES TB6612FNG
void moverMotores(int velIzq, int velDer) {
  // Motor Izquierdo (A)
  if (velIzq >= 0) {
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    velIzq = -velIzq;
  }
  analogWrite(PWMA, velIzq);

  // Motor Derecho (B)
  if (velDer >= 0) {
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
  } else {
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    velDer = -velDer;
  }
  analogWrite(PWMB, velDer);
}

void detenerMotores() {
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW); analogWrite(PWMA, 0);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW); analogWrite(PWMB, 0);
}
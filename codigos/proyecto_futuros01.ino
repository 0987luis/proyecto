// Incluir Bibliotecas Necesarias
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
// Parte de las Bibliotecas
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Servo servoMotor;

// Pines de los sensores ultrasónicos
const int TrigerFrente = 2;
const int EchoFrente = 3;
const int TrigerIzquierda = 4;
const int EchoIzquierda = 5;
const int TrigerDerecha = 6;
const int EchoDerecha = 7;
const int TrigerAtras = 8;
const int EchoAtras = 9;

// Pines del driver del motor
const int motorPin1 = 9;
const int motorPin2 = 10;
const int motorVelocidad = 11;

// Velocidad Del Motor
const int velocidadNormal = 220; // Velocidad Para Todo el Programa 

// Variables para almacenar distancias
long distanciaFrente, distanciaIzquierda, distanciaDerecha, distanciaAtras;

// Tiempo para la actualización de sensores y pantalla
unsigned long tiempoImprimir = 0;
const unsigned long intervaloActualizacion = 50; //Tiempo pra la siquiente actualizacion 

// Distancias mínimas permitidas para la fucion acomodar
const int distanciaMinimaIzquierda = 17;   // Distancia mínima permitida a la izquierda
const int distanciaMinimaDerecha = 17;     // Distancia mínima permitida a la derecha

// Variable para controlar el tiempo que debe esperar para Poder seguir con el programa
unsigned long esperaLuegoEvasion = 0;
const unsigned long intervaloEsperarDespuesEvasion = 800; // Esperar 800 ms después de evadir obstáculos

void setup() {
  lcd.begin(16, 2);
  lcd.backlight();

  // Configuración de los pines de los sensores
  pinMode(TrigerFrente, OUTPUT);
  pinMode(EchoFrente, INPUT);
  pinMode(TrigerIzquierda, OUTPUT);
  pinMode(EchoIzquierda, INPUT);
  pinMode(TrigerDerecha, OUTPUT);
  pinMode(EchoDerecha, INPUT);
  pinMode(TrigerAtras, OUTPUT);
  pinMode(EchoAtras, INPUT);

  // Configuración de los pines del motor
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorVelocidad, OUTPUT);

  // Configuración del servo motor
  servoMotor.attach(12); // Pin del servo motor

  // Encender el motor al inicio
  MovimientoVehiculo(1); 
}

// Función para medir distancia
long MedirDistancia(int trigerPin, int echoPin) {
  digitalWrite(trigerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigerPin, LOW);

  long duracion = pulseIn(echoPin, HIGH, 20000); // Timeout de 20 ms
  if (duracion == 0) return 500; // Si no hay respuesta, asumir distancia máxima (500 cm)
  
  long distancia = duracion * 0.034 / 2;
  return distancia;
}

// Función para controlar el servo
void controlServo(int angulo) {
  servoMotor.write(angulo);
}

// Función para mover el vehículo
void MovimientoVehiculo(int direccion) {
  analogWrite(motorVelocidad, velocidadNormal); // Establecer la velocidad 
  if (direccion == 1) { // Adelante
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  } else if (direccion == -1) { // Atrás
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
  } else { // Detener
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  }
}

// Función para actualizar la pantalla LCD
void actualizarLCD() {
  lcd.setCursor(0, 0);
  lcd.print("F:");
  lcd.print(distanciaFrente);
  lcd.print("cm B:");
  lcd.print(distanciaAtras);
  lcd.print("cm    ");  

  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(distanciaIzquierda);
  lcd.print("cm R:");
  lcd.print(distanciaDerecha);
  lcd.print("cm    ");  
}

// Función para actualizar las distancias
void actualizarDistancias() {
  distanciaFrente = MedirDistancia(TrigerFrente, EchoFrente);
  distanciaIzquierda = MedirDistancia(TrigerIzquierda, EchoIzquierda);
  distanciaDerecha = MedirDistancia(TrigerDerecha, EchoDerecha);
  distanciaAtras = MedirDistancia(TrigerAtras, EchoAtras);
}

// Función para evadir 
void Evadir() {
  // Si hay un objeto(pared)al lado mas grande se dirigira 
  if (distanciaFrente < 76) {
    MovimientoVehiculo(1); // Tener el motor encendido
    unsigned long tiempoInicio = millis();
    while (millis() - tiempoInicio < 950) { // evadir por 950 minisegundos
      if (distanciaIzquierda < distanciaDerecha) { // Si izquierda es mas grande se ajusta el serva para que se mueva a la izquierda
        controlServo(117);
      } else { // SI no se dirigira a la derecha
        controlServo(50);
      }
      delay(10); // para que tenga el sistema en cargar
      actualizarDistancias(); // Actualizar distancias durante la evasión
    }

    controlServo(93); // Centro
    esperaLuegoEvasion = millis(); // Registrar el tiempo de la última evasión
  }
}

// Función para acomodar el vehículo después de la evasión
void acomodar() {
  // Verificar si ha pasado suficiente tiempo desde la última evasión
  if (millis() - esperaLuegoEvasion < intervaloEsperarDespuesEvasion) {
    return; // Si no ha pasado suficiente tiempo, salir 
  }

  // Ajustar la posición Para que este lo mas centrado posible 
  if (distanciaIzquierda < distanciaMinimaIzquierda) {
    controlServo(113); // Girar hacia la derecha
    delay(450); 
    controlServo(70); 
    delay(350); 
    controlServo(93); 
    delay(350); // Esto lo que hace es girar y regresar al centro para que el coche no pierda potencia ademas que se mantenaga en un angulo el cual avance
  } else if (distanciaDerecha < distanciaMinimaDerecha) {
    controlServo(62); 
    delay(450); 
    controlServo(108); 
    delay(350); 
    controlServo(93); // Regresar al centro
    delay(350); 
  }
}

void loop() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoImprimir >= intervaloActualizacion) {
    actualizarDistancias(); // Actualizar las distancias
    actualizarLCD(); // Actualizar la pantalla LCD
    tiempoImprimir = tiempoActual; // Actualizar el tiempo de la última actualización
  }

  Evadir(); // Evasión 
  acomodar(); // Acomodar el vehículo después de evadir obstáculos
  delay(50); // Para no sobre calentar el sistema 
}

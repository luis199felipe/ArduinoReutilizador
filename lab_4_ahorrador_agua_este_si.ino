/*
 * Laboratorio #4, ahorrador de agua
 * Integrantes: Luis Felipe Tejada - German Gonzalez - Alejandro Castaño
 * Materia: Electrónica Digital
 * Uniquindío
 */
#include <EEPROM.h> //Libreria para almacenar los datos en memoria
#include <TimerOne.h> //Libreria de MultiShield
#include <Wire.h> //Libreria de MultiShield
#include <MultiFuncShield.h> //Libreria de MultiShield

int pin_sensor = 5; //Pin del sensor de agua
short estrato; //Variable para almacenar el estrato del usuario
int numeroDescargas; //Variable que almacena el numero de descargas

/*
 * Método de configuración que se ejecuta al iniciar el programa
 */
void setup()
{
  Serial.begin(9600);//Configuración de Arduino
  Timer1.initialize();//Inicializacion de MultiShield  
  MFS.initialize(&Timer1); //ini de MultiShield
  
  //Inicializa el sensor del nivel de agua
  pinMode(pin_sensor, INPUT);
  
  //Lee la cantidad de descargas de la memoria EEPROM
  numeroDescargas = EEPROM.read(0);
  
  //Lee el estrato guardado en EEPROM
  estrato = EEPROM.read(1);
}

/*
 * Método que se ejecuta constantemente
 */
void loop()
{ 
  byte btn = MFS.getButton(); //Se inicializa el boton
  delay(2000); //Se paraliza la ejecución 2 segundos

  /*
   * Condición que detecta el nivel del agua, esta energia se detecta por el pin 5,
   * en caso de ser verdadero (HIGH) se está generando una descarga del tanque
   */
  if(digitalRead(pin_sensor) == HIGH)
  {
    descargaTanque();
  }
  
  //Condición para detectar si un botón ha sido presionado
  if (btn)
  {
    byte buttonNumber = btn & B00111111;    
    byte buttonAction = btn & B11000000;

    /*
     * En caso de presionar el botón 2, invoca al método cambioEstrato
     */
    if(buttonNumber == 2)
    {
      cambioEstrato();
    }
    //Si presiona el botón 3 resetea la configuración (memoria) del arduino
    else if(buttonNumber == 3)
    {
      MFS.write("rese");
      EEPROM.write(0, 0);
      numeroDescargas = 0;
      delay(4000);
      MFS.write(0);
    }
  }
}

/*
 * Método para configurar el estrato del usuario
 */
void cambioEstrato()
{
  int i = 0;

  /*
   * Se itera (10 veces * 2000 ms), que equivalen a 20 segundos,
   * mientras el usuario hace el cambio del estrato
   */
  while(i < 10)
  {
    MFS.write(analogRead(POT_PIN)/100); //Se escribe en pantalla el valor del estrato configurado
    delay(2000); //Se pausa el proceso por 2 segundos
    i = i+1; //Se incrementa en 1 la variable
  }
  
  MFS.write("ESTR");
  delay(1000);//Se paraliza 1 segundo la ejecución del programa
  MFS.write(analogRead(POT_PIN)/100);//Se escribe en pantalla el estrato configurado
  estrato = (analogRead(POT_PIN)/100);//Se guarda el estrato configurado
}

/*
 * Método que se ejecuta cuando se realiza una descarga del tanque.
 * 1000 litros es 1 metro cubico  y cada metro cubico cuesta 1300 pesos.
 * El precio por metro cubico con acueducto, alcantarillado, aseo es de 2573 pesos
 */
void descargaTanque()
{
  numeroDescargas = numeroDescargas + 1; //Se incrementa el numero de descargas
  EEPROM.write(0, numeroDescargas); //Se guarda el numero de descargas en la memoria
  long din = calcularPrecioEstrato(); //Se calcula el precio dependiendo del estrato estrato.
  int decimales = 0; //Variable para determinar el número de decimales del precio

  /*
   * Condición para determinar el numero de digitos del precio, se divide por 1000 y 
   * dependendo de la cantidad se muestran 2 decimales hasta 99999
   */
  if(din >= 10000 && din <= 99999)
  {
    din = din/1000;
    decimales = 2;
  }
  //En caso del valor del dinero sea mayor a 9999 y menor a 999999 se muestra solo un decimal
  else if(din > 99999 && din <= 999999)
  {
    din = din/1000;
    decimales = 1;
  }
  else
  {
    //Borrado de la memoria del EEPROM
    MFS.write("rese");
    EEPROM.write(0, 0);
    numeroDescargas = 0; //Se resetea el numero de descargas
    delay(4000); //Se pausa el proceso por 4 segundos
    MFS.write(0); //Se imprime en el display el numero 0
  }
  
  //Escribe los valores en pantalla, descarga, numero de descargas y por ultimo el costo
  MFS.write("DESC");
  delay(1500);
  MFS.write(numeroDescargas);
  delay(1500);
  MFS.write("COSTO");
  delay(1500);

  //Se imprime en el display el valor del dinero con sus decimales
  MFS.write(din, decimales);
  delay(1500);
  
  //Espera a que se apague el sensor, (hasta que se llene el tanque del baño)
  while(digitalRead(pin_sensor) == HIGH)
  {
    MFS.write("----");
    delay(400);
  }

  //Se imprime en pantlla el valor del dinero con los decimales respectivos
  MFS.write(din, decimales);
  delay(1000);
}

/*
 * Método para calcular el precio del agua por metro cuadrado según el estrato
 */
long calcularPrecioEstrato()
{
  long dinero = 21*numeroDescargas; //Calcula cada vez el númeto de descargas por el valor del agua en m^3

  //Depende del estrato de la persona se calcula el precio del agua hasta el momento
  if(estrato == 1)
  {
    dinero = dinero - dinero*(0.585);
  }
  else if(estrato == 2)//Si es estrato 2
  {
    dinero = dinero - dinero*(0.31);
  }
  else if(estrato == 3)//Si es estrato 3
  {
    dinero = dinero - dinero*(0.115);
  }
  else if(estrato == 4)//Si es estrato 4
  {
    dinero = dinero + dinero*(0.60);
  }
  else if(estrato >= 5)//Si es estrato 5 o mayor
  {
    dinero = dinero + dinero*(0.70);
  }

  //Se retorna el valor calculado del dinero
  return dinero;
}

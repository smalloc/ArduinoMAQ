/*
  SD card read/write
 
 This example shows how to read and write data to and from an SD card file 	
 The circuit:
 * SD card attached to SPI bus as follows:
 ** UNO:  MOSI - pin 11, MISO - pin 12, CLK - pin 13, CS - pin 4 (CS pin can be changed)
  and pin #10 (SS) must be an output
 ** Mega:  MOSI - pin 51, MISO - pin 50, CLK - pin 52, CS - pin 4 (CS pin can be changed)
  and pin #52 (SS) must be an output
 ** Leonardo: Connect to hardware SPI via the ICSP header

 
 created   Nov 2010  by David A. Mellis
 modified 9 Apr 2012  by Tom Igoe
 This example code is in the public domain.
 	 
 */
#include <EEPROM.h> 
#include <SPI.h>
#include <SD.h>
#include <Keypad.h>
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <Wire.h>  // Incluye la librería Wire
#include "RTClib.h" // Incluye la librería RTClib

#define SALTO_DE_LINEA    10
#define RETORNO_DE_CARRO  13
#define NUMERAL           35
#define COMA              44

#define LEGAJO            "INGRESE LEGAJO"
#define CLAVE             "INGRESE CLAVE"

RTC_DS1307 RTC; // Crea el objeto RTC
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {46, 47, 48, 49}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {42, 43, 44, 45}; //connect to the column pinouts of the keypad
String identificador="";//variaable de ingreso x teclado
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
//LiquidCrystal lcd(41,40,39,38,37,36);
byte archivo =0;
int conexmot = 6;
int pulsomot = 7;
int rec_arch;
int sel_arch; // variable para selección de archivo para la SD
long posNV; //es donde arranca la cantidad en la tabla NV
long pos_nombre; //es donde arranca el nombre en la tabla USU_DEF
long posicion;//ultima coma encontrada en archivo SD en la coincidencia
long posclave;//posición clave usuario
long posart;//posición articulo seleccionado
long posdisp;//posición cantidad disponible resorte
long posperm;//posición saldo disponible usuario
int coma;//lectura caracter x caracter de la SD
int permiso=0;
File myFile;//nombre archivo abierto
String Legajo ="";//campo legajo en SD
String Art_id = "";//al encontarr coincidencia extrae todo el código de artículo
boolean coordine;
boolean operador;
String error = "";
String clave = "";
int num_campo = 0;  // indica el número de campo que se compara en la funcuión "compara"
String comparador ="";
String resorte = "";
long tiempo = 0;
long tiem_inicio = 0;
String fila = "";
String columna = "";
int elimin_coma = 0 ;
int digito=0; // caracter para extraer funcion extraer2/calculador
int dig_ext2=0; // no se si lo voy a usar es en el caso que de una misma tabla tenga que extraer 2 campos distintos!!!
int m=0; // contador de digito decena y unidad para extraer2 y para calculador
int devol_num; //  devuelve calculador
String sald_perm = "" ;
String sald_maq = "" ;
String Maq_Id="primermaq";
int acum_NV;//saldo x art_Id para carga de NV
int exist_maq; // saldo resorte para cuenta
int perm_usua; // saldo permitido para usuario
const int chipSelect = 4;
String varclav1;
int buzzer = 17;
int sel_columna_A = A2;
int sel_columna_B = A3;
int sel_columna_C = A4;
int sel_columna_D = A5;
int sel_estante_1 = 8;
int sel_estante_2 = 9;
int sel_estante_3 = 10;
int sel_estante_4 = 11;
int sel_estante_5 = 12;
int sel_estante_6 = 13;
long tiempomotor = 6100;
int cont_arch=1;
int cont_provs=3;
int minuto;
int archi_cop;
int arch30_cop;
int sin_movs;
int va_NV = 0; // variable que indica que hay que mandar la NV

String msg = "";




void setup(){    
  Serial.begin(9600); // Establece la velocidad de datos del puerto serie
  Serial1.begin(9600); // puerto para comunicacion con MAQ_GPRS

  Wire.begin(); // Establece la velocidad de datos del bus I2C
  RTC.begin(); // Establece la velocidad de datos del RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));
 // lcd.begin(16, 2);
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  
//                       EEPROM.write (0,1);
  
//  String Maq_Id="primermaq"; // ojo debemos generar una customizacion de la funcion buscar para buscar el maq_Id y asignarlo aquí
  pinMode(SS, OUTPUT);
  pinMode(conexmot, OUTPUT);
  pinMode(pulsomot, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  if (!SD.begin(chipSelect)){
    Serial.println("initialization failed!");
    return;
   }
   
  pinMode(22, INPUT); // pin que MAQ_GPRS avisa si está listo
  pinMode(sel_columna_A, OUTPUT);
  pinMode(sel_columna_B, OUTPUT);  
  pinMode(sel_columna_C, OUTPUT);
  pinMode(sel_columna_D, OUTPUT);
  digitalWrite(sel_columna_A , LOW);
  digitalWrite(sel_columna_B , LOW);
  digitalWrite(sel_columna_C , LOW);
  digitalWrite(sel_columna_D , LOW);
  pinMode(sel_estante_1, OUTPUT);
  pinMode(sel_estante_2, OUTPUT);
  pinMode(sel_estante_3, OUTPUT);
  pinMode(sel_estante_4, OUTPUT);
  pinMode(sel_estante_5, OUTPUT);
  pinMode(sel_estante_6, OUTPUT);
  digitalWrite(sel_estante_1 , LOW);
  digitalWrite(sel_estante_2 , LOW);
  digitalWrite(sel_estante_3 , LOW);
  digitalWrite(sel_estante_4 , LOW);
  digitalWrite(sel_estante_5 , LOW);
  digitalWrite(sel_estante_6 , LOW);
  archi_cop=1;
  arch30_cop=1;
  Serial.flush ();
  Serial2.flush ();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------


void loop(){
volver:
digitalWrite(pulsomot,LOW);
digitalWrite(conexmot,LOW);
operador=false;
exist_maq=0;
perm_usua=0;
sel_arch=1; //asigno 1 arranca con tabla USUA_DEF.txt
Art_id = "";//vacio variable
identificador="";//vacio variable
clave="";

ingdatos();                 // ingreso datos LEGAJO y CLAVE
lcd.setCursor(0, 1);
buscar();                 // busco LEGAJO en tabla
posclave=posicion+1;

  if (error != "error"){
    num_campo = 1;
    comparador = clave;    
    comparar();                 // comparo CLAVE en tabla
  }else{
    error = "";
    goto volver;    // error en CLAVE o LEGAJO -> empieza el void loop de nuevo
  }
  if (error!="error")
  {
    Legajo = identificador;
    num_campo = 4;
    comparador = "O";    
    comparar (); //vamos a revisar si es operador o usuario
    if (operador==true)
    {
    repositor ();
    goto volver;
    }
    resort ();  // ingreso de seleeccion de resorte-producto
  }
  else
  {
    error = "";
    goto volver;
  }
if (resorte=="1234")
   {
    cambioclave ();
    error = "";
    goto volver;    // error o termina cambio de clave
    }   
///// pasamos al PEDIDO  
  sel_arch=2 ; //asigno 2 tabla MAQ_DEF.txt
  identificador = resorte ;
buscar() ; // busco RESORTE en tabla MAQ_DEF
if (error == "error")
{
    error = "";
    goto volver;    // RESORTE VACIO -> empieza el void loop de nuevo
}  
posart=posicion+1;
extraer(); // a partir de acá se extrae el código de producto de la tabla MAQ_DEF
if (error == "error")
{
    error = "";
    goto volver;    // RESORTE VACIO -> empieza el void loop de nuevo
}  
// desde aqui se analiza el permiso del producto elegido
sel_arch=3;
identificador = Legajo + Art_id ;
buscar();
if (error == "error")
{
    error = "";
    goto volver;    // SIN PERMISO -> empieza el void loop de nuevo
}  
posperm=posicion+1;
extraer2() ;// desde aqui busco saldo permiso de usuario
sel_arch=2 ;
extraer2() ;// desde aqui busco saldo resorte
if (exist_maq<1)
{
    lcd.clear();
    lcd.setCursor(0, 0);             
    lcd.print("NO EXISTENCIA");
    error="error";
    delay(2000);
}
if (perm_usua<1)
{
    lcd.clear();
    lcd.setCursor(0, 0);             
    lcd.print("NO AUTORIZADO");
    error="error";
    delay(2000);
}
if (error=="error")
{
  error="";
  goto volver;
}
  
    lcd.clear();
    lcd.setCursor(0, 0);             
    lcd.print("ENTREGANDO");
    Serial.println("ENTREGANDO");
    delay(2000);
manejomotor_arduino();
Serial.println("++++entrego++++");
NV();
error="";
movimiento();//actualizar resorte, permitido y grabar operacion / operacion provisoria

}

//actualizacion de permisos de ususarios
//mecanismo similar a la actualización de máquina

//--------------------------------------------------------------------

void mns_trabaja()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESPERE");
    lcd.setCursor(0, 1);
    lcd.print("OCUPADO");
delay (2000);
return; 
}

//--------------------------------------------------------------------


void manejomotor_arduino(){
  int fila_sel=fila.toInt();
  int columna_sel=columna.toInt();
  
  switch (columna_sel){
             case 1:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , LOW);
               digitalWrite(sel_columna_C , LOW);
               digitalWrite(sel_columna_D , LOW);
               break;
               
             case 2:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , LOW);
               digitalWrite(sel_columna_C , LOW);
               digitalWrite(sel_columna_D , HIGH);
               break;
               
             case 3:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , LOW);
               digitalWrite(sel_columna_C , HIGH);
               digitalWrite(sel_columna_D , LOW);
               break;
             case 4:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , LOW);
               digitalWrite(sel_columna_C , HIGH);
               digitalWrite(sel_columna_D , HIGH);
               break;
             case 5:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , HIGH);
               digitalWrite(sel_columna_C , LOW);
               digitalWrite(sel_columna_D , LOW);
               break;
             case 6:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , HIGH);
               digitalWrite(sel_columna_C , LOW);
               digitalWrite(sel_columna_D , HIGH);
               break;
             case 7:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , HIGH);
               digitalWrite(sel_columna_C , HIGH);
               digitalWrite(sel_columna_D , LOW);
               break;
             case 8:
               digitalWrite(sel_columna_A , LOW);
               digitalWrite(sel_columna_B , HIGH);
               digitalWrite(sel_columna_C , HIGH);
               digitalWrite(sel_columna_D , HIGH);
               break;
             case 9:
               digitalWrite(sel_columna_A , HIGH);
               digitalWrite(sel_columna_B , LOW);
               digitalWrite(sel_columna_C , LOW);
               digitalWrite(sel_columna_D , LOW);
               break;
             case 10:
               digitalWrite(sel_columna_A , HIGH);
               digitalWrite(sel_columna_B , LOW);
               digitalWrite(sel_columna_C , LOW);
               digitalWrite(sel_columna_D , HIGH);
               break;
          }
        switch (fila_sel)
          {
            case 1:
              Serial.println("caso 1");
              digitalWrite(sel_estante_1 , HIGH);
              delay(tiempomotor);
              break;
            case 2:
              Serial.println("caso 2");
              digitalWrite(sel_estante_2 , HIGH);
              delay(tiempomotor);
              break;
            case 3:
              Serial.println("caso 3");
              digitalWrite(sel_estante_3 , HIGH);
              delay(tiempomotor);
              break;
            case 4:
              Serial.println("caso 4");
              digitalWrite(sel_estante_4 , HIGH);
              delay(tiempomotor);
              break;
            case 5:
              Serial.println("caso 5");
              digitalWrite(sel_estante_5 , HIGH);
              delay(tiempomotor);
              break;
            case 6:
              Serial.println("caso 6");
              digitalWrite(sel_estante_6 , HIGH);
              delay(tiempomotor);
              break;                     
         }
      digitalWrite(sel_estante_1 , LOW);
      digitalWrite(sel_estante_2 , LOW);
      digitalWrite(sel_estante_3 , LOW);
      digitalWrite(sel_estante_4 , LOW);
      digitalWrite(sel_estante_5 , LOW);
      digitalWrite(sel_estante_6 , LOW);
      digitalWrite(sel_columna_A , LOW);
      digitalWrite(sel_columna_B , LOW);
      digitalWrite(sel_columna_C , LOW);
      digitalWrite(sel_columna_D , LOW);
      
      return;
} 
  


void desbloke ()
{
elimin_coma = 0;  
long p=0;
Legajo = "";
clave = "";
error="";
      //pregunto si existe el archivo "default/desbloke.txt"
  if (SD.exists("default/desbloke.txt"))
    {
      Serial.println("si existe desbloke!");
      while (error!="error")
      {
      elimin_coma = 0;
      myFile = SD.open("default/desbloke.txt");
      myFile.seek (p);
      coma=0;
      while (coma!=RETORNO_DE_CARRO && coma!=SALTO_DE_LINEA)
      {
        coma=myFile.read();
        p=myFile.position(); // posicion copia plantilla original
        if (!myFile.available())
          {
            error="error";
            break;
          }
        else
          {
            
            if (coma!=RETORNO_DE_CARRO && coma!= SALTO_DE_LINEA)
            {
              if (coma == COMA)
                {
                 elimin_coma=elimin_coma+1;
                 coma = myFile.read();  
                }
				if (elimin_coma == 0)
                                        Legajo = Legajo + char(coma);
				if (elimin_coma == 1)
					clave = clave + char(coma);
              }
          }
      }
  myFile.close();        
  sel_arch=1;
  identificador = Legajo;
  buscar();
  if(error!="error")
  {
    posclave=posicion+1;
    myFile = SD.open("usua_def.txt",FILE_WRITE);
    myFile.seek (posclave);
    myFile.print(clave);
    myFile.close();
  }
  identificador = "";
  Legajo = "";
  clave = "";
    }
  SD.remove("default/desbloke.txt");
    }
return;
}
  
//--------------ñññ------------------------------------------------------  


void permadic() //esta funcion incorpora consumos adicionales, 
{
elimin_coma = 0;
String campo="";      
long p=0;
String ident="";
String cant_adic="";
Legajo = "";
String ccodigo = "";
      
      //pregunto si existe el archivo "default/PERMADIC.txt"
  if (SD.exists("default/perm_tem.txt"))
    {
      while (error!="error")
      {
      myFile = SD.open("default/perm_tem.txt");
      myFile.seek (p);
      coma=0;
      while (coma!=SALTO_DE_LINEA)
      {
//        Serial.println("entre al rulo de lectura");
        coma=myFile.read();
        p=myFile.position(); // posicion copia plantilla original
        if (!myFile.available())
          {
            error="error";
            break;
          }
        else
          {
            if (coma!=RETORNO_DE_CARRO and coma!= SALTO_DE_LINEA)
            {
              if (coma == COMA)
                {
                 elimin_coma=elimin_coma+1;
                 coma = myFile.read();  
                }
				if (elimin_coma == 0)
					Legajo = Legajo + char(coma);
				if (elimin_coma == 1)
					ccodigo = ccodigo + char(coma);
                if (elimin_coma<2)
                { 
                  ident=ident+char(coma);
                }
                else
                {
                campo=campo+char(coma);
                }
              }
           }
//           Serial.println(campo);
      }

  myFile.close();        
//  Serial.println("me dispongo a grabar1");
  sel_arch=3;
  identificador = ident;
  cant_adic = campo;        
  buscar();
  posperm=posicion+1;
  myFile = SD.open("PERMISOS.txt",FILE_WRITE);
  Serial.print("posicionamiento en permiso....");
  Serial.println(posperm);
  myFile.seek (posperm);
  myFile.print(cant_adic);
  myFile.close();
  identificador = "";
  elimin_coma = 0;
  ident = "";
  campo = "";
  cant_adic = "";
  Legajo = "";
/*  //aca grabo movimien y mov_gprs
  	DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
	int R_anio = (now.year()-2000);
	int R_mes = now.month();
	int R_dia = now.day();
	myFile = SD.open("mov_gprs.txt", FILE_WRITE);
	myFile.print("act_permisos ," + llegajo + "," + ccodigo);
	myFile.print(R_dia);
	myFile.print(',');
	myFile.print(R_mes);
	myFile.print(',');
	myFile.println(R_anio);
        myFile.close();	
	myFile = SD.open("movimien.txt", FILE_WRITE);
	myFile.print("act_permisos ,"+llegajo+","+ccodigo+",");
	myFile.print(R_dia);
	myFile.print(',');
	myFile.print(R_mes);
	myFile.print(',');
	myFile.println(R_anio);
	myFile.close();	*/
    }
  SD.remove("default/perm_tem.txt");
    }
return;
}

//-------------------------ENVIA NOTA DE VENTA  -----------------------

void enviaNV ()
{
coordine=false;
boolean copie=false;
  int z = 0;
  int n=0;
  int a = 0;
  if (SD.exists("NV.txt"));
  {
  unsigned long startTime = millis();
  long espera = millis();
  Serial1.println("n");  
  int bolu=1;
  String fin="no";
  while (!Serial1.find("nv") && (millis()-startTime)<3000)
    {
      if((millis()-startTime)>1000)
        bolu=2;
    }
   if(bolu==1) {
      myFile = SD.open("NV.txt");
      while(fin=="no")
      {
//      n=myFile.read();
      if(myFile.available())
        {
          //Serial.write(n);
          Serial1.write(myFile.read());
          z = z+1;
 //   Serial2.write(myFile.read());
  delay(1);
        }
       else
       {
        long espera = millis();
        while(!myFile.available() and (millis()-espera)<500)
        {
          if((millis()-espera)>450)
          fin="si";
        } 
       }
      }
      Serial.println(z);
      myFile.close();
      Serial1.println(z);
      a=Serial1.parseInt();
      Serial.println(a);
      if (a==(z+2))
      {
        SD.remove("NV.txt");
        va_NV = 0;
      }
      //fallo el envio vuelvo a intentar
      else{
        enviaNV();
      }
    }
    else
    {
    }
  }
  return;
}



//--------------------------------------------------------------------
void enviador()
{
coordine=false;
boolean copie=false;
  int z = 0;
  int n=0;
  int a = 0;
  if (SD.exists("movimien.txt"));
  {
  unsigned long startTime = millis();
  long espera = millis();
  Serial1.println("m");  
  int bolu=1;
  String fin="no";
  while (!Serial1.find("mov") and (millis()-startTime)<3000)
    {
      if((millis()-startTime)>1000)
        bolu=2;
    }
   if(bolu==1) {
      myFile = SD.open("movimien.txt");
      while(fin=="no")
      {
//      n=myFile.read();
      if(myFile.available())
        {
          //Serial.write(n);
          Serial1.write(myFile.read());
          z = z+1;
 //   Serial2.write(myFile.read());
  delay(1);
        }
       else
       {
        long espera = millis();
        while(!myFile.available() and (millis()-espera)<500)
        {
          if((millis()-espera)>450)
          fin="si";
        } 
       }
      }
      Serial.println(z);
      myFile.close();
      Serial1.println(z);
      a=Serial1.parseInt();
      Serial.println(a);
      if (a==(z+2))
      {
        SD.remove("movimien.txt");
      }
    }
    else
    {
    }
  }
  return;
}

void receptor()
{
  int permiso=0;
  while (permiso!=1)
    {
      for (int i=0;i<4000;i++)
        {
          delay(100);        
          permiso = digitalRead(22);
          if (permiso==1)
            i=4000;
        }
    }
  int z = 0;
  int n= 0;
  int a= 0;
  int bolu =1;
  long espera = millis();
  String fin="no";  
  unsigned long startTime = millis();
  if (operador==true)
  {
  Serial1.println("a");
  rec_arch=0;
  }
  else
    {
    switch (rec_arch)
    {
    case 1:
      Serial1.println("b");
      break;
    case 2:
      Serial1.println("c");
      break;
    case 3:
      Serial1.println("d");
      break;
    case 4:
      Serial1.println("e");
      break;
    case 5:
      Serial1.println("f");
      break;
    case 6:
      Serial1.println("g");
      break;
    }
    }
  while (!Serial1.find("ardu3") && (millis()-startTime)<3000)
    {
      if((millis()-startTime)>1000)
      bolu=2;
    }
  Serial1.flush(); //probando  
  if(bolu==1)
  {
   switch (rec_arch)
    {
    case 0:
      myFile = SD.open("default/maq_def.txt", FILE_WRITE);
      lcd.clear();
      lcd.setCursor(0, 0);             
      lcd.print("TRAB maq_def");
      break;
    case 1:
      myFile = SD.open("default/usua_def.txt", FILE_WRITE);
      lcd.clear();
      lcd.setCursor(0, 0);             
      lcd.print("TRAB def_usu");
      break;
    case 2:
      myFile = SD.open("default/permisos.txt", FILE_WRITE);
      lcd.clear();
      lcd.setCursor(0, 0);             
      lcd.print("TRAB permiso");
      break;
    case 3:
      myFile = SD.open("default/perm_tem.txt", FILE_WRITE);
      lcd.clear();
      lcd.setCursor(0, 0);             
      lcd.print("TRAB per_tem");
      break;
    case 4:
      myFile = SD.open("default/desbloke.txt", FILE_WRITE);
      lcd.clear();
      lcd.setCursor(0, 0);             
      lcd.print("TRAB des_blo");
      break;
    case 5:
      myFile = SD.open("usua_def.txt", FILE_WRITE); 
      lcd.clear();
      lcd.setCursor(0, 0);             
      lcd.print("TRAB usu_new");
      break;
    case 6:
      myFile = SD.open("permisos.txt", FILE_WRITE); 
      lcd.clear();
      lcd.setCursor(0, 0);             
      lcd.print("TRAB perm_new");
      break;

    }
    z=0;
    Serial1.flush();
    while(fin=="no")  
      {
      if (Serial1.available())
        {
          int w=Serial1.read();
          if (z>1)
          {
          myFile.write(w);       
          Serial.write(w);
          }
          z = z+1;
        }
      else
        {
          long espera = millis();
          while(!Serial1.available() and (millis()-espera)<500)
            {
              if((millis()-espera)>450)
                {
                  fin="si";
                }
            }               
        }  
      }
  myFile.close();
  Serial1.println(z);
  a=Serial1.parseInt();
  Serial.println(z);
  Serial.println(a);
      if (z!=(a+2))
        {
           switch (rec_arch)
             {
                  case 0:
                  SD.remove("default/maq_def.txt");
                  break;
                case 1:
                  SD.remove("default/usua_def.txt");
                  Serial.println("caso b abre archivo");
                  break;
                case 2:
                  SD.remove("default/permisos.txt");
                  break;
                case 3:
                  SD.remove("default/perm_tem.txt");
                  break;
                case 4:
                  SD.remove("default/desbloke.txt");
                  break;
                default:
                  int JJ= 0;
            } 
        }
    z=0;
  }
  else
    {
      Serial.println("no cordine recibir logo ardu3"); 
    }  
  return;
}


//-------------------------------------------------------------------------------------------------





void copiarch()
{
String campo="";
long p=0;
while (error!="error")
  {
   switch (archivo)
     {
      case 1:
      myFile = SD.open("default/PERMISOS.txt");  
      myFile.seek (p);
      break;  
      case 2:
      myFile = SD.open("default/PERMADIC.txt");  
      myFile.seek (p);
      break;            
      case 3:
      myFile = SD.open("default/maq_def.txt");  
      myFile.seek (p);
      break;            
     }        
   while (coma!=SALTO_DE_LINEA)
     {
      coma=myFile.read();
      p=myFile.position(); // posicion copia plantilla original
      if (!myFile.available())
        {
           error="error";
           break;
        }
      else
        {
           if (coma!=13 and coma!= 10)
             campo=campo+char(coma);
        }
     }
   myFile.close();  
   switch (archivo)
     {
       case 1:
         myFile = SD.open("PERMISOS.txt",FILE_WRITE);
         break;  
       case 2:
         myFile = SD.open("PERMISOS.txt",FILE_WRITE);
         break;  
       case 3:
         myFile = SD.open("maq_def.txt",FILE_WRITE);
         break;  
     }        
   myFile.println(campo);
   myFile.close();
   coma=0;
   campo="";    
  }
return;
}



//------------------------------------------------------------------------------------------------------



void recarga_perm () //esta función recarga los permisos de consumo de los ususarios al cambiar el mes
{
 int EE_mes;
 int EE_anio;
 int EE_tot;
 DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
 int R_anio = (now.year()-2000);
 int R_mes = now.month();
 int R_dia = now.day();
 EE_mes=EEPROM.read (0);
 EE_anio=EEPROM.read (1);
 if ( (EE_mes + EE_anio) != (R_mes + R_anio)) 
  {
    lcd.clear();
    lcd.setCursor(0, 0);             
    lcd.print("TRABAJANDO");
    EEPROM.write (0,R_mes);
    EEPROM.write (1,R_anio);
    //    grabar permisos mensuales en la tabla de combate
    String campo="";
    //byte existencia;
    error="";
    coma=0;
    byte x=0;
    long p=1;
    int c=1;
    SD.remove("PERMISOS.txt");
    archivo=1;
    copiarch();
    lcd.clear();
  }
//aca graba en movimientos la actualización de permisos de consumo
/*Serial.println("grabo permisos 3");
myFile = SD.open("mov_gprs.txt", FILE_WRITE);
myFile.print("act_permisos");
myFile.print(R_dia);
myFile.print(',');
myFile.print(R_mes);
myFile.print(',');
myFile.println(R_anio);
myFile.close();	
myFile = SD.open("movimien.txt", FILE_WRITE);
myFile.print("act_permisos");
myFile.print(R_dia);
myFile.print(',');
myFile.print(R_mes);
myFile.print(',');
myFile.println(R_anio);
myFile.close();	*/
return;
}


//-------------------------------------------------------------------------------------------------------------------------

void beep(){
  digitalWrite(buzzer,HIGH); //emite sonido
  delay(10); //espera 10 milisegundos
  digitalWrite(buzzer, LOW); //deja de emitir
  return;
}


//----------------------------------------------------------------------------------------------------------------------


void repositor ()
{
String campo="";
byte existencia;
error="";
coma=0;
byte x=0;
long p=1;
int c=1;
int permiso;
while (x!=42)
{
lcd.clear();
lcd.setCursor(0, 0);             
lcd.print("CAMBIA CONFIG?");
lcd.setCursor(0, 1);
lcd.print("# SI");
while ( x!=35 and x!=42)//teclear respuesta // acá tenemos problemas tenemos que ingresar 2 caracteres1!!!
{
  char customKey = customKeypad.getKey();
  if (customKey)
  {
    x=int(customKey);
    beep();
  }
}              
  if (x==42)
  break;
  if (x==35)
    {
      permiso=0;
      while (permiso!=1)
      {
        for (int i=0;i<4000;i++)
        {
        delay(100);        
        permiso = digitalRead(22);
          if (permiso==1)
          i=4000;
        }
      }
      if (permiso == 1)
        {
          Serial.println("receptor");
          SD.remove("default/maq_def.txt"); // ojo borrar temporal en default
          receptor(); 
          x=42;
        }
        else
        {
    ////  no nos paso la nueva configuración        
        beep();
        lcd.clear();
        lcd.setCursor(0, 0);             
        lcd.print("NO CAMBIO CONFIG");
        lcd.setCursor(0, 1);
        lcd.print("MANTEN ANTERIOR");
        delay (5000);
        break;  
        }
    }
}
SD.remove("maq_def.txt");
while (error!="error")
{
  myFile = SD.open("default/maq_def.txt");  
  myFile.seek (p);
  while (coma!=10)
  {
    coma=myFile.read();
    p=myFile.position(); // posicion copia plantilla original
      if (!myFile.available())
      {
        error="error";
        break;
      }
      else
      {
        if (coma!=13 and coma!= 10)
          campo=campo+char(coma);
      }
   }
    myFile.close();
    myFile = SD.open("maq_def.txt",FILE_WRITE);
    myFile.println(campo);
    myFile.close();
    coma=0;
    campo="";    
}

x=0;
while (x!=42)
{
lcd.clear();
lcd.setCursor(0, 0);             
lcd.print("TUVO FALTANTE?");
lcd.setCursor(0, 1);
lcd.print("# SI");
while ( x!=35 and x!=42)//teclear respuesta // acá tenemos problemas tenemos que ingresar 2 caracteres1!!!
{
  char customKey = customKeypad.getKey();
  if (customKey)
  {
    x=int(customKey);
    beep();
  }
}              
  if (x==42)
  break;
  if (x == 35)
    {
        resort();
        sel_arch=2 ; //asigno 2 tabla MAQ_DEF.txt
        identificador = resorte;
        buscar () ; // busco RESORTE en tabla MAQ_DEF
        x=0;
        posart=posicion+1; // posición del articulo con faltante!!!! disponible ideal luego de reposicion total
        lcd.clear();
        lcd.setCursor(0, 0);             
        lcd.print("INGRESE");
        lcd.setCursor(0, 1);             
        lcd.print("EXISTENCIA");
        campo="";
        while (x!=35)                               //cantidad existencia
          {    
            if (x==42)
            campo="";
/* tiempo = millis() - tiem_inicio;
          if (tiempo>20000)
          {
            x=35;
            error="error";
          }*/
          char customKey = customKeypad.getKey();
          if (customKey)
           {
              x=int(customKey);
              beep();
                if (x != 35)//NUMERAL 35
                  {
                    campo = campo + char(customKey);
                  }
           lcd.clear();
           lcd.setCursor(0, 0);
           lcd.print("EXISTENCIA..."+campo);
           lcd.setCursor(0, 1);
           lcd.print("# CONF * RECH");
         }
       }
        existencia=campo.toInt();
        myFile = SD.open("maq_def.txt",FILE_WRITE);
        myFile.seek(posart);
        coma=0;
        while (coma!=44)
        {
          coma=myFile.read();
        }
        if (existencia < 10)
        myFile.print('0');
        myFile.print(existencia);
        myFile.close();
       
       
       
                                                    // Grabado de la reposición
                                                    int c = 0 ;
                                                    DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
                                                    int anio = now.year();
                                                    int mes = now.month();
                                                    int dia = now.day();
                                                    int hora = now.hour();
                                                    int minuto = now.minute();
                                                    
                                                    myFile = SD.open("mov_gprs.txt", FILE_WRITE);
                                                    myFile.print("REPOSITOR ,"+Legajo + "," + clave +"," + resorte +","+existencia + ",");
                                                    myFile.print(dia);
                                                    myFile.print(',');
                                                    myFile.print(mes);
                                                    myFile.print(',');
                                                    myFile.print(anio);
                                                    myFile.print(',');
                                                    myFile.print(hora);
                                                    myFile.print(',');
                                                    myFile.println(minuto);
                                                    myFile.close();
                                                    myFile.close();
                                                    myFile = SD.open("movimien.txt", FILE_WRITE);
                                                    myFile.print("REPOSITOR ,"+Legajo + "," + clave +"," + resorte +","+existencia + ",");
                                                    myFile.print(dia);
                                                    myFile.print(',');
                                                    myFile.print(mes);
                                                    myFile.print(',');
                                                    myFile.print(anio);
                                                    myFile.print(',');
                                                    myFile.print(hora);
                                                    myFile.print(',');
                                                    myFile.println(minuto);
                                                    myFile.close();
                                                    
     }
        lcd.clear();
        lcd.setCursor(0, 0);             
        lcd.print("OTRO FALTANTE ?");
        lcd.setCursor(0, 1);
        lcd.print("# SI      * NO");
        x=0;
        while ( x!=35 and x!=42)//teclear respuesta // acá tenemos problemas tenemos que ingresar 2 caracteres1!!!
        {
          char customKey = customKeypad.getKey();
          if (customKey)
          {
          x=int(customKey);
          beep();
          }
        }              
}
                                                    if ( c == 1){
                                                    DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
                                                    int anio = now.year();
                                                    int mes = now.month();
                                                    int dia = now.day();
                                                    int hora = now.hour();
                                                    int minuto = now.minute();
                                                    myFile = SD.open("mov_gprs.txt", FILE_WRITE);
                                                    myFile.print("REPOSITOR ,"+Legajo + "," + clave + ",00,00" + ",");
                                                    myFile.print(dia);
                                                    myFile.print(',');
                                                    myFile.print(mes);
                                                    myFile.print(',');
                                                    myFile.print(anio);
                                                    myFile.print(',');
                                                    myFile.print(hora);
                                                    myFile.print(',');
                                                    myFile.println(minuto);
                                                    myFile.close();
                                                    myFile.close();
                                                    myFile = SD.open("movimien.txt", FILE_WRITE);
                                                    myFile.print("REPOSITOR ,"+Legajo + "," + clave + ",00,00" + "," );
                                                    myFile.print(dia);
                                                    myFile.print(',');
                                                    myFile.print(mes);
                                                    myFile.print(',');
                                                    myFile.print(anio);
                                                    myFile.print(',');
                                                    myFile.print(hora);
                                                    myFile.print(',');
                                                    myFile.println(minuto);
                                                    myFile.close();
                                                    }
return;
}

/*
void manejomotor()
{
  digitalWrite (pulsomot,LOW);
  digitalWrite (conexmot,HIGH);
  for (int i=0;i<fila.toInt();i++)
    {
      digitalWrite (pulsomot,HIGH);
      delay (50);
      digitalWrite (pulsomot,LOW);
      delay (50);
    }  
  digitalWrite (pulsomot,HIGH);
  delay (500);
  digitalWrite (pulsomot,LOW);
  delay (50);
  for (int i=0;i<columna.toInt();i++)
    {
      digitalWrite (pulsomot,HIGH);
      delay (50);
      digitalWrite (pulsomot,LOW);
      delay (50);
    }  
  digitalWrite (pulsomot,HIGH);
  delay (500);
  digitalWrite (pulsomot,LOW);
  delay (tiempomotor);
  digitalWrite (conexmot,LOW);
  return;
}
*/

void movimiento()//actualizar resorte, permitido y grabar operacion / operacion provisoria
{
  String def_usuario = "";
  myFile = SD.open("USUA_DEF.txt");
  myFile.seek(pos_nombre);
  int s = 0;
    while ( s != 13)
  {
    s = myFile.read();
    if(s!=13)
    def_usuario = def_usuario + char(s);
  }
  myFile.close();
  String Maquina = "";
  exist_maq = exist_maq - 1 ;
  myFile = SD.open("MAQ_DEF.txt", FILE_WRITE);
  //myFile.seek(posart);
  Maquina = fila + "," + columna + ",";
  s = 0;
  myFile.seek(posdisp);
  if (exist_maq < 10)
    myFile.print('0');
  myFile.print(exist_maq);  
  myFile.seek(posart);
  s = 0;
  while ( s != 13)
  {
    s = myFile.read();
    if(s!=RETORNO_DE_CARRO)
    Maquina = Maquina + char(s);
  }
  myFile.close();
  String usuario = "";
  perm_usua = perm_usua - 1 ;
  myFile = SD.open("PERMISOS.txt", FILE_WRITE);
  myFile.seek(posperm);
  usuario = Legajo + "," + Art_id + ",";
  s = 0;
  //entra en funciones el reloj
  DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
  int anio = now.year();
  int mes = now.month();
  int dia = now.day();
  int hora = now.hour();
  int minuto = now.minute();
  myFile.seek(posperm);
  if (perm_usua < 10)
  myFile.print('0');
  myFile.print(perm_usua);
  myFile.seek(posperm);
  s = 0;
  while ( s != RETORNO_DE_CARRO && s != SALTO_DE_LINEA)
  {
    s = myFile.read();
    if(s!=RETORNO_DE_CARRO && s != SALTO_DE_LINEA)
      usuario = usuario + char(s);
  }
  myFile.close();
  String movimiento = "" ;
  movimiento = usuario + "," + def_usuario + "," + Maquina;
  
  //grabar el movimiento
  myFile = SD.open("movimien.txt", FILE_WRITE);
  myFile.print(movimiento+",");
  myFile.print(dia);
  myFile.print(',');
  myFile.print(mes);
  myFile.print(',');
  myFile.print(anio);
  myFile.print(',');
  myFile.print(hora);
  myFile.print(',');
  myFile.println(minuto);
  myFile.close();
                          myFile = SD.open("movimien.txt");
                          while (myFile.available())
                          {
                            Serial.write(myFile.read());
                          }
  myFile = SD.open("mov_gprs.txt", FILE_WRITE);
  myFile.print(movimiento+",");
  myFile.print(dia);
  myFile.print(',');
  myFile.print(mes);
  myFile.print(',');
  myFile.print(anio);
  myFile.print(',');
  myFile.print(hora);
  myFile.print(',');
  myFile.println(minuto);
  myFile.close();
/*  Serial.println("--------------------GPRS----------------------------");
  myFile = SD.open("mov_gprs.txt");
  while (myFile.available())
  {
    Serial.write(myFile.read());
  }
  myFile.close();
Serial.println("--------------------NV----------------------------");
  myFile = SD.open("NV.txt");
  while (myFile.available())
  {
    Serial.write(myFile.read());
  }
  myFile.close();*/
  int permiso;
  permiso = digitalRead(22);
  if (permiso == 1)
    enviador(); // transfiere mov_gprs a la tarjeta con gprs
  return;
}



void NV()
/*genera informacion para la preparación de la NV
queremos preparar el remito con el cual se genera la facturacion por lo tanto apunta al maquina-dom entrega/cliente
los ajustes por +/- se trataran como transfers al depo_maq segufer -- ojo como ajustar dif en maquina
la preparación de la mercadería se baja como transfer de CDVS a depo_maq segufer
diseño registra/datos:
encabezado tabla NV.txt=domicilio de entrega-num_maq
campos: Art_id,cantidad
*/

{
identificador=Art_id;
sel_arch=4;
buscar ();
posNV=posicion+1;
extraer2 ();
return;
}


void extraer2()  // extrae el saldo del usuario permitido de ese producto y saldo del resorte
{
  m=0;
  String campo = ""; 
  switch (sel_arch)
  {
    case 2:
    myFile = SD.open("MAQ_DEF.txt");
    myFile.seek(posdisp);
    break;  
    case 3:
    myFile = SD.open("PERMISOS.txt");
    myFile.seek(posperm);
    break;  
    case 4:
    myFile = SD.open("NV.txt");
    myFile.seek(posNV);
    break;
  }
  coma=0;
  while (coma !=COMA and coma != SALTO_DE_LINEA)
{
    coma = myFile.read();
    if (coma !=COMA and coma != SALTO_DE_LINEA)
    {
      campo=campo+char(coma);
      digito=coma;
      if(sel_arch != 4)
      calculador();
      if (sel_arch==3)
      {
          sald_perm = campo;///texto saldo permitido usuario
          perm_usua=perm_usua+devol_num; // saldo permitido para usuario
      }
      if (sel_arch==2)
      {
          sald_maq = campo;///texto saldo resorte
          exist_maq=exist_maq+devol_num; // saldo resorte para cuenta
      }
     if (coma==RETORNO_DE_CARRO or !myFile.available())//lee caracter de fin de línea para escribir el nùmero completo (ojo en el caso de modificar estructura cambiar a coma)
     {
      if (sel_arch==4)
      {
        acum_NV=campo.toInt();
        myFile.close();
        myFile = SD.open("NV.txt",FILE_WRITE);
        myFile.seek(posNV);
        if (acum_NV < 99)
        myFile.print('0');
        if (acum_NV < 9)
        myFile.print('0');
        myFile.println(acum_NV+1);
        coma=10;
      }
     }
    }
  } 
coma=0;
myFile.close();
return ;
}  


int calculador()
{
  switch (digito)
    {
    case 48:
    digito=0;
    break;
    case 49:
    digito=1;
    break;
    case 50:
    digito=2;
    break;
    case 51:
    digito=3;
    break;
    case 52:
    digito=4;
    break;
    case 53:
    digito=5;
    break;
    case 54:
    digito=6;
    break;
    case 55:
    digito=7;
    break;
    case 56:
    digito=8;
    break;
    case 57:
    digito=9;
    break;
    default:
    Serial.println("error en calculador");    
    }   
  if(m==0)
  {
  devol_num=digito*10;
  }
  if(m==1)
  {
  devol_num=digito;      
  }
  m=m+1; 
return (devol_num);
}


void extraer()
{
  String campo = ""; 
  myFile = SD.open("MAQ_DEF.txt");          // Se abre el archivo y se lee desde la posición hasta salto de linea
  myFile.seek(posart);
  coma=0;
  while (coma !=COMA and coma != SALTO_DE_LINEA)
  {
    coma = myFile.read();
    if (coma !=COMA and coma != SALTO_DE_LINEA)
    {
      campo=campo+char(coma);
      posdisp=myFile.position()+1;
    }
    else
    {
      if (campo == "")
      {
        lcd.setCursor(2, 0);
        lcd.clear();
        lcd.print("ERROR EN PRODUCTO");
        delay (2000);
        error="error";
      }               
    }
  }
  coma=0;
  Art_id = campo;
  myFile.close();
  return ;
}  




void comparar()
{
  String campo = ""; 
  int n_campo = 1;
  myFile = SD.open("USUA_DEF.txt");          // Se abre el archivo y se lee desde la posición hasta salto de linea
  myFile.seek(posclave);
  coma=0;
  while (coma != SALTO_DE_LINEA){
     while (coma!=COMA){
        coma = myFile.read();
        if (num_campo==1)
          pos_nombre=myFile.position();
      
        if (coma !=COMA){
          campo=campo+char(coma);
        }else{
          if (campo == comparador and n_campo == num_campo){
            operador=true;
            error = "";
            myFile.close();
          }
        else
        {
          if (campo == "U" and n_campo == num_campo)
           {
             operador=false;
             error = "";
             myFile.close();             
           }
          else
           { 
            n_campo = n_campo + 1;
            coma = 0;
            campo="";
           }
        }
      }
       if (coma==10)
       {
          error = "error";
          lcd.setCursor(0, 0);
          lcd.clear();
          lcd.print("CLAVE ERRORNEA");
          delay(2000);
          coma=44;
          myFile.close();
       }  
   }
  if (!myFile.available())
    {
      coma=10;
    }
  }
return;
}
//####### FUNCION DE MUESTRA MENSAJE DE INGRESO DE DATOS ##########
void msg_ingr_datos(String msg){
    lcd.setCursor(2, 0);
    lcd.clear();
    //lcd.print("INGRESE LEGAJO");
    lcd.print(msg);
    lcd.setCursor(0, 1);
    lcd.print("FINALIZAR CON #");
  }

//----------------------------------
void sele ()
  {
    if(sin_movs == 5 || sin_movs == 10 || sin_movs == 15 || sin_movs == 20 || sin_movs == 30 || sin_movs == 40 || sin_movs == 45 || sin_movs == 50 || sin_movs == 55 || sin_movs == 59 )
      {                 

        if(Serial1.find("zz")||va_NV== 8 )
        {
          va_NV = 8;
          Serial.print("_________   zz  _______");
          int permiso = 0;
          unsigned long startTime;
          startTime=millis();
          while(permiso != 1 && (millis()-startTime)<30000)
          {
             permiso = digitalRead(22);
             lcd.clear();
             lcd.setCursor(2, 0);
             lcd.print("ESPERANDO ZZ");
          }
          Serial.print(" PERMISO----");
          Serial.println(permiso);          
          if (permiso == 1)
            enviaNV(); // transfiere NOTA DE VENTA a la tarjeta con gprs
        }
        if (archi_cop==1)
          {
            archi_cop=2;
            rec_arch=3;
            receptor();
            permadic();
            //msg_ingr_datos();
            return;
          }
        if (archi_cop==2)
          {
            archi_cop=3;
            rec_arch=4;
            receptor();
            desbloke();
            //msg_ingr_datos();
            return;
          }
        if (archi_cop==3)
          {
            archi_cop=4;
            rec_arch=5;
            receptor();
            //msg_ingr_datos();
            return;
          }
        if (archi_cop==4)
          {
            archi_cop=1;
            rec_arch=6;
            receptor();
            //msg_ingr_datos();
            return;
          }
      }
    if(sin_movs==25 || sin_movs==35)
      {  
        if (arch30_cop==1)
          {
            arch30_cop=2;
            rec_arch=1;
            receptor();
            //msg_ingr_datos();
            return;
          }
        if (arch30_cop==2)
          {
            arch30_cop=1;
            rec_arch=2;
            receptor();
            //msg_ingr_datos();
            return;
          }
          recarga_perm ();
      }
  }
//---------------------------------
//############FUNCION DE INGRESO DE DATOS###########
void ingdatos (){
  msg_ingr_datos(LEGAJO);
  long minuts;
  int c=0;
  int espera =0;
  int repeticion=0;
  DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
  minuts = millis(); //mantengo la variable con su nombre aunque no es el minuto
  //minuto = now.minute();
  int dia = now.day();
  int hora = now.hour();
 //int segs = now
    
  while (c!=NUMERAL){                             //carga de legajo
    char customKey = customKeypad.getKey();
  
    if (customKey){    
      minuts = millis(); 
      //minuto = now.minute();
      c= int(customKey);
      beep();
      if (c != NUMERAL)
        identificador += customKey;
    }
  
    DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
    long quieto = millis();
    //int quieto = now.minute();
    sin_movs = ((quieto - minuts)/60000);
    //sin_movs=quieto-minuto;
  if (sin_movs<0)
    sin_movs=sin_movs+60;
  if (sin_movs==5 || sin_movs==10 || sin_movs==15 || sin_movs==20 || sin_movs==25 ||
      sin_movs==30 || sin_movs==35 || sin_movs==40 || sin_movs==45 || sin_movs==50 || 
      sin_movs==55 || sin_movs==59){
      
      if(espera!= sin_movs){
          Serial.print(dia);
          Serial.print("---");
          Serial.print(hora);
          Serial.print(":");
          Serial.println(minuto);
          Serial.println("espera....");
          Serial.println(sin_movs);
          espera = sin_movs;
          repeticion=0;
       }
       
      if(repeticion<4){
          sele ();
          repeticion += 1;
          delay(20);
          //Serial.print("repeticion......");
          //Serial.println(repeticion);
        }
      if(repeticion==4)
        msg_ingr_datos(LEGAJO);
    }
   // FIN DEL WHILE
  }
 c=0;

msg_ingr_datos(CLAVE);
/*
 lcd.setCursor(2, 0);
 lcd.clear();
 lcd.print("INGRESE CLAVE");
 lcd.setCursor(0, 1);
 lcd.print("FINALIZAR CON #");
*/
  while (c!=NUMERAL)                               //carga de legajo
  {
  char customKey = customKeypad.getKey();
  if (customKey)
    {
      c= int(customKey);
      beep();
      if (c != NUMERAL)
      clave=clave+customKey;
    }
  }
    Serial.println("XXXXXXXXOOO   pase en ingdatos antes de buscar ZZ   OOOOXXXXXXX");
    if(Serial1.find("zz")||va_NV== 8 )
        {
          va_NV= 8;
          Serial.println("________________ENCONTRE ZZ en ingdatos________________");
          int permiso = 0;
          unsigned long startTime;
          startTime=millis();
          while(permiso != 1 && (millis()-startTime)<30000)
          {
             permiso = digitalRead(22);
             lcd.clear();
             lcd.setCursor(2, 0);
             lcd.print("ESPERANDO ZZ");
          }
          Serial.print(" PERMISO----");
          Serial.println(permiso);
          if (permiso == 1)
            enviaNV(); // transfiere NOTA DE VENTA a la tarjeta con gprs
        }  
return;
}



void buscar ()
{
  String ident="";
  coma=0;  
  switch (sel_arch)
  {
    case 1:
    myFile = SD.open("usua_def.txt");
    break;
    case 2:
    myFile = SD.open("MAQ_DEF.txt");
    break;  
    case 3:
    myFile = SD.open("PERMISOS.txt");
    elimin_coma = 1 ;
    break;
    case 4:
    if (!SD.exists("NV.txt"))
    {
      myFile = SD.open("NV.txt",FILE_WRITE);
      myFile.println (Maq_Id);
      myFile.close();
    }
    myFile = SD.open("NV.txt");
    break;
  }

  if (myFile)
  {
    while (myFile.available())      // mientras no sea fin de aarchivo
    {
      while (ident!=identificador)          // cambiar nombre campo identificador!!!!!
      {          
        ident = "";
        while (coma != 44)       // busca la PRIMER coma del registro
         {  
           coma = myFile.read();
           if(!myFile.available())  
             break;
            // aca la condición de eliminar coma
            if (coma == 44)
            {
             if (elimin_coma == 1)
            {
              elimin_coma = 0 ;
              coma = myFile.read();
            }
            }
            if (coma != 44)
               { 
                  ident=ident+char(coma);
                  posicion=myFile.position();
                }
        }
         while (coma != 10 )     // despues de la primer coma recorre todo el registro
           {
             coma = 0 ;
             coma = myFile.read();
             if(!myFile.available()) 
               break;
           }
           
         if (!myFile.available()and(ident!=identificador))
           {
              lcd.clear();
              lcd.setCursor(0, 0);             
              switch (sel_arch)
              {
                case 1:
                lcd.print("LEGAJO ERRONEO");
                delay(2000);
                break;
                case 2:
                lcd.print("ERROR EN PRODUCTO");
                delay(2000);
                break;  
                case 3:
                lcd.print("NO AUTORIZADO");
                delay(2000);
                break;  
                case 4:
                //Serial.print("llego error case 4");
                myFile.close();
                myFile = SD.open("NV.txt",FILE_WRITE);
                myFile.print(Art_id);
                posicion=myFile.position();    
                myFile.print(",000");
                break;  
              }
              error = "error";
              ident = identificador;
           }
 //aca agregar la carga de elimin_coma
            if (sel_arch == 3) 
            elimin_coma = 1 ; 
        }
        myFile.close();
     }
  }
return;
}




void resort()
{
  tiem_inicio = millis();
  int c=42;
  while (c == 42 and tiempo < 20000)
  {
    fila = "";
    columna = "";
    int n = 1;
    String res = "";
    c=0;
    lcd.setCursor(2, 0);
    lcd.clear();
    lcd.print("INGRESE PRODUCTO");
    Serial.println("INGRESE PRODUCTO");  
    while (c!=35 and c!= 42)                               //elije resorte
    {    
      tiempo = millis() - tiem_inicio;
      if (tiempo>20000)
      {
        c=42;
        error="error";
      }
      char customKey = customKeypad.getKey();
      if (customKey)
     {
        c=int(customKey);
        beep();
        if (c != 35 and c != 42)
        {
          if(n == 1)
          {
            fila = fila + customKey;
            n = n + 1;
          }
          else
          {
            columna = columna + customKey;
          }
           res=res+customKey;
           lcd.clear();
           lcd.setCursor(0, 0);
           lcd.println(res);
           lcd.setCursor(0, 1);
           lcd.print("# CONF * RECH");
        }
        else
        {
          switch (c)
          {
            case 35 :
            resorte = res;// hacer algo
            break ;
            case 42:
            c = 42;//reinicia funcion CARGA RESORTE
            break;
          }
        }
     }
    }
  }
tiempo = 0 ; 
return;
}

//#################### CAMBIO DE CLAVE #####################
String cambioclave(){
  //Definicion de variables
  String varclav1;
  String claveactual;
  String varclav2;
  int cambclav; // es tipo int? es tipo char?
  cambclav=0;
  int q=0;
  
  lcd.setCursor(0, 1);
  lcd.clear();
  lcd.print("CAMBIA CLAVE?");
  lcd.setCursor(0, 2);
  lcd.print("# CONF * RECH");
  delay(2000);

while (cambclav!=35 and cambclav!=42){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    cambclav = customKey; // es tipo int? es tipo char?
    beep();
    if (cambclav==35){
// desarrollo funcion//
      lcd.setCursor(0, 1);
      lcd.clear();
      lcd.print("INGRESE CLAVE");
      lcd.setCursor(0, 2);
      lcd.print("ACTUAL");
      while (q<5) // revisar si el rulo del for hace efecto¿????
      {
        char customKey = customKeypad.getKey();
        if (customKey)
        {
          q++;
          claveactual += customKey;
          beep();
        }
      }
      delay (500);
      q=0;
      if (clave==claveactual)
      {
      lcd.setCursor(0, 1);
      lcd.clear();
      lcd.print("INGRESE NUEVA CLAVE");
      lcd.setCursor(0, 2);
      lcd.print("DE 5 NUMEROS");
      while (q<5)
      {
        char customKey = customKeypad.getKey();
        if (customKey)
        {
          q++;
          varclav1=varclav1+customKey;
          beep();
        }
      }
      q=0;
      lcd.setCursor(0, 1);
      lcd.clear();
      lcd.print("RE-INGRESE");
      lcd.setCursor(0, 2);
      lcd.print("NUEVA CLAVE");
      while (q<5)
      {
        char customKey = customKeypad.getKey();
        if (customKey)
        {
          varclav2=varclav2+customKey;
          beep();
          q++;
        }
      }
      if (varclav1==varclav2)
      {
        lcd.setCursor(0, 0);
        lcd.clear();
        lcd.print("CAMBIO EXITOSO");
        lcd.setCursor(0, 1);
        lcd.print("VUELVA A INGRESAR");
        myFile = SD.open("USUA_DEF.txt",FILE_WRITE);          // Se abre el archivo y se lee desde la posición hasta salto de linea
        myFile.seek(posicion+1);
        myFile.print(varclav1);
        myFile.close();
                                                    DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
                                                    int anio = now.year();
                                                    int mes = now.month();
                                                    int dia = now.day();
                                                    int hora = now.hour();
                                                    int minuto = now.minute();
                                                    myFile = SD.open("mov_gprs.txt", FILE_WRITE);
                                                    myFile.print("CLAVE ,"+Legajo + "," + varclav1 +",");
                                                    myFile.print(dia);
                                                    myFile.print(',');
                                                    myFile.print(mes);
                                                    myFile.print(',');
                                                    myFile.print(anio);
                                                    myFile.print(',');
                                                    myFile.print(hora);
                                                    myFile.print(',');
                                                    myFile.println(minuto);
                                                    myFile.close();
                                                    myFile.close();
                                                    myFile = SD.open("movimien.txt", FILE_WRITE);
                                                    myFile.print("CLAVE ,"+Legajo + "," + varclav1 +",");
                                                    myFile.print(dia);
                                                    myFile.print(',');
                                                    myFile.print(mes);
                                                    myFile.print(',');
                                                    myFile.print(anio);
                                                    myFile.print(',');
                                                    myFile.print(hora);
                                                    myFile.print(',');
                                                    myFile.println(minuto);
                                                    myFile.close();

 
      }
      else
      {
        cambclav==42;
        error="error";
      }
    }
      else
      {
        cambclav==42;
        error="error";
      }
  }
if (cambclav==42)  
error="error";
}
}
return (varclav1);
}

int id =1;

int activity = 3;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600); 
  delay(1000); // Let the module self-initialize
  
}

void loop() {
    switch(activity){
      case 1:
          sendCommand("AT+CWQAP",300);
          sendCommand("AT+CWMODE=3",300);
          sendCommand("AT+CIPMUX=1",300);
          sendCommand("AT+CIPSERVER=1,80",300);
          sendCommand("AT+CWSAP=\"EcoLight1\",\"\",1,0",300);
          activity = 2;
          break;

      case 2:
          wifiConfiguration();
          break;

      case 3:
          delay(1000);
          sendCommand("AT+CIPSERVER=0",300);
          sendCommand("AT+CIPMUX=0",300);
          sendCommand("AT+CWMODE=1",300);
          sendCommand("AT+CIPCLOSE",300);
          activity = 4;
          break;
          
      default:
          DowloadData();
          break;
      
    }
}


void sendCommand(String command, int retardo){
    char dato;
    String data ="";
    Serial.println(command);
    Serial1.println(command);
    delay(retardo); 
    while(Serial1.available()){
       dato = Serial1.read();
       data.concat(dato); 
       
    }

}


String sendCommandData(String command, int retardo){
    char dato;
    String data ="";
    bool enviado = false;
    Serial.println(command);
    Serial1.println(command);
    delay(retardo); 
    while(!enviado){
       while(Serial1.available()){
          dato = (char)Serial1.read();
          data.concat(dato);
       }
       if( data.indexOf("OK") >=0 || data.indexOf("FAIL") >=0 || data.indexOf("ERROR") >=0 || data.indexOf("IP") >=0 ||  data.indexOf(">") >=0){
          enviado = true;
       }
    }
    return data;
}

String sendURL(String URL, int retardo){
    char dato;
    String data ="";
    bool enviado = false;
    Serial.println(URL);
    Serial1.println(URL);
    while(!enviado){
       while(Serial1.available()){
          dato = (char)Serial1.read();
          data.concat(dato);
       }
       if( data.indexOf("CLOSED") >=0 ){
          enviado = true;
       }
    }
    return data;
}





void wifiConfiguration(){
    char dato;
    String data ="";
    while(Serial1.available()){
       dato = (char)Serial1.read();   
       data+=dato; 
       delay(30);
    }
    if( data.indexOf("IPD")>=0 && data.indexOf("ssid")>=0 && data.indexOf("pass")>=0){
       String idPhone = data.substring(data.indexOf("IPD")+4,data.lastIndexOf(","));
       String ssid = data.substring(data.indexOf("ssid")+5,data.indexOf("&"));
       String pass = data.substring(data.indexOf("pass")+5);
       String RespuestaPhone = "ERROR";
       imprimir("idconect ",idPhone);
       data = sendCommandData("AT+CWJAP=\""+ssid+"\",\""+pass+"\"",5000);
       if(data.indexOf("IP") >=0){
          String respuesta = UploadDataID("id=1&nombre=Ninguno&grupo=Ninguno&estado=1&luz=1&distancia=1&retardo=1&encendido=1");
          if(respuesta.indexOf("OK")>=0){
            RespuestaPhone = "OK&1";
            activity = 3;
          }
       }
        data = sendCommandData("AT+CIPSEND="+idPhone+","+String(RespuestaPhone.length()),300);
        if(data.indexOf(">")>=0){
          data = sendCommandData(RespuestaPhone,300);
          imprimir("repsuesta del telefono:",data);
        }
        sendCommand("AT+CIPCLOSE=5",300);              
    }
}

void imprimir(String mensaje,String valor){
  Serial.println(mensaje+valor);
}


String UploadDataID(String variables){
  String data = "";
  data = sendCommandData("AT+CIPSTART=1,\"TCP\",\"sdiaz.nosze.co\",80",1000);
  if(data.indexOf("OK")>= 0){
    String URL = "GET http://sdiaz.nosze.co/prueba1/switch/Upload.php?"+variables;
    data = sendCommandData("AT+CIPSEND=1,"+String(URL.length()+2),300);
    if(data.indexOf(">")){
      data = sendCommandData(URL,300);
      if(data.indexOf("OK") >= 0){
        return "OK";
      }else{
        return "ERROR";
      }
    }else{
      return "ERROR";
    }
  }
}


String UploadData(String variables){
  String data = "";
  data = sendCommandData("AT+CIPSTART=\"TCP\",\"sdiaz.nosze.co\",80",1000);
  if(data.indexOf("OK")>= 0){
    String URL = "GET http://sdiaz.nosze.co/prueba1/switch/Upload.php?"+variables;
    data = sendCommandData("AT+CIPSEND="+String(URL.length()+2),300);
    if(data.indexOf(">")){
      data = sendCommandData(URL,300);
      if(data.indexOf("OK") >= 0){
        return "OK";
      }else{
        return "ERROR";
      }
    }else{
      return "ERROR";
    }
  }
}




void DowloadData(){
  String data = "";
  data = sendCommandData("AT+CIPSTART=\"TCP\",\"sdiaz.nosze.co\",80",700);

  if(data.indexOf("OK")>= 0){
    String URL = "GET http://sdiaz.nosze.co/prueba1/switch/Download.php?id=1";
    data = sendCommandData("AT+CIPSEND="+String(URL.length()+2),300);
    
    if(data.indexOf(">")){
       data = sendURL(URL,600);
       if(data.indexOf("CLOSED")>=0){
          String respuesta = data.substring(data.indexOf(":")+1,data.indexOf("CLOSED"));
          Serial.println(respuesta);
          if(respuesta.indexOf("&")>=0){
              Serial2.println(respuesta);  
          } 
       }
    }
  }
  sendCommand("AT+CIPCLOSE",300);
}

void serialEvent2(){
    //1&esadmaskndpasndpasnd&Ninguno&1&1&1&1&1
    //id&nombre&grupo&estado&encendido&distancia&luz&retardo;
    char dato;
    String data ="";
    while(Serial2.available()){
       dato = (char)Serial2.read();   
       data+=dato; 
    }
    if(data.indexOf("&")>=0){
        String variables= "id=1&nombre="+valueString(1,data)+"&grupo="+valueString(2,data)+"&estado="+valueString(3,data)+"&encendido="+valueString(4,data)+"&distancia="+valueString(5,data)+"&luz="+valueString(6,data)+"&retardo="+valueString(7,data)+""; 
        Serial.println(data);
        variables.trim();
        String llegada = UploadData(variables);
        if(llegada.indexOf("OK")>=0){
             Serial.println("Envio OK");
         }else{
              Serial.println("Envio ERROR");
         }
    }
}


String valueString(int posicion,String data){  
  for(int i=0;i< posicion;i++){
      data = data.substring(data.indexOf("&")+1);
    }
   return data.substring(0,data.indexOf("&"));  
}




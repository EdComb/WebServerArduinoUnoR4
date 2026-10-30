//Servidor Web Arduino Uno R4 con matriz LED (ON/OFF)
//Adaptación: EdComb | https://www.youtube.com/@EdComb

#include "WiFiS3.h" //Librería WiFi específica del Arduino UNO R4 WiFi (chip ESP32-S3)
#include "Arduino_LED_Matrix.h" //Librería de la matriz LED

// ==== Credenciales de la red WiFi ====
const char* ssid = "Nombre de tu red"; //Nombre (SSID) de la red WiFi
const char* password = "Contraseña de tu red"; //Contraseña de la red WiFi
int Led = 13; //Asignación de variable para led

ArduinoLEDMatrix matrix; //Objeto para la matriz de Led

// ==== Patrones para la matriz ====
//Patrón "ON" compacto (8 filas x 12 columnas)
byte patronON[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0 },
  { 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0 },
  { 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0 },
  { 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0 },
  { 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

//Patrón "OFF" compacto (8 filas x 12 columnas)
byte patronOFF[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
  { 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0 },
  { 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0 },
  { 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0 },
  { 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

// ==== Servidor web en el puerto 80 (HTTP estándar) ====
WiFiServer server(80); //Objeto servidor que escucha en el puerto 80

void setup() {
  Serial.begin(115200); //Inicia comunicación serie a 9600 baudios
  //Espera hasta 3 segundos a que se abra el Monitor Serie (no bloquea indefinidamente)
  unsigned long startMillis = millis(); //Guarda el instante actual
  while (!Serial && (millis() - startMillis < 3000)) {
    ;                                               //Espera activa hasta 3s
  }
  Serial.println("\n--- Iniciando Servidor Web ---");

  // Configura el LED integrado de la placa como salida y lo apaga al inicio
  pinMode(Led, OUTPUT); //Configuración de led como salida
  digitalWrite(Led, LOW); //Inicia el Led apagado

  // Inicia la matriz LED
  matrix.begin(); //Inicializa la matriz
  matrix.renderBitmap(patronOFF, 8, 12); //Muestra "OFF" al arrancar

  //Intenta conectar a la red WiFi
  Serial.print("Conectando a la red Wi-Fi: "); 
  Serial.println(ssid);

  //Bucle de conexión: repite hasta que el estado sea "conectado"
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.print("."); //Punto por cada intento fallido
    delay(1000); //Espera 1s entre intentos (estabilidad del módulo)
  }
  //¡SOLUCIÓN AL 0.0.0.0! Espera activa a que el router asigne la IP por DHCP
  Serial.print("\nEsperando asignación de IP por DHCP");
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    Serial.print("."); //Punto mientras la IP siga siendo 0.0.0.0
    delay(500);
  }

  server.begin(); //Inicia el servidor web en el puerto 80

  // Muestra la dirección IP asignada real
  Serial.println("\n¡Conectado exitosamente!");
  Serial.print("Dirección IP de tu Arduino: ");
  Serial.println(WiFi.localIP());
  Serial.println("Copia esta IP y pégala en tu navegador web.");
}

void loop() {
  //Escucha si hay clientes entrantes (navegadores web)
  WiFiClient client = server.available();

  if (client) { //Solo entra si hay un cliente conectado

    Serial.println("Nuevo cliente web conectado");
    String currentLine = "";   //Acumula los caracteres de la línea HTTP actual

    //Mientras el cliente siga conectado, procesa su petición
    while (client.connected()) {

      if (client.available()) { //Hay bytes disponibles para leer

        char c = client.read(); //Lee un byte del cliente
        Serial.write(c); //Lo imprime en el Monitor Serie (depuración)

        if (c == '\n') { //¿Fin de línea?

          //Si la línea está vacía, terminaron los encabezados HTTP → responder
          if (currentLine.length() == 0) {

            // ==== Encabezados de respuesta HTTP ====
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close"); //Cerrar conexión al terminar
            client.println(); //Línea en blanco: fin de encabezados

            // ==== Cuerpo de la página HTML ====
            client.println("<!DOCTYPE html><html>");

            //Cabecera: meta viewport para que se vea bien en móviles
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");

            //Evita que el navegador pida un favicon (icono)
            client.println("<link rel=\"icon\" href=\"data:,\">");

            //==== Estilos CSS con fondo degradado y botones redondeados ====
            client.println("<style>");
            client.println("body { font-family: Helvetica, sans-serif; margin: 0; padding: 0;");
            client.println("display: flex; justify-content: center; align-items: center;");
            client.println("min-height: 100vh; text-align: center;");
            client.println("background: linear-gradient(135deg, #1e3c72 0%, #2a5298 50%, #6a11cb 100%);");
            client.println("color: white; }");
            client.println(".card { background: rgba(255,255,255,0.1); backdrop-filter: blur(10px);");
            client.println("padding: 40px 30px; border-radius: 25px; box-shadow: 0 8px 32px rgba(0,0,0,0.3);");
            client.println("border: 1px solid rgba(255,255,255,0.2); max-width: 500px; }");
            client.println("h1 { font-size: 2em; margin-bottom: 30px; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }");
            client.println(".button { background-color: #4CAF50; border: none; color: white;");
            client.println("padding: 16px 40px; text-decoration: none; font-size: 22px;");
            client.println("margin: 8px; cursor: pointer; border-radius: 50px;");
            client.println("transition: all 0.3s ease; box-shadow: 0 4px 15px rgba(0,0,0,0.2); }");
            client.println(".button:hover { transform: translateY(-3px); box-shadow: 0 8px 20px rgba(0,0,0,0.3); }");
            client.println(".button2 { background-color: #555555; }");
            client.println(".button3 { background-color: #FF0000; margin-top: 30px; }");
            client.println("#estado { margin-top: 20px; font-size: 18px; opacity: 0.9; }");
            client.println("</style></head>");

            //Título de la página
            client.println("<body><div class=\"card\">");
            client.println("<h1>Arduino Uno R4 Web Server</h1>");

            //Botones con onclick (JavaScript) para evitar recarga de página
            client.println("<p><button class=\"button\" onclick=\"control('H')\">ENCENDER LED</button></p>");
            client.println("<p><button class=\"button button2\" onclick=\"control('L')\">APAGAR LED</button></p>");

            //Indicador de estado del LED
            client.println("<p id=\"estado\">Estado: desconocido</p>");

            //Botón de suscripción al canal de YouTube (abre en pestaña nueva)
            client.println("<p><a href=\"https://www.youtube.com/c/EdComb?sub_confirmation=1\" target=\"_blank\"><button class=\"button button3\">Suscribete</button></a></p>");

            //==== JavaScript: envía la petición sin recargar la página ====
            client.println("<script>");
            client.println("function control(accion) {");
            client.println("  fetch('/' + accion).then(() => {");
            client.println("    document.getElementById('estado').innerText =");
            client.println("      accion === 'H' ? 'Estado: LED ENCENDIDO' : 'Estado: LED APAGADO';");
            client.println("  });");
            client.println("}");
            client.println("</script>");

            client.println("</div></body></html>");

            break; //Rompe el bucle: ya se envió la respuesta
          } else {
            currentLine = ""; //Línea no vacía → reinicia para la siguiente
          }
        } else if (c != '\r') { //Ignora el retorno de carro (\r)
          currentLine += c; //Añade el carácter a la línea actual
        }
        // ==== Evalúa la petición para actuar sobre el LED y la matriz ====
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(Led, HIGH); //Enciende el LED
          matrix.renderBitmap(patronON, 8, 12); //Muestra "ON" en la matriz
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(Led, LOW); // Apaga el LED
          matrix.renderBitmap(patronOFF, 8, 12); //Muestra "OFF" en la matriz
        }
      }
    }
    // Cierra la conexión con el cliente
    client.stop();
    Serial.println("Cliente desconectado.");
  }
}
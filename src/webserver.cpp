#include "webserver.h"
#include "relay.h"
#include "serial.h"

static WiFiServer server(HTTP_PORT);

static String header;
static String outputState = "off";

static unsigned long currentTime = millis();
static unsigned long previousTime = 0; 

void wifiConnect(const char* ssid, const char* password)
{
    logger()->print("Connecting to ");
    logger()->println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logger()->print(".");
    }
    // Print local IP address and start web server
    logger()->println("");
    logger()->println("WiFi connected.");
    logger()->println("IP address: ");
    logger()->println(WiFi.localIP());
}

void webServerInit(void)
{
    server.begin();
}

void webServerHandler(void)
{

  WiFiClient client = server.available();  

  if (client) 
  {                             
    logger()->println("Client connect.");          
    String currentLine = "";                
    currentTime = millis();
    previousTime = currentTime;

    while (client.connected() && currentTime - previousTime <= HTTP_TIMEOUT_TIME) 
    { 
      currentTime = millis();         
      if (client.available()) 
      {            
        char c = client.read();            
        logger()->print(c);                   
        header += c;
        
        if (c == '\n') 
        {                  
          if (currentLine.length() == 0) 
          {            
            if (header.indexOf("GET /1/on") >= 0) 
            {
              logger()->println("GPIO 1 on");
              outputState = "on";
              relaySetState(ON);
              client.println("HTTP/1.1 302 Found");
              client.println("Location: /");
            } 
            else if (header.indexOf("GET /1/off") >= 0) 
            {
              logger()->println("GPIO 1 off");
              outputState = "off";
              relaySetState(OFF);
              client.println("HTTP/1.1 302 Found");
              client.println("Location: /");
            }
            else
            {
              client.println("HTTP/1.1 200 OK");
            }
          
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><script>function refresh(refreshPeriod) {setTimeout('location.reload(true)', refreshPeriod);} window.onload = refresh(5000);</script><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            client.println("<body><h1>ESP8266 Web Server</h1>");
                         
            switch (getRelayState())
            {
              case ON:
                outputState = "on";
                break;
              case OFF:
                outputState  = "off";
                break;
            }  
            client.println("<p>Relay - State " + outputState + "</p>");

            if (outputState=="off") 
            {
              client.println("<p><a href=\"/1/on\"><button class=\"button button2\">OFF</button></a></p>");
            } else 
            {
              client.println("<p><a href=\"/1/off\"><button class=\"button\">ON</button></a></p>");
            }
            client.println("</body></html>");
            
            client.println();
          
            break;
          } 
          else 
          { 
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  
          currentLine += c;      
        }
      }
    }
    
    header = "";
    
    client.stop();
    logger()->println("Client disconnected.");
    logger()->println("");
  }

}
    

#include "webserver.h"
#include "serial.h"

static WiFiServer server(HTTP_PORT);

static String header;
static String outputState = "off";

static unsigned long currentTime = millis();
static unsigned long previousTime = 0; 

static const char* m_ssid;
static const char* m_password;

static bool serverOK = false;
bool timeSet = false;

static time_t now;
static struct tm tm; 

static schedule schedules[SCHEDULE_SIZE];
static int8_t nextScheduleIndex = -1; //-1 means schedule deactivated or no schedule
static uint8_t nbSchedule = 0;

static void time_is_set(void);
static void updateTime(void);
static void scheduleHandler(void);

TickTwo timerSchedule(scheduleHandler, 1000, 0, MILLIS);

bool wifiConnect(const char* ssid, const char* password, unsigned long wifiTimeout)
{
    m_ssid = ssid;
    m_password = password;

    previousTime = millis();
    logger()->print("Connecting to ");
    logger()->println(ssid);
    WiFi.begin(ssid, password);
    while ((WiFi.status() != WL_CONNECTED) & (millis() - previousTime < wifiTimeout)) 
    {
      delay(500);
      logger()->print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      logger()->println("");
      logger()->println("WiFi connected.");
      logger()->println("IP address: ");
      logger()->println(WiFi.localIP());
      return true;
    }
    else
    {
      logger()->println("");
      logger()->println("Wifi not connected (connection timeout).");
      return false;
    }
}

bool networkCheck(void)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!serverOK)
    {
      wifiConnect(m_ssid, m_password, WIFI_TIMEOUT);
      webServerInit();
    }
    return true;
  }
  else 
  {
    if (serverOK)
    {
      logger()->println("");
      logger()->println("WiFi not connected (disconnected).");
      serverOK = false;
    }
    return false;
  }
}

void webServerInit(void)
{
    server.begin();
    serverOK = true;
}

void timeNTPInit(void)
{
  configTime(MY_TZ, MY_NTP_SERVER);
  settimeofday_cb(time_is_set);
}

void showTime() {
  updateTime();
  logger()->print("year:");
  logger()->print(tm.tm_year + 1900);  // years since 1900
  logger()->print("\tmonth:");
  logger()->print(tm.tm_mon + 1);      // January = 0 (!)
  logger()->print("\tday:");
  logger()->print(tm.tm_mday);         // day of month
  logger()->print("\thour:");
  logger()->print(tm.tm_hour);         // hours since midnight  0-23
  logger()->print("\tmin:");
  logger()->print(tm.tm_min);          // minutes after the hour  0-59
  logger()->print("\tsec:");
  logger()->print(tm.tm_sec);          // seconds after the minute  0-61*
  logger()->print("\twday");
  logger()->print(tm.tm_wday);         // days since Sunday 0-6
  if (tm.tm_isdst == 1)             // Daylight Saving Time flag
    logger()->print("\tDST");
  else
    logger()->print("\tstandard");
  logger()->println("");
}

void webServerHandler(void)
{
  WiFiClient client = server.accept();  

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

bool addSchedule(uint8_t hour, uint8_t minutes, uint8_t wday, RelayState status)
{
  if (!timeSet)
  {
    logger()->println("Could not sync time, the schedule is deactivated.");
    return false;
  }

  if (nbSchedule >= SCHEDULE_SIZE)
  {
    logger()->println("Add schedule failed, maximum schedule reached.");
    return false;
  }

  if ((hour > 23) || (minutes > 59) || (wday > 0x7F))
  {
    logger()->println("Add schedule failed, wrong time format.");
    return false;
  }

  schedule toAdd;
  toAdd = {status, hour, minutes, wday};
  for(size_t i = 0; i < nbSchedule; i++)
  {
    if ((schedules[i].hour == toAdd.hour) && (schedules[i].minutes == toAdd.minutes) && (schedules[i].wday = toAdd.wday))
    {
      if (schedules[i].status == toAdd.status)
      {
         logger()->println("Schedule already existed.");
      }
      else
      {
        logger()->println("Modify status of existed schedule.");
        schedules[i].status = toAdd.status;
      }
      updateNextSchedule();
      return true;
    }
  }
  schedules[nbSchedule++] = toAdd;
  logger()->println("New schedule added.");
  updateNextSchedule();
  return true;
}

bool deleteSchedule(uint8_t index)
{
  if (index >= nbSchedule)
  {
    logger()->println("Invalid schedule index.");
    return false;
  }
  else 
  {
    nbSchedule--;
    for (size_t i = index; i < nbSchedule; i++)
    {
      schedules[i] = schedules[i+1];
    }
    logger()->print("Delete schedule ");
    logger()->println(index);
    if (nbSchedule == 0)
      nextScheduleIndex = -1;

    return true;
  }
}
 

void updateNextSchedule()
{
  if (!timeSet)
  {
      logger()->println("Could not sync time, the schedule is deactivated.");
      return;
  }

  if (nbSchedule == 0)
    return;

  updateTime();
  nextScheduleIndex = -1;
  uint8_t nowDay = tm.tm_wday;
  int nowMinute = tm.tm_hour * 60 + tm.tm_min;
  int minDifMinute = 1500;
  int dayMinute = 0; 
  int difMinute;

  for(size_t count = 0; count < 7; count++) //dummy variable to avoid infinite loop
  {    
    for (size_t i = 0; i < nbSchedule; i++)
    {
      if (schedules[i].wday & (0x01 << nowDay)) 
      {
        dayMinute = schedules[i].hour * 60 + schedules[i].minutes;
        difMinute = dayMinute - nowMinute;
        if ((difMinute > 0) & (difMinute < minDifMinute))
        {
          minDifMinute = difMinute;
          nextScheduleIndex = i;
        }
      }
    }
    nowDay = (nowDay + 1) % 8;
    if (nextScheduleIndex != -1)
      return;
    
  }
}

static void scheduleHandler()
{
  if (nextScheduleIndex == -1)
    return;

  updateTime();
  if ((tm.tm_hour == schedules[nextScheduleIndex].hour) & (tm.tm_min == schedules[nextScheduleIndex].minutes))
  {
    relaySetState(schedules[nextScheduleIndex].status);
    updateNextSchedule();
  }
}

struct tm getTime()
{
   updateTime();
   return tm;
}

static void updateTime()
{
  time(&now);                       
  localtime_r(&now, &tm);   
}

static void time_is_set()
{
  timeSet = true;
  logger()->println("Schedule is in active.");
  updateNextSchedule();
  timerSchedule.start();
}
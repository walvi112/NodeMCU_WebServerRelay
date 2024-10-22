#include "webserver.h"
#include "serial.h"
#include <string.h>

static WiFiServer server(HTTP_PORT);

static char header[HTTP_HEADER_BUFFER];
static unsigned int headerIndex = 0;
static char body[HTTP_BODY_BUFFER];
static unsigned int bodyIndex = 0;
static char outputState[4] = "Off";


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
static void webServerRelayHTML(WiFiClient client, RequestType requestType);
static void webServerScheduleHTML(WiFiClient client);

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

static void webServerScheduleHTML(WiFiClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.println("<title>NodeMCU Webserver Relay</title>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
  client.println("h1 { text-align: center; }");
  client.println("form { max-width: 400px; margin: 0 auto; padding: 20px; border: 1px solid #ccc; border-radius: 5px; background-color: #f9f9f9; }");
  client.println("label { display: block; margin-bottom: 10px; }");
  client.println("input, select, button { width: 100%; padding: 8px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 4px; }");
  client.println("button { background-color: #4CAF50; color: white; cursor: pointer; }");
  client.println("button:hover { background-color: #45a049; }");
  client.println(".reset_button { background-color: #f44336; }");
  client.println(".reset_button:hover { background-color: #e53935; }");
  client.println(".day { display: -webkit-inline-flex; display: inline-flex; justify-content: space-evenly; align-items: center; padding: 8px; width: -webkit-fill-available;}");
  client.println(".label_title { min-width: 70px; margin: 0; }");
  client.println(".check_box { margin: 0; }");
  client.println(".time { width: calc(100% - 20px); }");
  client.println(".schedule { max-width: 400px; margin: 20px auto; padding: 15px; border: 1px solid #ccc; border-radius: 5px; background-color: #f2f2f2; }");
  client.println(".schedule-item { background-color: #e7f3fe; padding: 10px; border-radius: 5px; margin-bottom: 10px; }");
  client.println(".schedule-item p { margin: 5px 0; }");
  client.println("</style></head>");
  client.println("<body>");

  client.println("<h1>Set Light Schedule</h1>");
  client.println("<form action=\"/setschedule\" method=\"POST\" id=\"lightScheduleForm\">");
  client.println("<p style=\"display:block;margin-bottom:10px;margin-top:0px;\">Days of the week:</p>");
  client.println("<div style=\"display:flex;flex-direction:column;padding-left:5px;margin-bottom:10px\">");

  client.println("<div class=\"day\">");
  client.println("<label class=\"label_title\" for=\"sunday\">Sunday</label>");
  client.println("<input type=\"checkbox\" class=\"check_box\" id=\"sunday\" name=\"sunday\">");
  client.println("</div>");

  client.println("<div class=\"day\">");
  client.println("<label class=\"label_title\" for=\"monday\">Monday</label>");
  client.println("<input type=\"checkbox\" class=\"check_box\" id=\"monday\" name=\"monday\">");
  client.println("</div>");

  client.println("<div class=\"day\">");
  client.println("<label class=\"label_title\" for=\"tuesday\">Tuesday</label>");
  client.println("<input type=\"checkbox\" class=\"check_box\" id=\"tuesday\" name=\"tuesday\">");
  client.println("</div>");

  client.println("<div class=\"day\">");
  client.println("<label class=\"label_title\" for=\"wedneday\">Wednesday</label>");
  client.println("<input type=\"checkbox\" class=\"check_box\" id=\"wedneday\" name=\"wedneday\">");
  client.println("</div>");

  client.println("<div class=\"day\">");
  client.println("<label class=\"label_title\" for=\"thursday\">Thursday</label>");
  client.println("<input type=\"checkbox\" class=\"check_box\" id=\"thursday\" name=\"thursday\">");
  client.println("</div>");

  client.println("<div class=\"day\">");
  client.println("<label class=\"label_title\" for=\"friday\">Friday</label>");
  client.println("<input type=\"checkbox\" class=\"check_box\" id=\"friday\" name=\"friday\">");
  client.println("</div>");

  client.println("<div class=\"day\">");
  client.println("<label class=\"label_title\" for=\"saturday\">Saturday</label>");
  client.println("<input type=\"checkbox\" class=\"check_box\" id=\"saturday\" name=\"saturday\">");
  client.println("</div>");

  client.println("</div>");

  client.println("<div class=\"time\">");
  client.println("<label for=\"time\">Select Time:</label>");
  client.println("<input type=\"time\" id=\"time\" name=\"time\" required>");
  client.println("</div>");

  client.println("<label for=\"status\">Light Status:</label>");
  client.println("<select id=\"status\" name=\"status\" required>");
  client.println("<option value=\"1\">On</option>");
  client.println("<option value=\"0\">Off</option>");
  client.println("</select>");

  client.println("<button type=\"submit\">Set Schedule</button>");
  client.println("<button class=\"reset_button\" type=\"reset\">Reset Field</button>");
  client.println("</form>");

  client.println("<div class=\"schedule\">");
  client.println("<h2>Existing Schedule</h2>");
  client.println("<div id=\"scheduleList\">");

  client.println("</div>");
  client.println("</div>");
  client.println("</body></html>");

  client.println();

}

static void webServerRelayHTML(WiFiClient client, RequestType requestType)
{
  if (requestType != GET_R)
  {
    client.println("HTTP/1.1 302 Found");
    client.println("Location: /");
  }
  else
    client.println("HTTP/1.1 200 OK");

  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html><html>");
  client.println("<head><script>function refresh(refreshPeriod) {setTimeout('location.reload(true)', refreshPeriod);} window.onload = refresh(5000);</script><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<title>NodeMCU Webserver Relay</title>");
  client.println("<style> body {font-familly: Arial, sans-serif; margin: 20px; display: block; margin: 0px auto; text-align: center; max-width: 400px;}");
  client.println("h1 {text-align: center;}");
  client.println("button {background-color: #4CAF50; color: white; cursor: pointer; width: 50%; padding: 16px 40px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 4px; font-size: 25px}");
  client.println("button:hover {background-color: #45A049} </style></head>");


  client.println("<body><h1>NodeMCU Relay Control</h1>");
                
  switch (getRelayState())
  {
    case ON:
      strncpy(outputState, "On", 4);
      // outputState = "On";
      break;
    case OFF:
      strncpy(outputState, "Off", 4);
      // outputState  = "Off";
      break;
  }  
  client.println("<p style=\"font-size: 20px;\">Relay - State " + String(outputState) + "</p>");

  if (strstr(outputState, "Off") != NULL) 
  {
    client.println("<p><a href=\"/1/on\"><button>Turn On</button></a></p>");
  } else 
  {
    client.println("<p><a href=\"/1/off\"><button>Turn Off</button></a></p>");
  }
  client.println("<p><a href=\"/1/schedule\"><button>Set schedule</button></a></p>");
  client.println("</body></html>");
  
  client.println();

}



void webServerHandler(void)
{
  WiFiClient client = server.accept();  
  bool headerRead = false;
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
        headerRead ? body[bodyIndex++] = c : header[headerIndex++] = c;
        if (bodyIndex >= HTTP_BODY_BUFFER || headerIndex >= HTTP_HEADER_BUFFER)
        {
          logger()->println("Request buffer overflow.");
          logger()->println(bodyIndex);
          logger()->println(headerIndex);
          return;
        }
        
        if (c == '\n') 
        {                  
          if (currentLine.length() == 0) 
          {
            if (!headerRead)
            {
              if (strstr(header, "GET") != NULL)
              {
                if (strstr(header, "/1/on") != NULL) 
                {
                  logger()->println("GPIO 1 on");
                  strncpy(outputState, "On", 4);
                  relaySetState(ON);
                  webServerRelayHTML(client, GET_R_ON);
                } 
                else if (strstr(header, "/1/off") != NULL) 
                {
                  logger()->println("GPIO 1 off");
                  strncpy(outputState, "Off", 4);
                  relaySetState(OFF);
                  webServerRelayHTML(client, GET_R_OFF);
                }
                else if(strstr(header, "/1/schedule") != NULL)
                {
                  webServerScheduleHTML(client);
                }
                else
                {
                  webServerRelayHTML(client, GET_R);
                }
                break;
              }
              else if(strstr(header, "POST /setschedule") != NULL)
              {
                headerRead = true;
                client.println("HTTP/1.1 200 OK");
              }
            }
            else
            {
              //implement POST check
              
              break;
            }           
            
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
    
    memset(header, '\0', HTTP_HEADER_BUFFER);
    memset(body, '\0', HTTP_BODY_BUFFER);
    headerIndex = 0;
    bodyIndex = 0;
    
    client.stop();
    logger()->println("\nClient disconnected.");
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
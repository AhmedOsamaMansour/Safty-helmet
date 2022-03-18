#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL ""
#define AUTHOR_PASSWORD ""
#define RECIPIENT_EMAIL ""
SMTPSession smtp;
char auth[] = "dGbAt8YUkr4inlocBzw4fjfBgX0vXziQ";
TinyGPSPlus gps;
SoftwareSerial ss(4, 5); // The serial connection to the GPS device
float latitude , longitude;
int year , month , day, hour , minute , second;
String date_str , time_str , lat_str , lng_str , sms;
int pm;
unsigned long time1;
int vebration = D7, led_wifi = D4, counter = D5;
void smtpCallback(SMTP_Status status)
{
  Serial.println(status.info());
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;
    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
void sendemail()
{
  smtp.debug(1);
  smtp.callback(smtpCallback);
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  SMTP_Message message;

  message.sender.name = "Safety Helmet";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Help Please";
  message.addRecipient("team-12", RECIPIENT_EMAIL);

  String htmlMsg = "<div style=\"color:#FF0000;\"><h1>HELP ME !</h1><p>" + sms + "</p></div>";
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  if (!smtp.connect(&session))return;

  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}

void gps_fn()
{
while (ss.available() > 0) //while data is available
    if (gps.encode(ss.read())) //read gps data
    {
      if (gps.location.isValid()) //check whether gps location is valid
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6); // latitude location is stored in a string
        longitude = gps.location.lng();
        lng_str = String(longitude , 6); //longitude location is stored in a string
      }
      if (gps.date.isValid()) //check whether gps date is valid
      {
        date_str = "";
        day = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
        if (day < 10) date_str = '0';
        date_str += String(day);// values of day are stored in a string
        date_str += " / ";
        if (month < 10) date_str += '0';
        date_str += String(month); // values of month are stored in a string
        date_str += " / ";
        date_str += String(year); // values of year are stored in a string
      }
      if (gps.time.isValid())  //check whether gps time is valid
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
        if (minute > 59)
        {
          minute = minute - 60;
          hour = hour + 1;
        }
        hour = (hour + 2) ;
        if (hour > 23) hour = hour - 24;
        // checking whether AM or PM
        if (hour >= 12) pm = 1;
        else pm = 0;
        if (hour != 12 )hour = hour % 12;
        if (hour < 10)time_str = '0';
        time_str += String(hour); //values of hour are stored in a string
        time_str += " : ";
        if (minute < 10)
          time_str += '0';
        time_str += String(minute); //values of minute are stored in a string
        time_str += " : ";
        if (second < 10)time_str += '0';
        time_str += String(second);    //values of second are stored in a string
        if (pm == 1)time_str += " PM ";
        else time_str += " AM ";
      }
    }
    sms = "http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
    sms += lat_str;
    sms += "+";
    sms += lng_str;
    sms += "<br>";
    sms += date_str;
    sms += "<br>";
    sms += time_str;
    sms += "<br>";
}
void setup()
{
  Serial.begin(115200);
  ss.begin(9600);
  Blynk.begin(auth, WIFI_SSID, WIFI_PASSWORD);
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  pinMode(vebration, INPUT);
  pinMode(led_wifi, OUTPUT);
  pinMode(counter, OUTPUT);
  digitalWrite(led_wifi, HIGH);
  time1=millis();
}
int pin;
BLYNK_WRITE(V5)
{
  pin = param.asInt();
}
bool run1= false,run2 = true;
void loop()
{
  gps_fn();
  Blynk.run();
  int num = digitalRead(vebration);
  if (num == 1 && run2)
  {
    Serial.println("count is started for 30 second");
    run1 = true;
    run2 = false;
    time1 = millis();
    digitalWrite(counter, HIGH);
  }
  if (run1)
  {
    if (pin==0)
    {
      if (millis()- time1 > 30000)
      {
        sendemail();
        time1 = millis();
        run1 = false;
        run2 = true;
        digitalWrite(counter, LOW);
      } 
    }
    else
    {
      run1 = false;
      run2 = true;
      digitalWrite(counter, LOW);
      Serial.println("count is stoped :( ");
    }
  }
 
}

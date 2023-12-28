/*
 * Connect the SD card to the following pins:
 *
 * SD Card  |  ESP32
 *    CS        10
 *    SCK       12
 *    MOSI      11
 *    MISO      13
 *    VCC       5V
 *    GND       GND
 */

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ESPAsyncWebSrv.h>
#include <ModbusIP_ESP8266.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "FileOperations.h"

// power
const int PV_REG = 9;               // Modbus Hreg Offset
const int LOAD_REG = 2181;
const int FIR_ORDER = 100;

IPAddress remote(140, 115, 65, 193);  // Address of Modbus Slave device
const int LOOP_COUNT = 10;

ModbusIP mb;  //ModbusIP object
int fir_filter[FIR_ORDER]={0};

float solar_flaten(float input_mppt)
{
  int i;
  float temp=0;
  for(i=FIR_ORDER-1;i>0;i--)
  {
      fir_filter[i]=fir_filter[i-1];
  }
      
  fir_filter[0]=input_mppt;

  for(i =0;i<FIR_ORDER;i++)
    temp+= fir_filter[i]*(1.0/FIR_ORDER);
    /*
  Serial.print(" temp:");
  Serial.print(temp);*/
  return temp;
}
// end

String readHtmlFromSD(const char *filePath) {
    File file = SD.open(filePath);
    String content = "";

    if (file) {
        while (file.available()) {
        content += (char)file.read();
        }
        file.close();
    } else {
        Serial.println("Failed to open file on SD card");
    }

    return content;
}

const char *ssid = "唐崇祐的iPhone";
const char *password = "cs933600";
const char *ntpServer = "pool.ntp.org";
const long utcOffsetInSeconds = 28800;

AsyncWebServer server(80);

void setup(){
    Serial.begin(115200);

    if (!SD.begin()) {
        Serial.println("Card Mount Failed");
        return;
    }
    
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n\n", cardSize);

    // connect to wifi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
  
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    configTime(utcOffsetInSeconds, 0, ntpServer);
    while (!time(nullptr)) {
      delay(1000);
      Serial.println("Waiting for time sync...");
    }
    Serial.println("Time synced successfully");

    // 定義路由
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
       // 讀取 HTML
       String htmlContent = readHtmlFromSD("/index.html");

       // 發送 HTML WEB
       request->send(200, "text/html", htmlContent);
    });
  
    // 定義第二個路由，處理按鍵
    server.on("/button_pressed", HTTP_GET, [](AsyncWebServerRequest *request){
        // 按鈕被點擊
        Serial.println("Button pressed!");
  
        // 在 SD 卡上紀錄按鈕被按下
        File file = SD.open("/button_log.txt", FILE_APPEND);
        if (file) {
            // 獲取當前時間戳記
            time_t now = time(nullptr);
            
            // 將時間轉換為可讀格式
            char formattedTime[20]; // 預留足夠的空間
            strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", localtime(&now));

            // 將時間戳記和內容一併寫入文件
            int randomNumber = random(4001);
            String randomString = String(randomNumber);
            file.print(formattedTime);
            file.print(" - ");
            file.println(randomString);
            file.close();
            Serial.println("Log written to SD card");
        } else {
            Serial.println("Failed to open file on SD card");
        }
  
        request->send(200, "text/plain", "Button pressed!");
    });

    // 定義新的路由，處理讀取 button_log.txt 的請求
    server.on("/read_log", HTTP_GET, [](AsyncWebServerRequest *request) {
        // 讀取 /button_log.txt 的內容
        String logContent = readHtmlFromSD("/PV_power.txt");

        // 替換換行字元為 HTML 的換行標籤
        //logContent.replace("\n", "<br>");

        // 發送內容給客戶端
        request->send(200, "text/plain", logContent);
    });
  
    // 啟動 server
    server.begin();
}

char CMD[100];
char fileName[90];
char fullPath[90];

// power
uint16_t pv_power = 0;
uint8_t show = LOOP_COUNT;
uint16_t load_power =0;
uint16_t flatten_power = 0;
// end

void loop(){
  if (mb.isConnected(remote)) {   // Check if connection to Modbus Slave is established
    mb.readHreg(remote, PV_REG, &pv_power);  // Initiate Read Coil from Modbus Slave
    mb.readHreg(remote, LOAD_REG, &load_power);
  } else {
    mb.connect(remote);           // Try to connect if no connection
  }
  mb.task();                      // Common local Modbus task
  flatten_power = solar_flaten(pv_power*0.1);
  delay(200);                     // Pulling interval
  if (!show--) {                   // Display Slave register value one time per second (with default settings)
    Serial.print("PV_power:");
    Serial.print(pv_power*0.1);
    Serial.print(" Load_power");
    Serial.print(load_power);
    Serial.print(" Flatten_power");

   // 在 SD 卡上紀錄按鈕被按下
        File file = SD.open("/PV_power.txt", FILE_APPEND);
        if (file) {
            // 獲取當前時間戳記
            time_t now = time(nullptr);
            
            // 將時間轉換為可讀格式
            char formattedTime[20]; // 預留足夠的空間
            strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", localtime(&now));

            // 將時間戳記和內容一併寫入文件
            //int randomNumber = random(4001);
            //String randomString = String(randomNumber);
            file.print(formattedTime);
            file.print(" - ");
            file.println(pv_power*0.1);
            file.close();
            Serial.println("Log written to SD card");
        } else {
            Serial.println("Failed to open file on SD card");
        }
    
    /*
    for(int i = 0 ; i < 25 ; i++)
    {
        Serial.print("filter:");
        Serial.println(fir_filter[i]);
    }*/
      
    Serial.println(flatten_power);
    show = LOOP_COUNT;
  }
  

  /*
  // 等待輸入指令
  Serial.print("command:~$ ");
  while (Serial.available() == 0) {
    // 等待 Serial 可用
  }

  // 讀取輸入的指令，最多讀取 99 個字符，以保留一個位置給 null
  Serial.readBytesUntil('\n', CMD, sizeof(CMD) - 1);

  // 添加 null 終止符，以確保字元陣列是以 null 終止的
  CMD[sizeof(CMD) - 1] = '\0';

  Serial.println(CMD);

  // 根據不同的指令執行相應的操作
  if (strncmp(CMD, "ls", 2) == 0) { // listDir
    listDir(SD, "/", 0);
  } else if (strncmp(CMD, "write", 5) == 0) { // writeFile
    
    strcpy(fileName, CMD + 6);

    // 使用 strtok 將字串分割
    char *token = strtok(fileName, " ");
    
    // 第一個部分，即 "fileName"
    Serial.println(token);

    strcpy(fullPath, "/");
    strcat(fullPath, token);

    strcpy(fullPath, "/");
    strcat(fullPath, token);
    
    // 使用 strtok(NULL, " ") 繼續取得下一個部分
    token = strtok(NULL, "");
    
    // 第二個部分
    Serial.println(token);
    strcat(token, "\n");

    writeFile(SD, fullPath, token);
  } else if (strncmp(CMD, "append", 6) == 0) { // appendFile
    
    strcpy(fileName, CMD + 7);

    // 使用 strtok 將字串分割
    char *token = strtok(fileName, " ");
    
    // 第一個部分，即 "fileName"
    Serial.println(token);

    strcpy(fullPath, "/");
    strcat(fullPath, token);

    strcpy(fullPath, "/");
    strcat(fullPath, token);
    
    // 使用 strtok(NULL, " ") 繼續取得下一個部分
    token = strtok(NULL, "");
    
    // 第二個部分
    Serial.println(token);
    strcat(token, "\n");

    appendFile(SD, fullPath, token);
  } else if (strncmp(CMD, "cat", 3) == 0) { // readFile

    strcpy(fileName, CMD + 4);
    strcpy(fullPath, "/");
    strcat(fullPath, fileName);
    
    readFile(SD, fullPath);
  } else if (strncmp(CMD, "rm", 2) == 0) { // deleteFile

    strcpy(fileName, CMD + 3);
    strcpy(fullPath, "/");
    strcat(fullPath, fileName);
    
    deleteFile(SD, fullPath);
  } else {
    Serial.println("no such command");
  }

  // reset str
  memset(CMD, 0, sizeof(CMD));
  memset(fileName, 0, sizeof(fileName));
  */
}

# h1基於ESP32微控制器之能源管理系統建置

### h2step 1. Connect the SD card to the following pins:
```
SD Card  |  ESP32
   CS        10
   SCK       12
   MOSI      11
   MISO      13
   VCC       5V
   GND       GND
```
### step 2. set your own Modbus Hreg Offset in file WebSDcard.ino#L26
```
const int PV_REG = 0;
const int LOAD_REG = 0;
const int OUTPUT_POWER_REG = 0;
const int DEMAND_REG = 0;
```
### step 3. set Address of Modbus Slave device in WebSDcard.ino#L37

IPAddress remote(0, 0, 0, 0);

### step 4. set wifi in WebSDcard.ino#L48

const char *ssid = ""; // Your wifi ssid
const char *password = ""; // Your wifi ssid

### step 5. start the process and open the Serial Monitor. 
you will see the esp32 ip address on Serial Monitor (it shoud be 192.168.xxx.xxx determined by the internet DHCP)

### step 6. type the esp32 ip address in your browser 
your will see the 能源管理系統's website

### step 7. Extra features：on the WebSDcard.ino#L335 so you can type some command in the Serial Monitor's text bar.

for example：

```
flatten：Flatten_Enable        // 平滑化功能開啟
demand：Demand_Enable          // 需量反應功能開啟
disable：disable all           // 全部功能關閉
ls：listDir in SDcard          // 列出 SDcard 中的 Dir
write：writeFile into SDcard   // 將檔案寫入 SDcard (格式為：write "filename" "content")
append：appendFile             // 繼續寫入檔案 (格式為：append "filename" "content")
cat：readFile                  // 讀檔 (格式為：cat "filename")
rm：deleteFile                 // 刪除檔案 (格式為：rm "filename")
```

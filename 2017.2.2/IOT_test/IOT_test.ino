/*
    本程序可以用来控制四路继电器
    ESP8266烧入此程序直接，使用高低电频控制光耦继电器来控制电灯
    我的继电器默认高电频关闭，所以在初始化时都初始化为高电频，play关闭开启，stop关闭关闭，输入1-4打开或关闭对应的引脚
    代码基于https://github.com/bigiot/bigiotArduino/blob/master/examples/ESP8266/kaiguan/kaiguan.ino
    上的代码进行调整，修复了部分bug，解决了断线重连问题，此代码可以直接烧入到nodemcu模块，分享代码希望对大家有帮助
*/

#include <ESP8266WiFi.h>
#include <aJSON.h>

//=============  此处必须修该============
String DEVICEID="1376"; // 你的设备编号   ==
String  APIKEY = "a576c2924"; // 设备密码==
//=======================================
unsigned long lastCheckInTime = 0; //记录上次报到时间
const unsigned long postingInterval = 40000; // 每隔40秒向服务器报到一次

const char* ssid     = "Xiaomi_3591";//无线名称
const char* password = "gaoyu123456";//无线密码

const char* host = "www.bigiot.net";
const int httpPort = 8181;


boolean LED_Status = LOW; //LED状态的标志位值
void checkIn() ;
void setup() {
  Serial.begin(115200);//打开串口
  delay(1000);
  
  WiFi.begin(ssid, password);//这里便是esp8266连接wifi的语句
  pinMode(13, OUTPUT);//在这里首先设置这一个灯泡的功能为输出
  digitalWrite(13,LOW);//让这个灯泡的初始状态为灭。
  delay(1000);
  digitalWrite(13,HIGH);//让这个灯泡的初始状态为灭。
  delay(1000);
  digitalWrite(13,LOW);//让这个灯泡的初始状态为灭。
}

WiFiClient client;

void loop() {

  while (WiFi.status() != WL_CONNECTED) {    //这里进行判断wifi时候连接成功。
    delay(1000);
    Serial.print(".");
  }

  // Use WiFiClient class to create TCP connections
  if (!client.connected()) {
    if (!client.connect(host, httpPort)) {      //这里判断服务器是否连接成功
      Serial.println("connection failed");
      delay(5000);
      return;
    }
  }
//这里是进过一段时间后进行登陆验证，防止中途登陆失败，数据无法上传
  if(millis() - lastCheckInTime > postingInterval || lastCheckInTime==0) {         
    checkIn();
  }
  
  // Read all the lines of the reply from server and print them to Serial
  if (client.available()) {
    String inputString = client.readStringUntil('\n');
    inputString.trim();
    Serial.println(inputString);
    int len = inputString.length()+1;
    if(inputString.startsWith("{") && inputString.endsWith("}")){
      char jsonString[len];
      inputString.toCharArray(jsonString,len);
      aJsonObject *msg = aJson.parse(jsonString);
      processMessage(msg);
      aJson.deleteItem(msg);          
    }
  }
  int a = random(0,10);
   String msg = "{\"M\":\"update\",\"ID\":\"" + DEVICEID + "\",\"V\":{\"1345\":\"" + a + "\",\"1344\":\"" + LED_Status + "\"}}\n";
   client.print(msg);
//    client.print("{\"M\":\"update\",\"ID\":\"");
//    client.print(DEVICEID);                                 //这里是一个上传数据的语句，我在这里上传了一个随机数，进行数据上传测试。
//    client.print("\",\"V\":{\"");
//    client.print(1345);
//    client.print("\":\"");
//    client.print(a);
//    client.println("\"}}");
  //  Serial.println("update success!");
    //delay(1000);
}

void processMessage(aJsonObject *msg){                      //这里可以参看下bigiot的通讯协议。http://www.bigiot.net/help/1.html
  aJsonObject* method = aJson.getObjectItem(msg, "M");      //这里我其实不是很懂，我自己理解的意思是，从服务器传回的语句中，分析相应的成分做相应的判断。
  aJsonObject* content = aJson.getObjectItem(msg, "C");     //比如“M” 这里表示的位置就是通讯的类型，是登陆验证，还是数据上传，或者是web/App发来的控制信号
  aJsonObject* client_id = aJson.getObjectItem(msg, "ID");  //ID的话，就是设备的ID号。
  if (!method) {
    return;
  }
    String M = method->valuestring;
    if(M == "say"){                                           //在这里进行判断，M，以及C，进行相应的操作，
      String C = content->valuestring;
      String F_C_ID = client_id->valuestring;
      if(C == "GO"){

          digitalWrite(13, HIGH);
       
        sayToClient("U1310","Test Success");    //回传给服务器测试状态
        //LED_Status = LOW;
      }
//      else if(C == "play"){
//        for(int i=0;i<arr_len;i++){
//          state[i] = HIGH;
//          digitalWrite(pins[i], state[i]);
//        }
//        sayToClient(F_C_ID,"LED All on!");   
//         LED_Status = HIGH;
//      }else if(C == "minus"){
//        for(int i=0;i<arr_len;i++){
//          state[i] = LOW;
//          digitalWrite(pins[i], state[i]);
//        }
//          digitalWrite(12, HIGH);
//          sayToClient(F_C_ID,"Geen Light"); 
//          sayToClient("D1376","GO");
//        }else if(C == "plus"){
//        for(int i=0;i<arr_len;i++){
//          state[i] = LOW;
//          digitalWrite(pins[i], state[i]);
//        }
//          digitalWrite(13, HIGH);
//          sayToClient(F_C_ID,"Blue Light"); 
//        }else if(C == "up"){
//        for(int i=0;i<arr_len;i++){
//          state[i] = LOW;
//          digitalWrite(pins[i], state[i]);
//        }
//          digitalWrite(15, HIGH);
//          sayToClient(F_C_ID,"Red Light"); 
//        }
      }
    }


void checkIn() {
    String msg = "{\"M\":\"checkin\",\"ID\":\"" + DEVICEID + "\",\"K\":\"" + APIKEY + "\"}\n";  //这是一个登陆验证操作
    client.print(msg);
    lastCheckInTime = millis(); 
}

void sayToClient(String client_id, String content){
  String msg = "{\"M\":\"say\",\"ID\":\"" + client_id + "\",\"C\":\"" + content + "\"}\n";  //这里是向别的设备或者服务器发送消息的语句。
  client.print(msg);
  lastCheckInTime = millis();
}


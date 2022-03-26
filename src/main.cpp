#include <LwIP.h>
#include <STM32Ethernet.h>

IPAddress ip(192, 168, 3, 4); // STM32側のIPアドレス
IPAddress server(192, 168, 3, 69); // UTM-30LX-EWのIPアドレス
IPAddress subnet(255, 255, 0, 0); // サブネットマスク ipとserverが同一のネットワークになるように設定

EthernetClient client;

void setup() {
  // 
  Ethernet.begin(ip, subnet);
  Serial.begin(921000);
  while (!Serial);
  delay(1000);
  Serial.println("connecting...");

  if (client.connect(server, 10940)) {
    Serial.println("connected");
    client.print("PP\n");
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    client.print("MD0000108001000\n");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    const uint8_t LED_PIN = LED_BUILTIN;
    pinMode(LED_PIN, OUTPUT);
    while(1){
      digitalWrite(LED_PIN, 1);
      delay(1000);
      digitalWrite(LED_PIN, 0);
      delay(1000);
    }
  }
}

int decode(String code)
{
  int value = 0;
  for (int i = 0; i < code.length(); ++i) {
    value <<= 6;
    value &= ~0x3f;
    value |= code.charAt(i) - 0x30;
  }
  return value;
}

int split(String data, char delimiter, String *dst){
    int index = 0;
    int data_length = data.length();
    dst[0] = "";
    for (int i = 0; i < data_length; i++) {
        char tmp = data.charAt(i);
        if(tmp == delimiter){
          index++;
          dst[index] = "";
        } else
          dst[index] += tmp;
    }
    return (index + 1);
}
int all_distance[1081];
void loop() {
  static String data_string = "";
  uint32_t start = micros();
  if (client.available()) {
      char c = client.read();
      data_string += String(c);
      if(data_string.endsWith("\n\n")){
        data_string.remove(data_string.length()-2);
        String splited_data[100] = {""};
        int num = split(data_string, '\n', splited_data);
        String distance_string = "";
        for(int i=3; i<num; i++){
          splited_data[i].remove(splited_data[i].length()-1);
          distance_string += splited_data[i];
        }
        int distance_string_length = distance_string.length();
        if(distance_string_length > 0 && distance_string_length % 3 == 0){
          int distance_string_index = 0;
          int distance_index = 0;
          while (distance_string_index < distance_string_length){
            int distance = decode(distance_string.substring(distance_string_index, distance_string_index+3));
            all_distance[distance_index] = distance;
            distance_index++;
            distance_string_index += 3;
          }
          Serial.println(all_distance[540]);
        }
        data_string = "";
      }
  }
}
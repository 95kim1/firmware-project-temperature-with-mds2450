#include <HardwareSerial.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20PIN 33

#define RXD2 16
#define TXD2 17


/* Create an instance of OneWire */
OneWire oneWire(DS18B20PIN);

DallasTemperature sensor(&oneWire);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: "+String(TX));
  Serial.println("Serial Rxd is on pin: "+String(RX));
  
  sensor.begin();
}

void loop() { 
	int w_flag = 0;
	char OX;
  
	pinMode(RXD2, OUTPUT);
	pinMode(TXD2, INPUT);	
  
	while (1) {
		sensor.requestTemperatures(); 
		float tempinC = sensor.getTempCByIndex(0);
		
		String tempStr = String(tempinC);
		
		String temperStr = "T" + tempStr + "P";
		
		Serial.print(temperStr);
		
		int len = temperStr.length();
		int i;
		for (i = 0; i < len; i++) {
			
			OX = 'X';
			
			pinMode(RXD2, INPUT);
			while ((OX = Serial2.read()) != 'O');
			pinMode(RXD2, OUTPUT);
			
			Serial.write(OX);
			Serial.write('\n');
			
			pinMode(TXD2, OUTPUT);
			Serial2.write('O');
			pinMode(TXD2, INPUT);
			
			Serial2.write(temperStr[i]);
			
		}
	}
}

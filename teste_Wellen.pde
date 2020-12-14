import wellen.*; 
import netP5.*; 
import oscP5.*; 
import processing.serial.*;

// test connecting orientation sensor to wellen, creating a DSP synth

float mFreq = 440.0f;
float mAmp = 0.5f;
int mCounter = 0;

Serial myPort;  // The serial port

String device = "/dev/tty.SLAB_USBtoUART";

String readString = "";

float x = 0;
float y = 0;
float z = 0;
float w = 0;

// PShape rocket;

Quaternion q;


void setup() {
  size(640, 480, P3D);
  DSP.start(this);

  // rocket = loadShape("cassini.obj");
  
  printArray(Serial.list());
  myPort = new Serial(this, device, 115200);
  
  q = new Quaternion(0.0,0.0,0.0,0.0);

}

void draw() {
  while (myPort.available() > 0) {
    delay(1); 
    String input = myPort.readStringUntil('\n');  
    if (input != null) {
      parseSerial(input);
    }
  }
  
  background(255);
  stroke(0);  
  q = q.Euler();

  mFreq = map(q.eulerAngles.x, -1, 1, 55, 440);
  mAmp = map(q.eulerAngles.y, -1, 1, 0, 1);
  
  DSP.draw_buffer(g, width, height);

  /* debug sensor with 3d object
  lights();
  translate(width/2, height/2 + 50, -200);
  scale(5);

  q = q.Euler();
  rotateX(q.eulerAngles.x);
  rotateY(q.eulerAngles.y);
  rotateZ(q.eulerAngles.z);
  shape(rocket);
  */
  
}


void parseSerial(String input) {
    String cleaned = input.substring(13);
    String[] separated = cleaned.split(" ");
    if (separated.length < 4) {
      return;
    }
    x = float(separated[2]);
    y = float(separated[1]);
    z = float(separated[3]);
    w = float(separated[0]);

    q.set(x, y, z, w);
}

void audioblock(float[] pSamples) {
    for (int i = 0; i < pSamples.length; i++) {
        mCounter++;
        pSamples[i] = mAmp * sin(2 * PI * mFreq * mCounter / DSP.get_sample_rate());
    }
}

import wellen.*; 
import netP5.*; 
import oscP5.*; 
import processing.serial.*;

// simple test connecting orientation sensor to wellen

// image to process
int currentX, currentY;
int readerSize = 5;

float mFreq = 440.0f;
float mAmp = 0.5f;
int mCounter = 0;

private final LowPassFilter mFilter = new LowPassFilter(Wellen.DEFAULT_SAMPLING_RATE);


Serial myPort;  // The serial port

String device = "/dev/tty.SLAB_USBtoUART";

String readString = "";

float x = 0;
float y = 0;
float z = 0;
float w = 0;

PGraphics imageBuffer;
PImage imageData;

boolean hasGeneratedTexture = false;

// PShape shape;

Quaternion q;

void setup() {
  
  size(600, 200, P3D);
  imageBuffer = createGraphics(600, 200, P3D);

  DSP.start(this);

  // shape = loadShape("cassini.obj");
  
  printArray(Serial.list());
  myPort = new Serial(this, device, 115200);
  
  q = new Quaternion(0.0,0.0,0.0,0.0);
  
  //imageData = generateRandomTexture();
  imageData = loadImage("noise.png");
}

void draw() {
  background(0);
  
  // delay(30);
  
  while (myPort.available() > 0) {
    delay(1); 
    String input = myPort.readStringUntil('\n');  
    if (input != null) {
      parseSerial(input);
    }
  }
  
  image(imageData,0,0,width,height);
 
  processImage();
  
  
  q = q.Euler();
  // println(q.eulerAngles.x);
  if (!Float.isNaN(q.eulerAngles.x) && !Float.isNaN(q.eulerAngles.y) ) {
    mFilter.set_frequency(map(q.eulerAngles.x, -(float)Math.PI, (float)Math.PI, 1.0f, 5000.0f));
    mFilter.set_resonance(map(q.eulerAngles.y, -(float)Math.PI, (float)Math.PI, 0.0f, 0.97f));
  }
  

  fill(255,255,255);
  DSP.draw_buffer(g, width, height);

  /*
  q = q.Euler();

  mFreq = map(q.eulerAngles.x, -1, 1, 55, 440);
  mAmp = map(q.eulerAngles.y, -1, 1, 0, 1);
  
  DSP.draw_buffer(g, width, height);
  */

  /* debug sensor with 3d object
  lights();
  translate(width/2, height/2 + 50, -200);
  scale(5);

  q = q.Euler();
  rotateX(q.eulerAngles.x);
  rotateY(q.eulerAngles.y);
  rotateZ(q.eulerAngles.z);
  shape(shape);
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

void processImage () {
  
  rect(currentX, currentY, readerSize, readerSize);
 
  int sum = 0;
  imageData.loadPixels();
  for(int x = currentX; x<currentX+readerSize; x++){
    for(int y = currentY; y<currentY+readerSize; y++){
      int index = x + imageData.width*y;
      sum += brightness(imageData.pixels[index]);
    }
  }
  sum /= sq(readerSize);
  mFreq = (sum);
  mAmp = map(z, -1.0, 1.0, 0.0, 1.0);
  println(mAmp);
  currentX +=readerSize;
  if(currentX>imageData.width-readerSize){
    currentX =0;
    currentY += readerSize;
  }
  if(currentY >= imageData.height-readerSize){
    currentY =0;
  } 
  
}

PImage generateRandomTexture () {
    println("begin generate texture");

  imageBuffer.beginDraw();
    //test image
    for (int i=0; i<100; i++) {
      float size = int(random(40));
      fill(255,0,0);
      ellipse(random(height),random(width),size,size);
    }
    
  imageBuffer.endDraw();
  
  assert(imageBuffer.isGL());
  PGraphicsOpenGL g3 = (PGraphicsOpenGL) imageBuffer;
  PImage image = createImage(600, 400, ARGB);
  image.loadPixels();
  g3.getTexture().get(image.pixels);
  image.updatePixels();
  return imageBuffer;
  
}

void audioblock(float[] pSamples) {
    for (int i = 0; i < pSamples.length; i++) {
        mCounter++;
        pSamples[i] = mFilter.process(mAmp * sin(2 * PI * mFreq * mCounter / DSP.get_sample_rate()));
    }
}

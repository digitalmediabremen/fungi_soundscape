
#include "ImageProcessor.h"

// constructor
ImageProcessor::ImageProcessor () {
    drawHeight = 90;

    ofLog() << "initialized";
}


ofxCvGrayscaleImage * ImageProcessor::processImage(ofImage * img) {
    currentImage = *img;
    findContours(img);
    generateMatrix();
    return processedImage;
    }

vector<ofxCvBlob> ImageProcessor::findContours (ofImage * img) {
    
    if (!img || !img->isAllocated())
    {
        ofLog() << "img null";
        return;
    }
    // image to bits
     ofxCvColorImage colorImg;
    
    int width = img->getWidth();
    int height = img->getHeight();
    
    colorImg.allocate(width, height);
    grayImage.allocate(width,height);
    
    colorImg.setFromPixels(img->getPixels());
    grayImage = colorImg; // convert our color image to a grayscale image
    backgroundGrayImage.setFromPixels(grayImage.getPixels());

    grayImage.brightnessContrast(1, 1.6);
    grayImage.blurGaussian(8);
    grayImage.adaptiveThreshold(10);
    //grayImage.threshold(30);

    //grayImage.threshold(30);
    //grayImage.contrastStretch();
    grayImage.dilate();
    grayImage.dilate();
    grayImage.dilate();

    grayImage.invert();


    int totalArea = grayImage.width*grayImage.height;
    int minArea = totalArea * 0.02;
    int maxArea = totalArea * 0.80;
    int nConsidered = 5;

    contourFinder.findContours(grayImage, minArea, maxArea, nConsidered, false, false);
        
    return contourFinder.blobs;
}

void ImageProcessor::generateMatrix() {
    int dataWidth = contourFinder.getWidth();
    int dataHeight = contourFinder.getHeight();
    
    int w = MATRIX_WIDTH;
    int h = MATRIX_HEIGHT;

    int wMultiplier = dataWidth/w;
    int hMultiplier = dataHeight/h;

    backgroundGrayImage.brightnessContrast(1.0f, 3.0f);
    backgroundGrayImage.blurGaussian(50);
    backgroundGrayImage.invert();
    
    ofPixels pix;
    processedImage = new ofxCvGrayscaleImage();

    if (contourFinder.nBlobs > 0) {
        contoursFbo.allocate(dataWidth, dataHeight, GL_RED);
        contoursFbo.begin();
        ofPushMatrix();
        ofPushStyle();
            ofClear(0);
            backgroundGrayImage.draw(0, 0);
            ofSetLineWidth(1000);
             for( int i=0; i<(int)contourFinder.blobs.size(); i++ ) {
               ofNoFill();
               ofBeginShape();
               for( int j=0; j<contourFinder.blobs[i].nPts; j++ ) {
                   ofVertex( contourFinder.blobs[i].pts[j].x, contourFinder.blobs[i].pts[j].y );
               }
               ofEndShape();
           }
        ofPopStyle();
        ofPopMatrix();
        contoursFbo.end();
        
        pix.allocate(dataWidth, dataHeight, OF_IMAGE_GRAYSCALE); /// <-- manually set this.
        contoursFbo.readToPixels(pix);

        ofxCvGrayscaleImage temp;
        temp.setFromPixels(pix);
        processedImage->allocate(w, h);
    
        //colorImg.setFromPixels(pix);
        processedImage->scaleIntoMe(temp, CV_INTER_CUBIC);
        //processedImage->blur(5);
        // processedImage->dilate();
        //processedImage->contrastStretch();
        
        } else {
            processedImage->allocate(w, h);
            //backgroundGrayImage.contrastStretch();
            backgroundGrayImage.invert();
            backgroundGrayImage.blurGaussian(10);
            backgroundGrayImage.adaptiveThreshold(15);
            backgroundGrayImage.erode();
            //backgroundGrayImage.erode();

            processedImage->scaleIntoMe(backgroundGrayImage, CV_INTER_CUBIC);
        }
        

       // processedImage.contrastStretch();
         
        ofLog() << "num channels" << ofToString(pix.getNumChannels());
        /*
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                ofColor thisPixColor;
                if (contourFinder.nBlobs > 0) {
                    thisPixColor = processedImage.getPixels().getColor(i*wMultiplier, j*hMultiplier);
                } else {
                    thisPixColor = grayImage.getPixels().getColor(i*wMultiplier,j*hMultiplier);
                }

                pattern->setColor(i % w, j % h, thisPixColor);
            }
        }
        pattern->update();
     */
}


void ImageProcessor::draw() {
    int space = 200;
    int marginBottom = 500;
    
    int leftMargin = 380;
    int topMargin = 30;
    
    int rightMargin = 430;

    
    float scale = 0.25;

    int imgWidth = int(640 * scale);
    int imgHeight = int(480 * scale);
    


    if (processedImage != NULL) {
            ofPushMatrix();
               ofTranslate(leftMargin, topMargin);
                float ratio = currentImage.getWidth()/currentImage.getHeight();
        
        float containerWidth = ofGetWidth() - (leftMargin + rightMargin);
        float containerHeight = ofGetHeight() - marginBottom;
        
                float finalWidth = currentImage.getWidth() * 1.37;
                float finalHeight = (currentImage.getWidth() / ratio) * 1.37;

        if (finalWidth > containerWidth) {
            finalWidth = containerWidth;
            finalHeight = containerWidth / ratio;
        }
        if (finalHeight > containerHeight) {
             finalWidth = containerHeight * ratio;
             finalHeight = containerHeight;
         }
        
               currentImage.draw(0, 0, finalWidth, finalHeight);
           ofPopMatrix();
        
        ofPushMatrix();
            ofTranslate(leftMargin, containerHeight + 50);
            //ofScale(0.4,0.4,0.4);
            backgroundGrayImage.draw(0, 0, imgWidth, imgHeight);
        ofPopMatrix();


        
        ofPushMatrix();
            ofTranslate(leftMargin + imgWidth + 20, containerHeight + 50);
            //ofScale(0.4,0.4,0.4);
            grayImage.draw(0, 0, imgWidth, imgHeight);
        ofPopMatrix();
        
        
        ofPushMatrix();
            ofTranslate(leftMargin + (imgWidth*2) + 20 * 2, containerHeight + 50);
            processedImage->draw(0,0, imgWidth, imgHeight);
        ofPopMatrix();
        
        if (contourFinder.nBlobs > 0) {
            ofPushMatrix();
            ofPushStyle();
                ofTranslate(leftMargin + (imgWidth*3) + 20 * 3, containerHeight + 50);
                ofScale(scale, scale, scale);
            ofSetLineWidth(1);

            for( int i=0; i<(int)contourFinder.blobs.size(); i++ ) {
                ofNoFill();
                ofBeginShape();
                for( int j=0; j<contourFinder.blobs[i].nPts; j++ ) {
                    ofVertex( contourFinder.blobs[i].pts[j].x, contourFinder.blobs[i].pts[j].y );
                }
                ofEndShape();
            }
            ofPopStyle();
            ofPopMatrix();
        }
         

    }
}

#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

//あらかじめ録画したビデオを使用する場合には、ここをコメントアウト
#define _USE_LIVE_VIDEO

class fiveCircle{
public:
    ofPoint mPoint = ofPoint(0,0);
    int mCirclesize, h;
    ofVec3f velocity;
    ofColor color = ofColor(255,255,255);
    double bright = 255;
    
    fiveCircle(){
        mCirclesize = 50;
        mPoint = ofPoint(ofRandom(mCirclesize, ofGetWidth()-mCirclesize),ofRandom(mCirclesize,ofGetHeight()-mCirclesize));
        velocity.x=ofRandom(-10, 10);
        velocity.y=ofRandom(-10, 10);
        h = ofRandom(255);
    }; //constructor
    ~fiveCircle(){}; //destructer
    void update(double volume){
        color.setHsb(h, 255, volume);
        mPoint = mPoint + velocity;
        if(mPoint.x > ofGetWidth() - mCirclesize || mPoint.x < mCirclesize){
            velocity.x = -velocity.x;
        }
        if(mPoint.y > ofGetHeight() - mCirclesize || mPoint.y < mCirclesize){
            velocity.y = -velocity.y;
        }
        
    }
    void draw(){
        ofFill();
        ofSetColor(color);
        ofSetLineWidth(20);
        ofCircle(mPoint, mCirclesize);
    };
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
#ifdef _USE_LIVE_VIDEO
    //ライブカメラを使用する際には、カメラ入力を準備
    ofVideoGrabber vidGrabber;
#else
    //あらかじめ録画した映像を使用する際には、ビデオプレイヤーを準備
    ofVideoPlayer vidPlayer;
#endif
    
    ofxCvColorImage colorImg; //オリジナルのカラー映像
    ofxCvColorImage colorImgx;
    ofxCvGrayscaleImage grayImage; //グレースケールに変換後
    ofxCvGrayscaleImage grayBg; //キャプチャーした背景画像
    ofxCvGrayscaleImage grayDiff; //現在の画像と背景との差分
    ofxCvContourFinder contourFinder; //輪郭抽出のためのクラス

    
    bool bLearnBakground; //背景画像を登録したか否か
    bool showCvAnalysis; //解析結果を表示するか
    int threshold; //2値化の際の敷居値
    int videoMode; //表示する画像を選択
    
    //Sound,波形
    ofSoundPlayer mySound;
    int GetPrecision=1;
    float *volume;
    int patternMode;
    
    //1
    int angle;
    
    //2 波形
    int slide;//波形を心電図のようにずらしていくための変数
    int elapsedTime;//経過時間
    int red,green,blue;//rgb値(ランダムとかで使う)
    void audioIn(float * input, int bufferSize, int nChannels); //サウンド入力するためのイベント
    vector <float> left;//左チャンネルのベクトル(左側の音声出力について考える)
    ofSoundStream soundStream;//サウンドストリーム(音声の出力入力等をコントロールしている)

    //5
    vector<fiveCircle *> circles;
//    fiveCircle circles[FIVECIRCLE_NUM];
    
};

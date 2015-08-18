#include "ofApp.h"
#define FIVECIRCLE_NUM 60

#define W 320
#define H 240

//--------------------------------------------------------------
void ofApp::setup(){
    //画面の基本設定
    ofBackground(0, 0, 0);
    ofEnableAlphaBlending();
    ofSetFrameRate(60);
    
    //カメラ使用の場合
#ifdef _USE_LIVE_VIDEO
    //カメラから映像を取り込んで表示
    vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(W,H);
#else
    //カメラ不使用の場合ムービーファイルを読み込んで再生
    vidPlayer.loadMovie("fingers.mov");
    vidPlayer.play();
#endif
    
    //使用する画像の領域を確保
    colorImg.allocate(W,H);
    grayImage.allocate(W,H);
    grayBg.allocate(W,H);
    grayDiff.allocate(W,H);
    
    //変数の初期化
    bLearnBakground = true;
    showCvAnalysis = false;
    threshold = 20;
    videoMode = 0;
    
    
    //AudoVisualizer
    
    ofSetCircleResolution(100);
    
    mySound.loadSound("sample.mp3"); //サウンドファイルの読込み
    mySound.setLoop(true); //ループ再生をONに
    mySound.play();
    
    elapsedTime=0;//経過時間を初期化
    
    int bufferSize = 256;//bufferSizeを256に設定
    left.assign(bufferSize, 0.0);//ベクトルの大きさをbufferSize、成分を0.0とする(まずは初期化)
    soundStream.setup(this, 0, 2, 44100, bufferSize, 4);//設定した引数でストリームを開始します。
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);//加算合成
}

//--------------------------------------------------------------
void ofApp::update(){
    //新規フレームの取り込みをリセット
    bool bNewFrame = false;
    
#ifdef _USE_LIVE_VIDEO
    //カメラ使用の場合はカメラから新規フレーム取り込み
    vidGrabber.update();
    //新規にフレームが切り替わったか判定
    bNewFrame = vidGrabber.isFrameNew();
#else
    //カメラ不使用の場合は、ビデオプレーヤーから新規フレーム取り込み
    vidPlayer.idleMovie();
    //新規にフレームが切り替わったか判定
    bNewFrame = vidPlayer.isFrameNew();
#endif
    
    //フレームが切り替わった際のみ画像を解析
    if (bNewFrame){
#ifdef _USE_LIVE_VIDEO
        //取り込んだフレームを画像としてキャプチャ
        colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
        //左右反転
        colorImg.mirror(false, true);
#else
        //取り込んだフレームを画像としてキャプチャ
        colorImg.setFromPixels(vidPlayer.getPixels(), 320,240);
#endif
        //カラーのイメージをグレースケールに変換
        grayImage = colorImg;
        
        //まだ背景画像が記録されていなければ、現在のフレームを背景画像とする
        if (bLearnBakground == true){
            grayBg = grayImage;
            bLearnBakground = false;
        }
        
        //グレースケールのイメージと取り込んだ背景画像との差分を算出
        grayDiff.absDiff(grayBg, grayImage);
        //画像を2値化(白と黒だけに)する
        grayDiff.threshold(threshold);
        grayDiff.invert();
        //2値化した画像から輪郭を抽出する
        contourFinder.findContours(grayDiff, 25, grayDiff.width * grayDiff.height, 10, false, false);
        
    }
    
    //AudoVisualizer
    
    volume = ofSoundGetSpectrum(GetPrecision); //再生中のサウンドの音量を取得
    elapsedTime++;
}

//--------------------------------------------------------------
void ofApp::draw(){
    //現在のモードに応じて、表示する映像を切り替え
    ofSetColor(255);
    switch (videoMode) {
            
        case 1:
            //グレースケール映像
            grayImage.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
            
        case 2:
            //背景画像
            grayBg.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
            
        case 3:
            //2値化された差分映像
            grayDiff.draw(0, 0, ofGetWidth(), ofGetHeight());
            
            break;
            
        default:
            //カラー映像
            colorImg.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
    }
    
    //画面に対する映像の比率を計算
    float ratioX = ofGetWidth()/320;
    float ratioY = ofGetHeight()/240;
    
    //解析結果を表示する場合
    if (showCvAnalysis) {
        //検出した解析結果を表示
        for (int i = 0; i < contourFinder.nBlobs; i++){
            ofPushMatrix();
            //画面サイズいっぱいに表示されるようリスケール
            glScalef((float)ofGetWidth() / (float)grayDiff.width,
                     (float)ofGetHeight() / (float)grayDiff.height,
                     1.0f);
            contourFinder.blobs[i].draw(0,0);
            ofFill();
            ofSetColor(255, 0, 0);
            ofEllipse(contourFinder.blobs[i].centroid.x, contourFinder.blobs[i].centroid.y, 4, 4);
            ofPopMatrix();
        }
    }
    
    //ログと操作説明を表示
    ofSetColor(255, 255, 255);
    ofDrawBitmapString("FPS: "+ofToString(ofGetFrameRate()), 20, 20);
    ofDrawBitmapString("Threshold: "+ofToString(threshold), 20, 35);
    ofDrawBitmapString("Number of Blobs: "+ofToString(contourFinder.nBlobs), 20, 50);
    ofDrawBitmapString("[0] Show original video", 20, 65);
    ofDrawBitmapString("[1] Show grayscale video", 20, 80);
    ofDrawBitmapString("[2] Show captured background", 20, 95);
    ofDrawBitmapString("[3] Show difference from background", 20, 110);
    ofDrawBitmapString("[space] Captuer background", 20, 125);
    ofDrawBitmapString("[a] Analysis on / off", 20, 140);
    
    
    //Audiovisualizer
    
    if(patternMode==0){
        ofPushMatrix();
        ofSetColor(24, 58, 135);
        ofSetLineWidth(50);
        ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
        for(int i=0;i<36;i++){
            ofBeginShape();
            ofRotate(10);
            ofVertex(0, -300);
            if(left[i]<0){
                left[i] = -left[i];
            }
            ofVertex(0, -300-1500*left[i]);
            ofEndShape();
        }
        ofPopMatrix();
        ofSetColor(245, 58, 135);//波形の色を指定
        ofSetLineWidth(30);//線の幅を指定
        ofNoFill();
        slide++;//波形をずらしていく変数をインクリメント
        if(slide==130)slide=0;//130ずれたらリセット
        ofBeginShape();//点を結んだ集合を以下で描画
        for (unsigned int i = 0; i < left.size(); i++){//左チャンネルの配列が終わるまで描画
            //第一引数はx座標,第二引数はy座標(leftの配列の値を表現)
            ofVertex(i*10, ofGetHeight()/2 -left[i+slide]*3000.0f);
        } ofEndShape(false);
    }
    if(patternMode==1){
        ofSetColor(122, 168, 90);
        ofFill();
        ofSetLineWidth(1);
        ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
        ofRotate(angle);
        ofScale(10*volume[0], 10*volume[0]);
        if (angle > 360) {
            angle = 0;
        }
        angle++;
        ofBeginShape();
        ofVertex(-320, 330/10);
        ofVertex(100,-50);
        ofVertex(-140, -350);
        ofVertex(540, 150);
        ofVertex(50, -410);
        ofVertex(-270, 250);
        ofVertex(240, -450);
        ofVertex(490, 110);
        ofEndShape();
        
        ofNoFill();
        ofSetColor(25, 190, 245);
        ofSetLineWidth(10);
        ofCircle(0, 0, ofGetHeight()/2-5000*volume[0]);
    }
    
    //円大量生成
    if (patternMode==5) {
        for(int i = 0; i < FIVECIRCLE_NUM; i++){
            circles.at(i) -> update(500*volume[0]);
            circles.at(i) -> draw();
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //キー入力でモード切り替え
    switch (key){
        case 'q':
            //カラー映像表示
            videoMode = 0;
            break;
            
        case 'w':
            //グレースケール映像表示
            videoMode = 1;
            break;
            
        case 'e':
            //背景画像表示
            videoMode = 2;
            break;
            
        case 'r':
            //2値化した差分映像
            videoMode = 3;
            break;
            
        case 'a':
            //解析結果の表示の on / off
            showCvAnalysis ? showCvAnalysis=false : showCvAnalysis=true;
            break;
            
        case 'f':
            //フルスクリーンに
            ofSetFullscreen(true);
            break;
            
        case ' ':
            //背景画像を新規に取り込む
            bLearnBakground = true;
            break;
            
        case '+':
            //2値化の閾値を増加
            threshold ++;
            if (threshold > 255) threshold = 255;
            break;
            
        case '-':
            //2値化の閾値を減少
            threshold --;
            if (threshold < 0) threshold = 0;
            break;
            
        case '0':
            patternMode = 0;
            break;
            
        case '1':
            patternMode = 1;
            break;
            
        case '2':
            for(int i=0;i<FIVECIRCLE_NUM;i++){
                circles.push_back(new fiveCircle);
            }
            patternMode = 5;
            break;
            
    }
}

//---------------------------以下音の入力イベント----------------------------
void ofApp::audioIn(float *input, int bufferSize, int nChannels){
    if(elapsedTime%2==0){//(8/100)秒に一回波形を取得
        for (int i = 0; i < bufferSize; i++){//
            left[i] = input[i*2]*0.5;//偶数はleftに(左チャンネルのみ取得)            
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

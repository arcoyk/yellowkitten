#include "ofApp.h"
#include <math.h>
#include <vector>
#include <stdlib.h>
#include "ofxNetwork.h"

ofVideoGrabber vidGrabber;
int width = 1024 / 3;
int height = 768 / 3;
ofImage img;
unsigned char *recorded, *show, *tmp, *back, *merged;
int image_size = 3 * width * height;

// play
int play_cnt = 0;
int play_margin = 10;
int play_start = 0;
ofTrueTypeFont myfont;

// record
int record_size = 500;
int rec_cnt = 0;
bool rec_flag = false;
std::ofstream outfile;

// track
std::vector<tracker> curr_trackers;
std::vector< std::vector<tracker> > time_and_trackers;
float sum_distance;
ofxUDPManager udpConnection;
std::vector<tracker> trackers;
int trackers_cnt = 3;

// bgs
int bgs_thre = 150;

typedef struct{
    unsigned char id[4];
    unsigned char x[4];
    unsigned char y[4];
    unsigned char z[4];
    unsigned char roll[4];
    unsigned char pitch[4];
    unsigned char yaw[4];
}type_Mocap_Info;

//--------------------------------------------------------------
void ofApp::setup(){
    //we can now get back a list of devices.
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        cout << devices[i].id << ": " << devices[i].deviceName;
        if( devices[i].bAvailable ){
            cout << endl;
        }else{
            cout << " - unavailable " << endl;
        }
    }
    vidGrabber.setDeviceID(1);

    
    
    vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(width, height);
    recorded = (unsigned char*)malloc(3 * height * width * record_size);
    tmp = (unsigned char*)malloc(3 * height * width);
    back = (unsigned char*)malloc(3 * height * width);
    merged = (unsigned char*)malloc(3 * height * width);
    show = vidGrabber.getPixels();
    udpConnection.Create();
    udpConnection.Bind(1511);
    udpConnection.SetNonBlocking(true);
    for (int i = 0; i < trackers_cnt; i++) {
        tracker t = tracker(i);
        trackers.push_back(t);
    }
    outfile.open("/users/yui/desktop/yellow_imgs/teacher/log.txt", std::ios_base::app);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    show = vidGrabber.getPixels();
    vidGrabber.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    img.setFromPixels(show, width, height, OF_IMAGE_COLOR);
    img.draw(width * 2, 0);
    // save_image(img);
    if (rec_flag) {
        record();
    }
    if (play_cnt++ >= play_margin || play_cnt >= record_size) {
        play_cnt = 0;
        play_start = get_start_point();
    }
    
    show_cut(play_start + play_cnt);
    show_bgs();
    show_back();
    show_merge(play_start + play_cnt);
    // text
    ofDrawBitmapString("play_cnt:" + ofToString(play_cnt), 0, 10);
    ofDrawBitmapString("record_cnt:" + ofToString(rec_cnt), 0, 20);
    ofDrawBitmapString("R", trackers[0].x, trackers[0].y);
    ofDrawBitmapString("G", trackers[1].x, trackers[1].y);
    ofDrawBitmapString("B", trackers[2].x, trackers[2].y);

}

void ofApp::save_image(ofImage img) {
    char buf[1];
    sprintf(buf, "%d", rec_cnt);
    string file_name = "/users/yui/desktop/yellow_imgs/teacher/";
    file_name += buf;
    file_name += ".png";
    img.saveImage(file_name);
}

int ofApp::get_start_point() {
    float min_distance = FLT_MAX;
    int min_index = INT_MAX;
    for (int i = 0; i < time_and_trackers.size(); i++) {
        curr_trackers = get_trackers();
        sum_distance = 0;
        std::vector<tracker> cnd_trackers = time_and_trackers[i];
        for (int k = 0; k < curr_trackers.size(); k++) {
            tracker crr = curr_trackers[k];
            tracker cnd = cnd_trackers[k];
            float x_dis = pow(crr.x - cnd.x, 2);
            float y_dis = pow(crr.y - cnd.y, 2);
            float z_dis = pow(crr.z - cnd.z, 2);
            sum_distance += sqrt(x_dis + y_dis + z_dis);
        }
        if (sum_distance < min_distance) {
            min_distance = sum_distance;
            min_index = i;
        }
    }
    return min_index;
}

void ofApp::record() {
    memcpy(recorded + image_size * rec_cnt, show, image_size);
    time_and_trackers.push_back(get_trackers());
    print_trackers();
    if (rec_cnt++ >= record_size) {
        rec_cnt = 0;
    }
}

void ofApp::show_cut(int play_cnt) {
    memcpy(tmp, recorded + image_size * play_cnt, image_size);
    bgs(tmp);
    img.setFromPixels(tmp, width, height, OF_IMAGE_COLOR);
    img.draw(width, 0);
}

void ofApp::show_bgs() {
    memcpy(tmp, show, image_size);
    bgs(tmp);
    img.setFromPixels(tmp, width, height, OF_IMAGE_COLOR);
    img.draw(0, 0);
}

void ofApp::show_back() {
    img.setFromPixels(back, width, height, OF_IMAGE_COLOR);
    img.draw(0, height);
}


void ofApp::show_merge(int play_cnt) {
    memcpy(tmp, recorded + image_size * play_cnt, image_size);
    memcpy(merged, show, image_size);
    bgs(tmp);
    image_merge(merged, tmp, 0);
    img.setFromPixels(merged, width, height, OF_IMAGE_COLOR);
    img.draw(width, height);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'r' ) {
        rec_flag = !rec_flag;
    } else if (key == 'b') {
        memcpy(back, show, image_size);
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

void ofApp::print_trackers() {
    float x, y, z;
    for (int i = 0; i < trackers.size(); i++) {
        x = trackers[i].x;
        y = trackers[i].y;
        z = trackers[i].z;
        outfile << rec_cnt;
        outfile << ",";
        outfile << i;
        outfile << ",";
        outfile << x;
        outfile << ",";
        outfile << y;
        outfile << ",";
        outfile << z;
        outfile << "\n";
    }
    // printf("%2d\t%3.3fm\t%3.3fm\t%3.3fm\t%3.2f\t%3.2f\t%3.2f\n",*id,*x,*y,*z,*roll,*pitch,*yaw);
}

unsigned char* ofApp::bgs(unsigned char * target) {
    unsigned char tar_c, back_c;
    int d;
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            d = 0;
            for (int c = 0; c < 3; c++) {
                tar_c = target[h * width * 3 + w * 3 + c];
                back_c = back[h * width * 3 + w * 3 + c];
                d += abs(tar_c - back_c);
            }
            if (d < bgs_thre) {
                for (int c = 0; c < 3; c++) {
                    target[h * width * 3 + w * 3 + c] = 0;
                }
            }
        }
    }
}

std::vector<tracker> ofApp::get_trackers() {
    float r_rate, g_rate, b_rate;
    float max_r, max_g, max_b;
    max_r = max_g = max_b = 0;
    int r_h, r_w, g_h, g_w, b_h, b_w, r, g, b;
    r_h = r_w = g_h = g_w = b_h = b_w = 0;
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            r = show[h * width * 3 + w * 3 + 0];
            g = show[h * width * 3 + w * 3 + 1];
            b = show[h * width * 3 + w * 3 + 2];
            
            r_rate = 100 * r / (r + g + b + 1);
            g_rate = 100 * g / (r + g + b + 1);
            b_rate = 100 * b / (r + g + b + 1);
            
            if (max_r < r_rate) {
                max_r = r_rate;
                r_h = h;
                r_w = w;
            }
            if (max_g < g_rate) {
                max_g = g_rate;
                g_h = h;
                g_w = w;
            }
            if (max_b < b_rate) {
                max_b = b_rate;
                b_h = h;
                b_w = w;
            }
        }
    }
    
    trackers[0].x = r_w;
    trackers[0].y = r_h;
    trackers[1].x = g_w;
    trackers[1].y = g_h;
    trackers[2].x = b_w;
    trackers[2].y = b_h;
    
    return trackers;
//    --------------------------------------------------------------MoCap
//    type_Mocap_Info *data = (type_Mocap_Info *)malloc(sizeof(type_Mocap_Info));
//    char recvData[28];
//    udpConnection.Receive(recvData, 28);
//    data = (type_Mocap_Info *)recvData;
//    int* id = (int *)data->id;
//    float* x = (float *)data->x;
//    float* y = (float *)data->y;
//    float* z = (float *)data->z;
//    float* roll= (float *)data->roll;
//    float* pitch= (float *)data->pitch;
//    float* yaw= (float *)data->yaw;
//    
//    trackers[*id].x = *x;
//    trackers[*id].y = *y;
//    trackers[*id].z = *z;
//    
//    udp_rcv();
//    
//    return trackers;
//    --------------------------------------------------------------pseudo
//    std::vector<tracker> trackers;
//    tracker t0 = tracker(0);
//    tracker t1 = tracker(1);
//    tracker t2 = tracker(2);
//    tracker t3 = tracker(3);
//    t0.x = 10;
//    t0.y = 120;
//    t0.z = 21;
//    t1.x = 121;
//    t1.y = 12;
//    t1.z = 233;
//    t2.x = 10;
//    t2.y = 10;
//    t2.z = 21;
//    t3.x = 11;
//    t3.y = 120;
//    t3.z = 212;
//    trackers.push_back(t0);
//    trackers.push_back(t1);
//    trackers.push_back(t2);
//    trackers.push_back(t3);
//    return trackers;
}

void ofApp::udp_rcv() {
    type_Mocap_Info *data = (type_Mocap_Info *)malloc(sizeof(type_Mocap_Info));
    char recvData[28];
    udpConnection.Receive(recvData, 28);
    data = (type_Mocap_Info *)recvData;
    int* id = (int *)data->id;
    float* x = (float *)data->x;
    float* y = (float *)data->y;
    float* z = (float *)data->z;
    float* roll= (float *)data->roll;
    float* pitch= (float *)data->pitch;
    float* yaw= (float *)data->yaw;
    // printf("%2d\t%3.3fm\t%3.3fm\t%3.3fm\t%3.2f\t%3.2f\t%3.2f\n",*id,*x,*y,*z,*roll,*pitch,*yaw);
}


void ofApp::image_merge(unsigned char * target, unsigned char * add, int add_c) {
    unsigned char sum = 0;
    unsigned char new_c;
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
//            for (int c = 0; c < 3; c++) {
//                new_c = add[h * width * 3 + w * 3 + add_c];
//                target[h * width * 3 + w * 3 + add_c] = new_c;
//            }
            sum = 0;
            for (int c = 0; c < 3; c++) {
                sum += add[h * width * 3 + w * 3 + c];
            }
            if (sum != 0) {
                new_c = add[h * width * 3 + w * 3 + add_c];
                target[h * width * 3 + w * 3 + add_c] = new_c;
            }
        }
    }
    
}


#pragma once

#include "ofMain.h"
#include "tracker.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

    void record();
    void show_cut(int play_cnt);
    std::vector<tracker> get_trackers();
    int get_start_point();
    void udp_rcv();
    void print_trackers();
    unsigned char * bgs(unsigned char * target);
    void save_image(ofImage img);
    void show_bgs();
    void show_back();
    void image_merge(unsigned char * target, unsigned char * add, int add_c);
    void show_merge(int play_cnt);
};


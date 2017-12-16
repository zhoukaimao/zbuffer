#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include<iostream>
#include<vector>
using namespace std;
using namespace cv;

struct vec3f;
struct Poly;
struct Edge;

Mat screen;

const int width = 800;
const int height = 700;
const int background = 0x000000;

int POLYCNT = 0;//ploygon count
int EDGECNT = 0;//edge count
Edge* X0;//left border
Edge* X1;//right border
Poly* BG;//background

Edge* ET[height];
vector<Edge*> AET;
vector<Poly*> PT;
vector<Poly*> IPL;


struct vec3f {
	float comp[3];//component
	vec3f() { comp[0] = comp[1] = comp[2] = 0; }
	vec3f(float x, float y, float z) { comp[0] = x; comp[1] = y; comp[2] = z; }
	float getX() { return comp[0]; }
	float getY() { return comp[1]; }
	float getZ() { return comp[2]; }
};

struct Poly {
	int ID;
	float para[4];//a,b,c,d
	int color;//color,ARGB
	bool flag;//in or out
	Poly* next;
	Edge* firste;//first edge
	Poly() { ID = POLYCNT++; }
};
struct Edge {
	int ID;
	int polyId;
	float x;//上端点的x坐标
	float dx;//斜率是k,dx则是-1/k
	float dy;//边跨越的扫描线数目
	int ymax;

	Edge* next;
	Edge() { ID = EDGECNT++; }
};



void draw_line(int x1, int x2,int color) {

}
void build_ply() {

}
void update() {

}

void init() {
	screen = Mat::zeros(height, width, CV_8UC3);
	screen.setTo(0);
	BG = new Poly();
	X0 = new Edge();
	X1 = new Edge();
	X0->x = 0;
	X0->ymax = height;
	X0->dx = 0;
	X0->polyId = ;
	X
}
void load(string filepath) {

}
void scan(int ymin = 0, int ymax = height) {
	build_ET();
	build_PT();

	for (int y = ymin; y < ymax; y++) {

	}
}
int main() {
	init();
	load("example.poly");
	scan();

	imshow("show", screen);
	waitKey();
	return 0;
}
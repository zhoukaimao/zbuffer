#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include<iostream>
#include<fstream>
#include<vector>
using namespace std;
using namespace cv;

struct vec3f;
struct Poly;
struct Edge;

Mat screen;

const int width = 800;
const int height = 700;
const int background = 0x0066ccff;

int POLYCNT = 0;//ploygon count
int EDGECNT = 0;//edge count
Edge* X0;//left border
Edge* X1;//right border
Poly* BG;//background

Edge* ET[height];
vector<Edge*> AET;
vector<Poly*> PT;
vector<Poly*> IPL;

vector<float[3]> vertices;
vector<vector<int> > indices;


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
	float x;//上端点的x坐标
	float dx;//斜率是k,dx则是-1/k
	float dy;//边跨越的扫描线数目
	int ymax;

	Poly* ply;
	Edge* next;
	Edge() { ID = EDGECNT++; }
};


Vec3b int2color(int color) {
	Vec3b c;
	c[2] = (color & 0x00ff0000) >> 16;
	c[1] = (color & 0x0000ff00) >> 8;
	c[0] = color & 0x000000ff;
	return c;
}
void draw_line(int y, int x1, int x2,int color) {
	Vec3b c = int2color(color);
	screen(Rect(x1, y, x2, y)) = c;
}
void build_ply() {

}
void update() {

}

void init() {
	screen = Mat::zeros(height, width, CV_8UC3);
	screen = int2color(background);

	BG = new Poly();
	X0 = new Edge();
	X1 = new Edge();
	X0->x = 0;
	X0->ymax = height-1;
	X0->dx = 0;
	X0->ply = BG;
	X1->x = width - 1;
	X1->ymax = height - 1;
	X1->dx = 0;
	X1->ply = BG;
	X0->next = X1;
	BG->firste = X0;
	BG->color = background;

}
void load(string filepath) {
	ifstream is(filepath);
	if (!is.is_open()) {
		cout << "fail to open file1" << endl;
		exit(1);
	}

}
void scan(int ymin = 0, int ymax = height) {
	//build_ET();
	//build_PT();

	for (int y = ymin; y < ymax; y++) {

	}
}
int main() {
	init();
	load("example.poly");
	build_ET();
	build_PT();
	scan();

	imshow("show", screen);
	waitKey();
	return 0;
}
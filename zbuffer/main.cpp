#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include<iostream>
#include<fstream>
#include<vector>
#include<math.h>
#include   <time.h> 
using namespace std;
using namespace cv;

#define ZEROF 0.000001f

int base_color[8] = { 0x00000000,0x00ff0000,0x0000ff00,0x000000ff,0x00ffff00,0x00ff00ff,0x0000ffff,0x00ffffff };

struct vec3f;
struct Poly;
struct Edge;

cv::Mat screen;

int width = 800;
int height = 700;
const int background = 0x0066ccff;

int POLYCNT = 0;//ploygon count
int EDGECNT = 0;//edge count
Edge* X0;//left border
Edge* X1;//right border
Poly* BG;//background

Edge** ET;//边表，Y筒,organized by ymax
Poly** PT;//多边形表，Y筒,organized by ymax
Edge* AET;//活化边表，按边的x值排列
Poly* IPL;//活化多边形表

vector<vec3f> vertices;
vector<vector<int> > indices;


struct vec3f {
	float comp[3];//component
	vec3f() { comp[0] = comp[1] = comp[2] = 0; }
	vec3f(float x, float y, float z) { comp[0] = x; comp[1] = y; comp[2] = z; }
	float getX() { return comp[0]; }
	float getY() { return comp[1]; }
	float getZ() { return comp[2]; }
	friend ifstream& operator >> (ifstream& is,vec3f& v) {
		is >> v.comp[0] >> v.comp[1] >> v.comp[2];
		return is;
	}
};

struct Poly {
	int ID;
	float para[4];//a,b,c,d
	int dy;//跨越的扫描线个数
	int color;//color,ARGB
	bool flag;//in or out
	Poly* next;
	Poly() { ID = POLYCNT++; }
	float getZ(float x, float y) {
		if (abs(para[2]) < ZEROF) {
			return -INFINITY;
		}
		return (para[0] * x + para[1] * y + para[3]) / (-para[2]);
	}
};
struct Edge {
	int ID;
	float x;//上端点的x坐标
	float dx;//斜率是k,dx则是-1/k
	int dy;//边跨越的扫描线个数
	//int ymax;

	Poly* ply;
	Edge* next;
	Edge() { ID = EDGECNT++; }
	bool lt(const Edge* rh) { return x > rh->x || x == rh->x && dx > rh->dx; }//larger than
};


cv::Vec3b int2color(int color) {
	cv::Vec3b c;
	c[2] = (color & 0x00ff0000) >> 16;
	c[1] = (color & 0x0000ff00) >> 8;
	c[0] = color & 0x000000ff;
	return c;
}
void draw_line(int y, int x1, int x2,int color) {
	cv::Vec3b c = int2color(color);
	if (y<0 || y>height - 1 || x1<0 || x1>width - 1 || x2<0 || x2>width - 1) {
		cout << "out of range!" << endl;
		exit(1);
	}
	//cout << "width:" << x2 - x1 << endl;

	screen(cv::Rect(x1, y,x2-x1,1)) = c;
	//cout << "y:" << y << " x1:" << x1 << " x2:" << x2 << endl;
	//line(screen, cv::Point(1, 1), cv::Point(250, 250), c, 1, CV_AA);
}

void init() {
	screen = cv::Mat::zeros(height, width, CV_8UC3);
	screen = int2color(0x000000ff);

	BG = new Poly();
	X0 = new Edge();
	X1 = new Edge();

	X0->x = 0;
	X0->dy = height;
	X0->dx = 0;
	X0->ply = BG;

	X1->x = width - 1;
	X1->dy = height;
	X1->dx = 0;
	X1->ply = BG;

	BG->color = background;
	BG->dy = height;
	BG->flag = false;
	BG->para[0] = BG->para[1] = 0;
	BG->para[2] = BG->para[3] = 1;//0+0+z+1=0
	
	ET = new Edge*[height];
	PT = new Poly*[height];

	for (int i = 0; i < height; i++) {
		ET[i] = NULL;
		PT[i] = NULL;
	}
	ET[height-1] = X0; X0->next = X1; X1 -> next = NULL;
	PT[height - 1] = BG; BG->next = NULL;

}
vector<string> split(const string &s, const string &seperator) {
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()) {
		//找到字符串中首个不等于分隔符的字母；
		int flag = 0;
		while (i != s.size() && flag == 0) {
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]) {
					++i;
					flag = 0;
					break;
				}
		}

		//找到又一个分隔符，将两个分隔符之间的字符串取出；
		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0) {
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]) {
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j) {
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

int string2int(string s) {
	stringstream ss(s);
	int i;
	ss >> i;
	return i;
}
vector<int> string2int(const vector<string>& src) {
	vector<int> res;
	for (int i = 0; i < src.size(); i++) {
		res.push_back(string2int(src[i]));
	}
	return res;
}
//load data from file into vertices and indices
void load(const char* filepath) {
	ifstream is(filepath);
	if (!is.is_open()) {
		cout << "fail to open file1" << endl;
		exit(1);
	}
	int polycnt = 0;
	int vercnt = 0;
	is >> polycnt >> vercnt;
	for (int i = 0; i < vercnt; i++) {
		vec3f v;
		is >> v;
		vertices.push_back(v);
	}
	string line; getline(is, line);
	for (int i = 0; i < polycnt; i++) {
		getline(is, line);
		vector<int> idx = string2int(split(line, " "));
		indices.push_back(idx);
	}
}
//compute parameter a,b,c,d of polygon
void comp_para(Poly* ply,vec3f v1,vec3f v2,vec3f v3) {//compute parameter
	float x1 = v1.getX(); float y1 = v1.getY(); float z1 = v1.getZ();
	float x2 = v2.getX(); float y2 = v2.getY(); float z2 = v2.getZ();
	float x3 = v3.getX(); float y3 = v3.getY(); float z3 = v3.getZ();
	ply->para[0] = ((y2 - y1)*(z3 - z1) - (z2 - z1)*(y3 - y1));//a
	ply->para[1] = ((z2 - z1)*(x3 - x1) - (x2- x1)*(z3 - z1));//b
	ply->para[2] = ((x2 - x1)*(y3 - y1) - (y2 - y1)*(x3 - x1));//c
	ply->para[3] = (0 - (ply->para[0] *x1 + ply->para[1] *y1 + ply->para[2] *z1));//d
}

//normalize x,y,z to (-1,1)
void normalize() {
	float xmax, xmin, ymax, ymin, zmax, zmin;
	for (int i = 0; i < vertices.size(); i++) {
		float x = vertices[i].comp[0];
		float y = vertices[i].comp[1];
		float z = vertices[i].comp[2];
		if (i == 0) {
			xmax = xmin = x;
			ymax = ymin = y;
			zmax = zmin = z;
		}
		xmax = xmax > x ? xmax : x;
		xmin = xmin < x ? xmin : x;
		ymax = ymax > y ? ymax : y;
		ymin = ymin < y ? ymin : y;
		zmax = zmax > z ? zmax : z;
		zmin = zmin < z ? zmin : z;

	}
	if (xmax - xmin < ZEROF || ymax - ymin < ZEROF || zmax - zmin < ZEROF) {
		cout << "vertices data error!" << endl;
		exit(1);
	}
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].comp[0] = (2 * vertices[i].comp[0] - xmax - xmin) / (xmax - xmin)/0.9;
		vertices[i].comp[1] = (2 * vertices[i].comp[1] - ymax - ymin) / (ymax - ymin)/0.9;
		vertices[i].comp[2] = (2 * vertices[i].comp[2] - zmax - zmin) / (zmax - zmin);
	}
}
//convert x,y from (-1,1) to screen space
void convert_coord() {
	//normalize();
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].comp[0] = (vertices[i].getX() + 1) / 2 * width;
		vertices[i].comp[1] = (vertices[i].getY() + 1) / 2 * height;
	}
}
//build PT, ET from vertices and indices
void build() {
	convert_coord();
	for (int i = 0; i < indices.size(); i++) {
		vector<int> idx = indices[i];
		Poly* ply = new Poly();
		float pymax = -1;//ymax of polygon
		float pymin = height;//ymin of polygon

		for (int j = 0; j < idx.size(); j++) {
			float x1 = vertices[idx[j]].getX();
			float y1 = vertices[idx[j]].getY();
			float x2 = vertices[idx[(j + 1) % idx.size()]].getX();
			float y2 = vertices[idx[(j + 1) % idx.size()]].getY();
			if (abs(y1 - y2) < ZEROF)continue;//horizon line

			Edge* edge = new Edge();
			edge->dx = (x1 - x2) / (y2 - y1);
			edge->dy = abs(floor(y1) - floor(y2));
			edge->x = y1 > y2 ? x1 : x2;
			edge->ply = ply;

			//insert into ET
			int ymax = floor(y1 > y2 ? y1 : y2);
			edge->next = ET[ymax];
			ET[ymax] = edge;

			//update ymax of polygon
			pymax = pymax > y1 ? pymax : y1;
			pymax = pymax > y2 ? pymax : y2;
			pymin = pymin < y1 ? pymin : y1;
			pymin = pymin < y2 ? pymin : y2;
		}
		//srand((unsigned)time(0));
		ply->color = base_color[rand()%8];
		ply->flag = false;
		comp_para(ply, vertices[idx[0]], vertices[idx[1]], vertices[idx[2]]);
		ply->dy = floor(pymax) - floor(pymin);

		//insert into PT
		int ymax = floor(pymax);
		ply->next = PT[ymax];
		PT[ymax] = ply;
		
	}
}
void insert_AET(Edge* edge) {
	if (AET == NULL) {
		AET = edge;
		edge->next = NULL;
	}
	else {
		Edge* e;
		Edge* pre = NULL;
		e = AET;
		while (e != NULL) {
			if (e->lt(edge))break;
			pre = e;
			e = e->next;
		}
		edge->next = e;
		if (pre == NULL) {
			AET = edge;
		}
		else {
			pre->next = edge;
		}
	}
}
//sort AET,gurantee edge->x increase
//insert sort
void sort_AET() {
	Edge* edge;
	Edge* pre;
	Edge* ne;
	edge = AET;
	AET = NULL;
	while (edge != NULL) {
		ne = edge->next;
		insert_AET(edge);
		edge = ne;
	}
}
//update AET,IPL
void update() {

	//AET --
	Edge* edge = AET;
	Edge* prev = NULL;
	while (edge != NULL) {
		edge->dy--;
		edge->x += edge->dx;

		Edge* ne = edge->next;//next edge backup

		if (edge->dy <= 0) {//delete edge
			if (prev == NULL) {
				AET = edge->next;
			}
			else {
				prev->next = edge->next;
			}
			delete edge;
			edge = ne;
		}
		else {
			prev = edge;
			edge = edge->next;
		}

	}

	//IPL--
	Poly* ply = IPL;
	Poly* prevp = NULL;
	while (ply != NULL) {
		ply->dy--;
		Poly* np = ply->next;//polygon backup
							 //delete polygon
		if (ply->dy <= 0) {
			if (prevp == NULL) {
				IPL = ply->next;
			}
			else {
				prevp->next = ply->next;
			}
			delete ply;
			ply = np;
		}
		else {
			prevp = ply;
			ply = ply->next;
		}

	}

	sort_AET();
}

void add_new_edge_ply(int y) {
	Edge* edge;
	Poly* ply;

	//add new edge
	edge = ET[y];

	while (edge != NULL) {

		//find the position to insert
		Edge* ne = edge->next;
		insert_AET(edge);
		edge = ne;
	}

	//add new polygon
	ply = PT[y];
	while (ply != NULL) {
		Poly* np = ply->next;//next poly backup
		ply->next = IPL;
		IPL = ply;
		ply = np;
	}

	//set ipl flag to false
	ply = IPL;
	while (ply != NULL) {
		ply->flag = false;
		ply = ply->next;
	}
}
//scan from top to bottom
void scan(int ymin = 0, int ymax = height-1) {
	int J = 0;
	//init_AET_IPL();
	AET = NULL;
	IPL = NULL;
	for (int y = ymax; y > ymin; y--) {
		add_new_edge_ply(y);
		Edge* edge = AET;
		Edge* prev = NULL;
		vector<Poly*> ipl_inter;//IPL in this interval;
		bool first = true;//first interval or not
		while (edge != NULL) {
			if (prev == NULL) {
				edge->ply->flag = !edge->ply->flag;
				prev = edge;
				edge = edge->next;
				continue;
			}
			//如果prev和edge在同一个polygon上，且与扫描线相较于同一点
			if ( edge->x - prev->x < ZEROF && prev->ply->ID == edge->ply->ID) {
				//如果两条边分别位于扫描线的两侧，则视为同一个点。不做任何操作
				if (edge->dy == 0 && prev->dy > 0 || edge->dy > 0 && prev->dy == 0) {
					continue;
				}

			}
			//计算该区间上的IPL，并按照左端点的深度排序（降序）
			if (first) {
				first = false;
				Poly* p = IPL;
				while(p != NULL) {
					if (p->flag) {
						//找到第一个z值比p的z值小的位置
						int i = 0;
						for (i = 0; i < ipl_inter.size(); i++) {
							if (ipl_inter[i]->getZ(prev->x, y) < p->getZ(prev->x, y)) {
								break;
							}
						}
						ipl_inter.insert(ipl_inter.begin() + i, p);

					}
					p = p -> next;
				}
			}
			else {
				//出多边形
				if (!prev->ply->flag) {
					//删除prev->ply
					int i = 0;
					for (i = 0; i < ipl_inter.size(); i++) {
						if (ipl_inter[i]->ID == prev->ply->ID) {
							break;
						}
					}
					ipl_inter.erase(ipl_inter.begin() + i);
				}
				else {
					//insert prev->ply
					int i = 0;
					for (i = 0; i < ipl_inter.size(); i++) {
						float z1 = ipl_inter[i]->getZ(prev->x, y);
						float z2 = prev->ply->getZ(prev->x, y);
						if (ipl_inter[i]->getZ(prev->x, y) < prev->ply->getZ(prev->x, y)) {
							break;
						}
					}
					ipl_inter.insert(ipl_inter.begin() + i, prev->ply);
				}
			}
			
			if (ipl_inter.size() <= 1) {
				draw_line(y, prev->x, edge->x, ipl_inter[0]->color);
			}
			else {//贯穿
				draw_line(y, prev->x, edge->x, ipl_inter[0]->color);
			}


			edge->ply->flag = !edge->ply->flag;
			prev = edge;
			edge = edge->next;

		}
		update();
	}
}
int main() {
	init();
	load("example01.poly");
	build();
	scan();

	imshow("show", screen);
	cv::waitKey();
	return 0;
}
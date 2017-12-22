#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include<iostream>
#include<fstream>
#include<vector>
#include<math.h>
using namespace std;

#define ZEROF 0.000001f

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

Edge** ET;//�߱�YͲ,organized by ymax
Poly** PT;//����α�YͲ,organized by ymax
Edge* AET;//��߱����ߵ�xֵ����
Poly* IPL;//�����α�

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
	int dy;//��Խ��ɨ���߸���
	int color;//color,ARGB
	bool flag;//in or out
	Poly* next;
	Poly() { ID = POLYCNT++; }
};
struct Edge {
	int ID;
	float x;//�϶˵��x����
	float dx;//б����k,dx����-1/k
	int dy;//�߿�Խ��ɨ���߸���
	//int ymax;

	Poly* ply;
	Edge* next;
	Edge() { ID = EDGECNT++; }
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
	screen(cv::Rect(x1, y, x2, y)) = c;
}

void init() {
	screen = cv::Mat::zeros(height, width, CV_8UC3);
	screen = int2color(background);

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
		//�ҵ��ַ������׸������ڷָ�������ĸ��
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

		//�ҵ���һ���ָ������������ָ���֮����ַ���ȡ����
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
	float x3 = v2.getX(); float y3 = v3.getY(); float z3 = v3.getZ();
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
		vertices[i].comp[0] = (2 * vertices[i].comp[0] - xmax - xmin) / (xmax - xmin);
		vertices[i].comp[1] = (2 * vertices[i].comp[1] - ymax - ymin) / (ymax - ymin);
		vertices[i].comp[2] = (2 * vertices[i].comp[2] - zmax - zmin) / (zmax - zmin);
	}
}
//convert x,y from (-1,1) to screen space
void convert_coord() {
	normalize();
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
			if (y1 - y2 < ZEROF)continue;//horizon line

			Edge* edge = new Edge();
			edge->dx = (x1 - x2) / (y2 - y1);
			edge->dy = abs(floor(y1) - floor(y2)) + 1;
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

		ply->color = rand();
		ply->flag = false;
		comp_para(ply, vertices[idx[0]], vertices[idx[1]], vertices[idx[2]]);
		ply->dy = floor(pymax) - floor(pymin) + 1;

		//insert into PT
		int ymax = floor(pymax);
		ply->next = PT[ymax];
		PT[ymax] = ply;
		
	}
}
//update AET,IPL and ET,PT
void update(int y) {
	//AET--
	//IPL--
	//add new polygon and edge

	//set ipl flag to false
}
//scan from top to bottom
void scan(int ymin = 0, int ymax = height-1) {
	int J = 0;
	AET = ET[height - 1];
	ET[height - 1] = NULL;
	IPL = PT[height - 1];
	PT[height - 1] = NULL;

	for (int y = ymax; y > ymin; y--) {
		Edge* edge = AET;
		Edge* prev = NULL;
		while (edge != NULL) {
			if (prev == NULL) {
				edge->ply->flag != edge->ply->flag;
				prev = edge;
				edge = edge->next;
				continue;
			}
			//���prev��edge��ͬһ��polygon�ϣ�����ɨ���������ͬһ��
			if (prev->x - edge->x < ZEROF && prev->ply->ID == edge->ply->ID) {
				//��������߷ֱ�λ��ɨ���ߵ����࣬����Ϊͬһ���㡣�����κβ���
				if (edge->dy == 0 && prev->dy > 0 || edge->dy > 0 && prev->dy == 0) {
					continue;
				}

			}
			//ɨ��IPL��ȷ��Ҫ���Ƶ���ɫ
			Poly* p = IPL;
			while(p != NULL) {
				//���ֻ��һ��polygon��flag��true��ֱ�ӻ���
				//��������ǰ���polygon����ɫ����
				//�ᴩ��ô�죿
			}
			//�����������


			edge->ply->flag != edge->ply->flag;
			prev = edge;
			edge = edge->next;

		}
		update(y);
	}
}
int main() {
	init();
	load("example.poly");
	build();
	scan();

	imshow("show", screen);
	cv::waitKey();
	return 0;
}
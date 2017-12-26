#pragma once
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include <iostream>
#include <fstream>

#include <math.h>
#include <time.h> 
#pragma once

#include "Util.h"
#include "ScanLine.h"



using namespace std;


int main() {



	init();
	string filename;
	cout << "Please input file name:\n>>";
	cin >> filename;

	load(filename);
	build();
	scan();
	show();
	return 0;
}
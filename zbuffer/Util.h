#pragma once
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include <vector>
#include <string>
using namespace std;

#define ZEROF 0.00001f

int string2int(string s) {
	stringstream ss(s);
	int i;
	ss >> i;
	return i;
}

cv::Vec3b int2color(int color) {
	cv::Vec3b c;
	c[2] = (color & 0x00ff0000) >> 16;
	c[1] = (color & 0x0000ff00) >> 8;
	c[0] = color & 0x000000ff;
	return c;
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

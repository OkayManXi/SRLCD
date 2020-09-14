#pragma once
#ifndef LOOP_CLASS
#define LOOP_CLASS 

class Loop {
public:
	//构造函数
	Loop(int current_frame_in, int loop_frame_in, float similarity_in);
	//成员有当前帧、回环检测database帧和相似度
	int current_frame;
	int loop_frame;
	float similarity;
};


#endif
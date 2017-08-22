//
//  main.cpp
//  MyC
//
//  Created by lichong on 2017/8/16.
//  Copyright © 2017年 lichong. All rights reserved.
//
#include "md5.h"
#include <iostream>
#include<sstream>

extern int a,b;
extern int c;
extern float f;

int main(int argc, const char * argv[]) {
    // insert code here...
    using namespace std;
    string str = "caffe_hiscene";
    MD5 md5(str);
    string result = md5.md5();
    cout << "加密密钥：" << result << endl;
    
    //需要加密的文件
    
    //已经加密的文件 并输出加密文件
    
    //对加密文件进行解密操作 并输出解密文件
    
   int n=111;
    
    ostringstream oss;//用于向string写入,和cout<<一样，仅仅重载了<<
    oss<<n;
    str=oss.str();
    cout<<str<<endl;
    
    return 0;
}

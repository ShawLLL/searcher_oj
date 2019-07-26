#pragma once
#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include<boost/algorithm/string.hpp>
class FileUtil{
public:
        static bool Read(const std::string& file_path,
                         std::string* content){
            std::ifstream file(file_path.c_str());
             
            if(!file.is_open()){
                return false;
            }
            
            //按行读取文件内容
            std::string line ; //每行文件内容
            while(std::getline(file,line)){
                *content += line+"\n";//将读取的每一行相加，合成一个文件
            }
            //读取结束后关闭文件
            file.close();
            return true;
            }
            //对文件进行写操作
            static bool Write(const std::string&  file_path,
                              const std::string& content){
                std::ofstream file(file_path.c_str());
                if(!file.is_open()){
                    return false;
                }
                file.write(content.c_str(),content.size());//写操作        
                file.close();
                return true;
    }
};

class StringUtil{
public:
 static void Split(const std::string& input,
                   const std::string& split_char,

                   std::vector<std::string>* output){                                                   
     boost::split(*output,input,boost::is_any_of(split_char),
                  boost::token_compress_off);


    }
};


#pragma once
#include<string>
#include<vector>
#include<unordered_map>
#include<map>
#include<fstream>
#include"util.hpp"
struct Question{
    std::string id;
    std::string name;
    std::string dir; //题目对应得目录，目录中包含了题目描述/题目的代码框架/题目的测试用例
    std::string star;//难度

    //以下这几个字段要根据diri段来获取到
    std::string desc;   //题目得描述
    std::string header_cpp;//题目的代码框架中的代码
    std::string tail_cpp;//题目的测试用例代码
};

class OjModel{
private:
    //哈希值算法：MD5,SHA1
    std::map<std::string ,Question>model_;
    
public:
     //把文件上的数据加载起来    
    bool Load()
    {
        //1.先加载 oj_config.cfg文件
        std::ifstream file("./oj_data/oj_config.cfg");
        if(!file.is_open())
        {
            return false;
        }
        //2.按行读取此文件，并解析
        std::string line;
        while(std::getline(file,line))
        {
        //3.根据解析结果拼装成Question结构体
            std::vector<std::string> tokens;
            StringUtil::Split(line,"\t",&tokens);
            if(tokens.size()!=4)
            {
                LOG(ERROR)<<"config file format error!\n";
                continue;
            }
        //4.把结构体加入到hash表中
        Question q;
        q.id = tokens[0];
        q.name = tokens[1];
        q.star = tokens[2];
        q.dir = tokens[3];
        FileUtil::Read(q.dir +"./desc.txt",q.desc);
        FileUtil::Read(q.dir +"./header.cpp",q.header_cpp);
        FileUtil::Read(q.dir +"./tail.cpp",q.tail_cpp);
        //[]如果key不存在，就创建新的键值对；如果key存在，就查找对应得value
        model_[q.id] = q;
        }
        file.close();
        LOG(INFO)<<"Load"<<model_.size()<<"question\n";
        return true;
    }

    bool GetAllQuestions(std::vector<Question>*question)const
    {
        //遍历哈希表
        //C++里面学过得很多得性能优化手段，都是在挠痒痒
        //auto推到出的类型是一个键值对
        //pushback需要的类型是Question
        question->clear();
        for(const auto& kv:model_)//c++11新增遍历
        {
                question->push_back(kv.second);
        }
        return true;
    }

    bool GetQuestion(const std::string& id,Question* q)
    {
        auto pos = model_.find(id);
        if(pos == model_.end())
        {
            //该id未找到
            return false;
        }
        *q = pos->second;

        return true;
    }
    
};

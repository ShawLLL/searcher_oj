#pragma once

//#include "cppjieba/Jieba.hpp"
//#include<iostream>
#include <fstream>
#include<vector>
#include<string>
#include<unordered_map>
#include"cppjieba/Jieba.hpp"
//构建索引模块和搜索模块

namespace searcher{

struct DocInfo{
    uint64_t doc_id;//文档编号
    std::string title;
    std::string content;
    std::string url;
};
//Weight类表示:某个词在某个文档中出现过，并且出现的权重
struct Weight{ 
    uint64_t doc_id;  //长整型数据
    int weight; //权重，为了后面键值对排序准备的
                //当前采用词频来计算权重
    std::string key;
};

typedef std::vector<Weight> InverteList;

//索引模块类
class Index{
private:
    //1.正排索引
    //知道 id 获取到对应的文档内容
    //知道vector下标来表示文档id
    std::vector<DocInfo> forward_index_;
    //2.倒排索引
    //知道词，找到对应的id列表
    //unordered_map :时间复杂度为O(1)
    std::unordered_map<std::string,InverteList >inverted_index;
    
    const DocInfo* BulidForward(const std::string& line);
    void BulidInverted(const DocInfo& doc_info);
    
    cppjieba::Jieba jieba_;

public:
    Index();
    //读取raw_input文件，在内存中构建索引
    bool Bulid(const std::string& input_path);
    //查正排索引
    //给定id，找到文档内容
    const DocInfo* GetDocInfo(uint64_t doc_id)const ;
    
    //查倒排索引
    //给定词，找到词在哪个文档出现过
    const InverteList* GetInvertedList(const std::string& key)const;
    
    //分词函数
    
   void CutWord(const std::string& input,std::vector<std::string>* output); 
    
};

//索引模块
class Searcher{
private:
    Index* index_;
    std::string GetDesc(const std::string& content,const std::string& key); //正文提取描述的函数返回值为string
public:
    Searcher():index_(new Index()){

    }
    ~Searcher(){
        delete index_;
    }
    //加载索引
    bool Init(const std::string& input_path);
    //通过特定的格式 在result字符串中表示搜索结果
    bool Search(const std::string& query,std::string* result);
};
} //end searcher





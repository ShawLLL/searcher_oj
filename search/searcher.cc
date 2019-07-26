#include <iostream>
#include "../common/util.hpp"
#include"searcher.h"
#include<algorithm>
#include<jsoncpp/json/json.h>

//#include"../common/util.hpp"                                  

namespace searcher{
/*
 *索引模块
 */
const char* const DICT_PATH = "../jieba_dict/jieba.dict.utf8";
const char* const HMM_PATH = "../jieba_dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../jieba_dict/user.dict.utf8";
const char* const IDF_PATH = "../jieba_dict/idf.utf8";
const char* const STOP_WORD_PATH = "../jieba_dict/stop_words.utf8";

Index::Index():jieba_(DICT_PATH,HMM_PATH,USER_DICT_PATH,
                      IDF_PATH,STOP_WORD_PATH){
}

    //先实现简单函数
    //用指针代替值拷贝，可以代替占内存过大
    const DocInfo* Index::GetDocInfo(uint64_t doc_id)const{
            if(doc_id>=forward_index_.size())
            {
                //如果下标越界，则无解
                return NULL;
            }
            return &forward_index_[doc_id];
            
    }

    const InverteList* Index::GetInvertedList(
                            const std::string& key)const{
           auto pos= inverted_index.find(key);//find返回的是迭代器
           if(pos == inverted_index.end()){
               //没找到
                return NULL;
           } 
           //unordered_map迭代器指向的数据类型 ，键值对
           //所以返回value时，返回地址
            return &pos->second;
    }
    bool Index::Bulid(const std::string& input_path){
        std::cout<<"Index Bulid Start!"<<std::endl;
        //1.按行读取文件内容(每一行就对应一个文档)
        std::ifstream file(input_path.c_str());
        if(!file.is_open())
        {
            std::cout<<"input_path open failed! input_path="
                    <<input_path<<std::endl;
            return false;
        }
        //打开成功后按行读取
        std::string line;
        while(getline(file,line)){ 
            //按行读出这个内容不包含结尾的'\n'
        
    //2.构造 DocInfo 对象，更新正排索引数据
    //  对读到的一行文件进行解析，得到 DocInfo 对象再插入vector
    const DocInfo* doc_info = BulidForward(line);

    //3.更新倒排索引数据
    BulidInverted(*doc_info);
    
        if(doc_info->doc_id % 100==0)
        {
            std::cout<<"Build doc_id="<<doc_info->doc_id<<std::endl;
        }
        }
    std::cout<< "Index Bulid Finish!"<<std::endl;  
    file.close();
    return true;
    }    
    const DocInfo* Index::BulidForward(const std::string& line){
        //1.对这一行数据处理模块中的内容进行切分(\3)
        std::vector<std::string>tokens;//存放切分结果
        //使用strtok进行切分比较麻烦，所以可以使用boost切分
        StringUtil::Split(line,"\3",&tokens);          
        //StringUtil::Split(line,"\3",&tokens);
        if(tokens.size()!=3){
            std::cout<<"tokens not ok"<<std::endl;
            return NULL;
        }
        //2.构造一个 DocInfo 对象
        DocInfo doc_info;   //分别将划分的字符串放入标题，url，正文
        doc_info.doc_id = forward_index_.size();
        doc_info.title = tokens[0];
        doc_info.url = tokens[1];
        doc_info.content=tokens[2];
        //3.把这个对象插入到正排索引中
        forward_index_.push_back(doc_info);
        //由于doc_info为局部变量，所以不能返回地址
        //所以只能返回vector变量:forward_index_ 
        return &forward_index_.back();
    }    
    void Index::BulidInverted(const DocInfo& doc_info){
            //1.对doc_info进行两方面分词:标题和正文
        std::vector<std::string>title_tokens;
        CutWord(doc_info.title,&title_tokens);

        std::vector<std::string>content_tokens;
        CutWord(doc_info.content,&content_tokens);

            //2.对doc_info中标题和正文进行词频统计
        //当前词在标题中出现几次，在正文中出现几次
        //出现在标题出现多的应该权重更重
        struct WordCnt{
            int title_cnt;
            int content_cnt;
        };
        //用一个hash表完成词频的统计
        std::unordered_map<std::string , WordCnt> word_cnt;
        for(auto word:title_tokens){
            //忽略大小写，大小写都出现，算两次
            boost::to_lower(word);//将所有字符转化为小写

            ++word_cnt[word].title_cnt;
        }
        for(auto word:content_tokens){
            boost::to_lower(word);
            ++word_cnt[word].content_cnt;

        }
            //3.遍历分词结果，在倒排索引中查找
        for(const auto& word_pair:word_cnt){
            Weight weight;
            //记录文档编号
            weight.doc_id=doc_info.doc_id;
            //记录词频和权重
            weight.weight = 10*word_pair.second.title_cnt+word_pair.second.content_cnt;
            weight.key=word_pair.first;//first:键值对中的key
            //4.如果该分词在倒排中不存在，就构建新的键值对
            //  如果该分词在倒排中存在，找到对应的值(vector),构建新的weight对象插入到vector中
        InverteList& inverted_list =inverted_index[word_pair.first];
        inverted_list.push_back(weight);
    }
    }    
        void Index::CutWord(const std::string& input,std::vector<std::string>* output)
        {
            jieba_.CutForSearch(input,*output);
        }



/*
 * 搜索模块的实现
 */

bool Searcher::Init(const std::string& input_path)
{
    return index_->Bulid(input_path);
}
bool Searcher::Search(const std::string& query,std::string* json_result){
    //1.<分词>对查询词进行分词
    std::vector<std::string>tokens;
    index_->CutWord(query,&tokens);//调用分词函数

    //2.<触发>针对分词结果进行查倒排索引，找到哪些文档是具有相关性的
    //整合将分词后的查询词的每一个的词频
    std::vector<Weight>all_token_result;
    for(auto word:tokens){
        //将查询此转成小写
        boost::to_lower(word);
        //inverted_list 为查询到的词
        auto* inverted_list = index_->GetInvertedList(word);
        //存在查不到的数据
        if(inverted_list==NULL){
            //如果有某个分词不再索引文档中
            continue;
        }
        //待改进点，不同分词对应统一文档的整合和去重
        //方式：类似合并有序链表
        all_token_result.insert(all_token_result.end(),inverted_list->begin(),inverted_list->end());
    }
    //3.<排序>把这些结果按照一定规则排序
    //sort第三个参数可以使用 仿函数/函数指针/lambda函数
    //此处用lambda函数,lambda函数实际上是匿名函数：[](){} ,一次性函数
    //[]:为函数名，():放入参数，{}:放返回值
    std::sort(all_token_result.begin(),all_token_result.end(),
              [](const Weight& w1,const Weight& w2){
                return  w1.weight>w2.weight;});
    
    //4.<构造结果>查正排索引，按照编号找到每个搜索结果的标题，正文，url
            //搜索结果包含若干条内容，每条内容中又包含了标题，描述url
            //通过序列化的方式把这些内容组织成一个字符串
           //预期结构体数据
           //[
           //   {
           //       "title":"标题"
           //       ”desc“："描述正文"
           //       "url":"url网址"
           //   } 
           //]
    Json::Value results;//所有查询结果
    for(const auto& weight:all_token_result){
        const auto* doc_info = index_->GetDocInfo(weight.doc_id);
        if(doc_info==NULL){
            //如果正查不到
            continue;
        }
        //如果可以查倒，就创建JSON结构
        Json::Value result;//表示一个搜索结果中的JSON对象i
        result["title"]=doc_info->title;
        result["url"] = doc_info->url;
        result["desc"]=GetDesc(doc_info->content,weight.key); //GetDesc：获取正文的部分信息
        results.append(result);//整合result
    }
    //借助jsoncpp能够快速的完成json对象和字符串的转化
    Json::FastWriter writer;
    *json_result = writer.write(results);//利用write进行转化
    return true;
}
std::string Searcher::GetDesc(const std::string& content,
                              const std::string& key){
    //描述中需要包含查询词
    //1.先在正文中查找到正文中的位置
    size_t pos = content.find(key);
    if(pos==std::string::npos){
        //如果查询词只在标题中出现，就从正文开头截取一段话就行
        if(content.size()<250)
        {
            return content;
        }else{
        return content.substr(0,250)+"..'-'.."; //随便定的250个字符
             }
    }
    //2.然后以这词为中心，向前截取一段话，向后截取一段话
    size_t begin_pos = pos<60 ? 0: (pos-60);
    if(begin_pos +160 >= content.size()){
        //begin_pos之后的长度不足以160，就把剩下的内容都算作描述
        return content.substr(begin_pos);
    }else{
        return content.substr(begin_pos,250)+"..'-'..";
    }
    
}

}//end searche

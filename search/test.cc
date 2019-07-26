#include <iostream>
#include"searcher.h"
//调用Index模块，进行验证
#if 0
int main()
{
    searcher::Index index;
    bool ret= index.Bulid("../data/tmp/raw_input");//创建索引
    if(!ret){
        std::cout<<"Build failed"<<std::endl;
        return 1;
    }
    //用倒排索引方式查找
    auto* inverted_list = index.GetInvertedList("filesystem");
    if(inverted_list==  NULL)
    {
        std::cout<<"GetInvertedList failed"<<std::endl;
        return 1;
    }
    for(auto weight: *inverted_list){
        //把存在的id和权重都打出来
        std::cout<<"id:"<<weight.doc_id<<"weight:"<<weight.weight<<std::endl;
        //倒排索引查找到id后，验证一下正排索引是否正确
        const auto* doc_info=index.GetDocInfo(weight.doc_id);
        std::cout<<"title:"<<doc_info->title<<std::endl;
        std::cout<<"url"<<doc_info->url<<std::endl;
     //   std::cout<<"content"<<doc_info->content<<std::endl;
    }

    
    return 0;
}
#endif

int main(){
    //验证搜索模块是否正确
    searcher::Searcher searcher;
    bool ret = searcher.Init("../data/tmp/raw_input");
    if(!ret){
        std::cout<<"searcher failed"<<std::endl;
        return 1;
    }
    std::string query ="简单";
    std::string result;
    searcher.Search(query,&result);
    std::cout<<"result: "<<result;
    return 0;
}

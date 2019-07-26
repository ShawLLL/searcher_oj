#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "httplib.h"
#include "../search/searcher.h"

#define INDEX_HTML "template/searcher.html"
int main()
{
    daemon(1, 1);
    //1.创建 searcher 对象并且进行初始化
    searcher::Searcher s;
    bool ret = s.Init("../data/tmp/raw_input");

    if(!ret){
        std::cout<<"searcher Init fail"<<std::endl;
        return 1;
    }
    using namespace httplib;
    Server server;
    //search? query=firesystem
    server.Get("/", [](const Request &req, Response &rsp){
                std::string body;
                boost::filesystem::path file(INDEX_HTML);
                int64_t fsize = boost::filesystem::file_size(file);
                body.resize(fsize);
                std::ifstream name(INDEX_HTML, std::ios::binary);
                name.read(&body[0], fsize);
                name.close();
                rsp.set_content(body, "text/html");
               });
    server.Get("/search",[&s](const Request&req , Response& res){
               std::string query = req.get_param_value("query");
               std::string result;//查询结果
              //Json::Value result;
        s.Search(query,&result);
                  //解析result
        std::unordered_map<std::string,std::string>body_kv;
//        UrlUilt::
        res.set_content(result,"text/html");
               });
    //查看端口号是否有人使用
    //netstat -anp|grep 9095
    server.listen("0.0.0.0",9099);
    
    return 0;
}


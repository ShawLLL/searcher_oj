#include"httplib.h"
#include<string>
#include<iostream>
#include"compile.hpp"
#include<jsoncpp/json/json.h>
int main()
{
using namespace httplib;
Server server;
//Get("路径"，()[]{});
//lambda表达式
server.Post("/compile",[](const Request& req,Response& resp){
//根据具体的问题场景，根据请求，计算出响应结果
	(void)req; //消除警告
	//TODO如何从req中获取到JSON请求
	//JSON如何和HTTP协议结合
	//JSON如何解析和构造？jsoncpp第三方库
	//需要的请求格式是JSON格式，而HTTP能够提供的格式，是另外一种键值对的格式，所以此处要进行格式的转换
    //此处由于提交的代码可能包含一些特殊符号，这些特殊符号要想正确传输，就需要进行转义，浏览器自动帮我们完成了
    //然后解析这个数据整理成需要的JSON格式
    //键值对，用哪个数据结构表示？
    
    std::unordered_map<std::string,std::string> body_kv;
    UrlUtil:: ParseBody(req.body,&body_kv); 

    //在这里调用CompileAndRun
	/*Json::Value req_json;//从req对象中获取到
	for(std::unordered_map<std::string,std::string>::iterator it = body_kv.begin();it!=body_kv.end();++it)
    {
        req_json[it->first] =  it ->second;
    }*/
    //auto 替换
    Json::Value req_json;
    for(auto p:body_kv){
        //p的类型和*it得到的类型是一致的
        req_json[p.first] = p.second;
    }
    Json::Value resp_json;//resp_json放到响应中
    Compiler::CompileAndRun(req_json,resp_json);
	
	//需要把json::Value对象序列化成一个字符串，才能返回
	Json::FastWriter writer;
	resp.set_content(writer.write(resp_json),"text/plain");//josn将html替换,纯文本内容
	//	resp.set_content("<htm>hello world</html>","text/html");
	//	//html,常识性问题
});//回调函数

    //加上这个目录是为了让浏览器可以访问到这个静态页面
    //静态页面:index.html不会发生变化
    //动态页面：随着参数不同编译结果发生变化
    server.set_base_dir("./wwwroot");
    server.listen("0.0.0.0",9092);
return 0;
}

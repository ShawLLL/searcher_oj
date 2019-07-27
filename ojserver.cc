//#pragma once
#include "httplib.h"
#include<jsoncpp/json/json.h>
#include"oj_view.hpp"
#include"oj_model.hpp"
#include"util.hpp"
#include"compile.hpp"




int main()
{
    OjModel model;
    model.Load();
    using namespace httplib;
    Server server;
    
    server.Get("/all_questions",[&model](const Request& req,Response& resp){
               (void)req;
               std::vector<Question> all_questions;
               model.GetAllQuestions(&all_questions);
               std::string html;
               OjView::RenderAllQuestions(all_questions,&html);
               resp.set_content(html,"text/html");
               });
        // R"()"c++11引入的语法，原始字符串（忽略字符串的转义字符）
        // \d+正则表达式
        
    server.Get(R"(/question/(\d+))",[&model](const Request& req,Response& resp){
                Question question;
                model.GetQuestion(req.matches[1].str(),&question);
               // std::cout<<"ok"<<std::endl;
                LOG(INFO)<<"desc"<<question.desc<<"\n";
                std::string html;
                OjView::RenderQuestion(question,&html);
                resp.set_content(html,"text/html");
               });   
    server.Post(R"(/compile/(\d+))",[&model](const Request& req,Response& resp){
                //1.根据id获取题目信息
                Question question;
                model.GetQuestion(req.matches[1].str(),&question);
                //解析body，获取到用户提交的代码
                std::unordered_map<std::string,std::string> body_kv;                                                                                                 
                UrlUtil:: ParseBody(req.body,&body_kv); 
                const std::string& user_code = body_kv["code"];

                //3.构造JSON结构的参数
                Json::Value req_json;//从req对象中获取到
                //真实需要编译的代码，是用户提交的代码
                //+题目的测试用例代码
                req_json["code"] = user_code +question.tail_cpp;
                Json::Value resp_json; //resp_json放到响应中
                //4.调用编译模块进行编译了
                Compiler::CompileAndRun(req_json,resp_json);
                //5.根据编译结果构造成最终的网页
                std::string html;
                OjView::RenderResult(resp_json["stdout"].asString(),resp_json["reason"].asString(),&html);
                resp.set_content(html,"text/html");
    });   
    server.set_base_dir("./wwwroot");
    server.listen("0.0.0.0",9095);
    return 0;
}


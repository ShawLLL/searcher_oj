#include <iostream>
//数 据处理模块
//把boost文档中涉及到的html进行处理
//1.去标签
//2.把文件进行合并
//  把文档中涉及到的N个HTML的内容合并成一个行文本文件
//  生成的结果是一个大文件，里面包含很多行，每一行对应一个boost文档中的一个html
//  这么做的目的是为了让后面的索引模块处理起来更方便
//3.对文档的结构进行分析，提取出文档的标题，正文，目标url 
//
#include<string>
#include<vector>
#include<fstream>
//为了目录遍历和枚举所用的头文件
#include<boost/filesystem/path.hpp>
#include<boost/filesystem/operations.hpp>
#include"../common/util.hpp"
//const std::string g_input_path = "~/bit/oj/Linux/template";
const std::string g_input_path = "../data/input/";//文档输入路径
const std::string g_output_path = "../data/tmp/raw_input";//输入路径
//doc指的是文档，也就是待搜索的html
//
struct DocInfo{
    std:: string title;
    std:: string content;
    std:: string url;

};

//枚举文件目录
bool EnumFile(const std::string&input_path,
              std::vector<std::string>*file_list){
    namespace fs = boost::filesystem;//boost命名空间
    //input_path是一个字符串，根据这个字符串构造出一个path对象
    fs::path root_path(input_path);
    if(!fs::exists(root_path)){
        std::cout<<"input_path not exitst ! input_path="
                 <<input_path<<std::endl;
        return false;
    }
    //如果对象存在
    //boost递归遍历目录，借助一个特殊的迭代器即可
    //构造一个未初始化的迭代器作为遍历结束标志
    fs::recursive_directory_iterator end_iter;
    for(fs::recursive_directory_iterator iter(root_path);
        iter !=end_iter;++iter){
        //尾插所有文件
        //但是需要去除目录文件和图片
        if(!fs::is_regular_file(*iter))
        {
            //如果不是普通文件，就跳出文件
            continue;
        }
        //所以要根据扩展名只保留.html文件
        if(iter->path().extension()!=".html"){
            continue;
        }
        file_list ->push_back(iter->path().string());

    }
    return true;
}

//解析标题
bool ParseTitle(const std::string& html, std::string* title){
  //1.查找<title> 
  //然后查找</title>
  size_t Titbeg = html.find("<title>");
  if(Titbeg==std::string::npos){
      std::cout<<"<title>not found"<<std::endl;
      return false;
  }
    size_t Titend = html.find("</title>");
    if(Titend==std::string::npos){
        std::cout<<"</title> not found"<<std::endl;
        return false;
    }
  //2.通过字符串取子串的方式取到title标签中的内容
    Titbeg +=std::string("<title>").size();
    if(Titbeg>Titend){
        std::cout<<"begin--end errno!"<<std::endl;
        return false;
    }
    *title+=html.substr(Titbeg,Titend-Titbeg);//截取标题
    return true;
}

//解析正文
//除了标签以外的东西都认为是正文
bool ParseContent(const std::string& html,std::string* content){
    //取正文方法：
    //      一个一个字符的读取：舍弃'<','>'内的内容
    //      如果当前的读取的字符是'<',就认为是标签开始。
    //      接下来的字符都舍弃，一直到遇到'>'就认为标签结束
    //      其余内容都是正文，全部写入
    
    //判断是否为正文：
    bool is_content = true;
    for(auto c:html){
        if(is_content){
        if(c=='<'){
            //不是正文，进入标签内容
            is_content=false;
        }
        else{
            //当前为正文
            if(c=='\n')
            {
                c=' ';
            }
            content->push_back(c);
        }
    }
    else{
        //当前的标签内容
        if(c=='>'){
            is_content=true;
        }
        
    }
    }
    return true;
}

bool ParseUrl(const std::string& file_path ,std::string* url)
{   
    //std::string prefix ="https://www.boost.org/doc/libs/1_64_0/doc/";
    std::string prefix = "http://120.79.62.3:9095/";
    std::string tail = file_path.substr(g_input_path.size());
    *url = prefix+tail;
    
    return true;    
}

bool ParseFile(const std::string& file_path,
               DocInfo* doc_info){
            //1.打开文件，读取文件内容
            std::string html;
            bool ret = FileUtil::Read(file_path,&html);
            if(!ret){
                std::cout<<"Read file failed! file_path="
                         <<file_path<<std::endl;
                return false;
            }
            //2.解析标题
            ret = ParseTitle(html,&doc_info->title);
            if(!ret){
            std::cout<<"ParseTitle failed!file_path="<<file_path<<std::endl;
            return false;
            }
            //3.解析正文，并且去除html标签
            ret = ParseContent(html,&doc_info->content);
            if(!ret){
                std::cout<<"ParseContent failed! file_path="<<file_path<<std::endl;
                return false;
            }
            //4.解析出url
            ret = ParseUrl(file_path,&doc_info->url);
            if(!ret){
                 std::cout<<"ParseUrl failed! file_path="<<file_path<<std::endl;
                return false;
            }
        return true;
}

//C++中的iostream 和 fstream 等这些对象都是禁止拷贝的
//最终输出结果是一个行文本文件，每一行对应一个html文件
//也就是每一行对应一个doc_info
void WriteOutput(const DocInfo& doc_info,std::ofstream& file)
{   
    //将标题，正文，url放在一行中，容易发生粘包问题
    //所以我们使用分隔符方式来防止粘包
    //http解决粘包问题有两种方式:
    //          1.分隔符：例如换行
    //          2.指定长度:但是不灵活
    
    //'\0'   '\3'
    std::string line = doc_info.title + "\3"+
                       doc_info.url+"\3"+
                       doc_info.content+"\n";

    file.write(line.c_str(),line.size());
}

int main()
{
    
    //先搭建整体框架再搭建细节
    //1.枚举出输入路径中所有的html文档的路径
    //vector中的每个元素就是一个文件的路径
    std::vector<std::string>file_list;//文件路径
    bool ret = EnumFile(g_input_path,&file_list);//枚举文件
    if(!ret){
        std::cout<<"EnumFile failed!"<<std::endl;
        return 1;//进程执行完，结果正确，退出码为0，不正确，退出码非0
    }
    //简单的测试代码
    //验证EnumFile是不是正确
    for(const auto& file_path:file_list){
        std::cout<<file_path<<std::endl;
    }

    std::ofstream output_file(g_output_path.c_str());//dui文件进行写造作的对象
    if(!output_file.is_open()){
        //查看打开文件是否成功，成功才能进入写操作
        std::cout<<"open output_file failed! g_output_path = "
                <<g_output_path<<std::endl;
        return 1;
    }
    //2.依次每个枚举出的路径，
    //  对该文件进行分析，
    //  分析出文件的标题，正文，url，并且进行去标签
    for(auto&  path:file_list)
    {
        //C++11中基于区间的循环:range base for
        DocInfo info;
        //输入当前要解析的文件路径
        //输出的是解析之后得到的DoInfo结构
        ret = ParseFile(path,&info);//解析文件
        if(!ret){
            std::cout<<"PaserFile failed! file_path"<<path<<std::endl;
            continue;
        }
    
    //3.把分析结果按照一行的形式写入到输出文件中  
    WriteOutput(info,output_file);

    }
    output_file.close();
 
    return 0;
}


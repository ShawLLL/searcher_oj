#include"util.hpp"
#include<jsoncpp/json/json.h>
#include"compile.hpp"

void test()
{
    Json::Value req;
    req["code"] = "#include<stdio.h>\n int main() {printf(\"hehe\"); return 0;}";
    // req["code"] = "#include <stdio.h> \nint main() \n  {printf(\"hello world!\\n\");\nreturn 0;\n}";
    req["stdin"] = "";

    Json::Value resp;
    Compiler::CompileAndRun(req,resp);
    Json::FastWriter writer;
    LOG(INFO)<<writer.write(resp)<<std::endl;

}

void test2()
{
    int fd = open("./text.txt", O_WRONLY | O_CREAT, 0666);
    if(fd < 0)
    {
        std::cerr << "open" << std::endl;
        exit(1);
    }
    std::string str;
    FileUtil::Read("text.txt", str);
    std::cout << str << std::endl;
}
int main()
{
    test();
    return 0;
}

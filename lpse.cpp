#include <node.h>
#include <v8.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <cstring>

using namespace v8;


char* ValueToChar(Handle<Value> val) {
    String::Utf8Value s(val->ToString());
    return (char*)std::string(*s, s.length()).data();
}

char** Params(const Arguments& args) {
    char **results = new char*[args.Length()+1];
    for (int i = 0; i < args.Length(); i++) {
        char *arg = ValueToChar(args[i]);
        results[i]=arg;
    }
    results[args.Length()] = NULL;
    return results;
}


Handle<Value> sexec(const Arguments& args) {
    HandleScope scope;
    int stdoutPipeFD[2];
    int stderrPipeFD[2];
    
    if (pipe(stdoutPipeFD) == -1 || pipe(stderrPipeFD) == -1) {
        return ThrowException(String::New("Pipe not created"));
    }
    if (fork() == 0) {
        char **params = Params(args);
        close(stdoutPipeFD[0]);
        close(stderrPipeFD[0]);
        dup2(stdoutPipeFD[1], 1);
        dup2(stderrPipeFD[1], 2);
        close(stdoutPipeFD[1]);
        close(stderrPipeFD[1]);
        execvp(params[0], params);
    } else {
        char buff[1024];
        Local<String> stdout = String::Empty();
        Local<String> stderr = String::Empty();
        Local<Object> ret = Object::New();
        int readed = 0;
        
        ret->Set(
            String::NewSymbol("target"),
            args[0]
         );
        
        close(stdoutPipeFD[1]);
        close(stderrPipeFD[1]);
        while ((readed = read(stdoutPipeFD[0], buff, sizeof(buff))) != 0) {
            stdout = String::Concat(stdout, String::New(buff, readed));
        }
        ret->Set(
            String::NewSymbol("out"),
            stdout
        );
        while ((readed = read(stderrPipeFD[0], buff, sizeof(buff))) != 0) {
            stdout = String::Concat(stderr, String::New(buff, readed));
        }
        ret->Set(
            String::NewSymbol("err"),
            stderr
        );
        return scope.Close(ret);
    }
    return scope.Close(String::New("null"));
}

void init(Handle<Object> target) {
    target->Set(String::New("exec"), FunctionTemplate::New(sexec)->GetFunction());
}

NODE_MODULE(pse, init);

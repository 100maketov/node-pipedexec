#include <node.h>
#include <v8.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <cstring>

using namespace v8;

Handle<Value> sexec(const Arguments& args) {
    HandleScope scope;
    int stdoutPipeFD[2];
    int stderrPipeFD[2];
    char **arguments = new char*[args.Length()+1];
    Handle<String> target = String::Empty();
    Handle<String> space = String::New(" ");
    
    if (pipe(stdoutPipeFD) == -1 || pipe(stderrPipeFD) == -1) {
        return ThrowException(String::New("Pipe not created"));
    }
    
    for (int i = 0; i < args.Length(); i++) {
        String::Utf8Value* s = new String::Utf8Value(args[i]->ToString());
        arguments[i] = new char[s->length()];
        strcpy(arguments[i], **s);
        delete s;
        target = String::Concat(target, args[i]->ToString());
        if (i < args.Length()-1) {
            target = String::Concat(target, space);
        }
    }
    arguments[args.Length()] = NULL;
    
    if (fork() == 0) {
        close(stdoutPipeFD[0]);
        close(stderrPipeFD[0]);
        dup2(stdoutPipeFD[1], 1);
        dup2(stderrPipeFD[1], 2);
        close(stdoutPipeFD[1]);
        close(stderrPipeFD[1]);
        execvp(arguments[0], arguments);
    } else {
        char buff[1024];
        Local<String> stdout = String::Empty();
        Local<String> stderr = String::Empty();
        Local<Object> ret = Object::New();
        int readed = 0;
        
        ret->Set(
            String::NewSymbol("target"),
            target
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

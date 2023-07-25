/************************************************************************ 
* @file: echo.cpp                                                    
* @author: dennis
* @revise: dennis <killme2008@gmail.com> http://www.blogjava.net/killme2008
*          相对完整的echo server，可以接受多个客户端连接，并且可以通过键入quit正常关闭

************************************************************************/

#ifdef _DEBUG
#pragma comment (lib,"aced.lib")
#else
#pragma comment (lib,"ace.lib")
#endif

#include "ace/Reactor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/OS.h"
#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"
#include "ace/Thread_Manager.h"
#include<iostream>
#include<string>

#define PORT_NO 8080
typedef ACE_SOCK_Acceptor Acceptor;
//forward declaration
class Echo_Handler;


class Echo_Handler:public ACE_Event_Handler
{
public:
    //construcor
    Echo_Handler()
    {
    }
    virtual ~Echo_Handler()
    {
    }
    //Called back to handle any input received
    int handle_input(ACE_HANDLE)
    {
        //receive the data
        ssize_t recvBytes = peer().recv(data,12);
        if(recvBytes <= 0)
        {
            ACE_DEBUG((LM_DEBUG,"%s\n","客户端断开连接"));
            return -1;
        }
        data[recvBytes] = 0;

        ACE_DEBUG((LM_DEBUG,"%s\n",data));


        if(ACE_OS::strcmp(data,"q") == 0)
        {
            ACE_DEBUG((LM_DEBUG,"%s\n","客户端退出"));
            peer().close(); // Gracefully Shutdown ??
            return -1;
        }
        peer().send_n(data,recvBytes);
        // do something with the input received.
        // 
        // keep yourself registerd with the reator
        return 0;
    }

    int handle_close(ACE_HANDLE h,ACE_Reactor_Mask m)
    {
        delete this;
        return  0;
    }

    //Used by the reactor to determine the underlying handle
    ACE_HANDLE get_handle()  const 
    {
        return this->peer_.get_handle();
    }

    //Returns a reference to the underlying stream.
    ACE_SOCK_Stream& peer()
    {
        return this->peer_;
    }

private:
    ACE_SOCK_Stream peer_;
    char data [12];
};



class Echo_Accept_Handler:public ACE_Event_Handler
{
public:
    //Constructor
    Echo_Accept_Handler(ACE_Addr &addr)
    {
        this->open(addr);
    }
    virtual ~Echo_Accept_Handler(){}

    int open(ACE_Addr &addr) // open peer_acceptor，使他监听
    {
        if(peer_acceptor.open(addr)==-1)
            ACE_ERROR_RETURN((LM_ERROR,"启动服务器错误\n"),1);
        return 0;
    }


    int handle_input(ACE_HANDLE handle) // Acceptor，夹带私货，也就是给对方发“按q提示”
    {
        //Client has requested connection to server.
        //Create a handler to handle the connection
        Echo_Handler *eh; // client想连接，创建个新Handler。
        ACE_NEW_RETURN(eh,Echo_Handler,-1);
        
        ACE_INET_Addr cliaddr;

        // 让这个新Handler去跟对方交涉，也就是accept到eh头上去。
        //Accept the connection "into" the Event Handler
        if(this->peer_acceptor.accept(eh->peer(),//stream
            &cliaddr,//remote address
            0,//timeout
            1) == -1)//restart if interrupted
            ACE_DEBUG((LM_ERROR,"Error in connection \n"));

        ACE_DEBUG((LM_DEBUG,"连接已经建立,来自%s\n",cliaddr.get_host_addr()));

        // 再给eh加个handler。等会儿上面的accept就给它读了。
        ACE_Reactor::instance()->register_handler(eh,ACE_Event_Handler::READ_MASK);

        const char* msg = "按q键使服务安全退出\r\n"; // 提示对方能够按键退出。
        eh->peer().send_n(msg,strlen(msg)+1);
        return 0;
    }

    //Used by the reactor to determine the underlying handle
    ACE_HANDLE get_handle(void) const
    {
        return this->peer_acceptor.get_handle();
    }
    int handle_close(ACE_HANDLE h,ACE_Reactor_Mask m){
        peer_acceptor.close();
        delete this;
        return 0;
    }

private:
    Acceptor peer_acceptor;
};




class Quit_Handler:public ACE_Event_Handler
{
public:
    Quit_Handler(ACE_Reactor* r):ACE_Event_Handler(r){}
    virtual int handle_exception(ACE_HANDLE)
    {
        ACE_DEBUG((LM_DEBUG,"停止服务器中\n"));
        reactor()->end_reactor_event_loop();
        return -1;
    }
    int handle_close(ACE_HANDLE h,ACE_Reactor_Mask m)
    {
        delete this;
        return 0;
    }
    virtual ~Quit_Handler(){}
};



static ACE_THR_FUNC_RETURN run_events (void *arg);
static ACE_THR_FUNC_RETURN controller (void *arg);


int ACE_TMAIN(int argc,char *argv[])
{

    ACE_Reactor* reactor=ACE_Reactor::instance();
    if(ACE_Thread_Manager::instance()->spawn(run_events,reactor,THR_DETACHED | THR_SCOPE_SYSTEM)==-1)
        return 1;
    if(ACE_Thread_Manager::instance()->spawn(controller,reactor,THR_DETACHED | THR_SCOPE_SYSTEM)==-1)
        return 1;
    return ACE_Thread_Manager::instance()->wait();
}



static ACE_THR_FUNC_RETURN run_events (void *arg)
{
    ACE_INET_Addr addr(PORT_NO); // INET Addr包括host, port，甚至协议族。

    Echo_Accept_Handler *eh= NULL; // 弄一个新eh。
    ACE_NEW_RETURN(eh,Echo_Accept_Handler(addr),(void*)1);

    ACE_Reactor::instance()->owner(ACE_OS::thr_self()); // 我也忘了在哪儿看的了，好像owner线程是要自己辅助指定的？

    ACE_Reactor::instance()->register_handler(eh,ACE_Event_Handler::ACCEPT_MASK);
    
    ACE_Reactor::instance()->run_reactor_event_loop();
    return 0;
}

static ACE_THR_FUNC_RETURN controller (void *arg) // 它弄了个eh，又用多线程+notify的法子，最后用reactor()->end_reactor_event_loop();结束一切。
                                                    // 我认为这是瞎扯淡，直接调用这个end_reactor_event_loop不就好了，套这么多皮干啥。
{
    Quit_Handler *quit_handler=0;
    ACE_NEW_RETURN(quit_handler,Quit_Handler(ACE_Reactor::instance()),(void*)1);
    for(;;)
    {
        std::string line;
        std::getline(std::cin,line,'\n');
        if(line=="quit"){
            ACE_DEBUG((LM_DEBUG,"请求停止服务器\n"));
            ACE_Reactor::instance()->notify(quit_handler); // notify可以指定一个event_handler，无脑给它派活儿。虽然这个eh并未register到reactor上，但依然可以。
            break;
        }
    }
    return 0; 
}
#include <stdio.h>
#include <ace/ACE.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/Asynch_Connector.h>
#include <ace/Asynch_IO.h>
#include <ace/Proactor.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>
#include <ace/Event_Handler.h>
#include <ace/Log_Priority.h>
#include <ace/Log_Msg.h>
#include <ace/Time_Value.h>
#include <ace/OS_NS_time.h>
#include "ace/Message_Queue.h"
#include "ace/OS.h"
#include <ace/Reactor.h>
#include <ace/Select_Reactor.h>
#include <ace/Message_Block.h>
#include <ace/OS_NS_time.h>
#include "ace/Message_Queue.h"
#include "ace/OS.h"


ACE_Time_Value time_out(2);

//练习一 建立最简单的通信客户端，没有采用reactor模式
/*int main(int argc,char* argv[])
{
	ACE::init();
	ACE_SOCK_Connector connector;
	ACE_INET_Addr addr(2452,"127.0.0.1");
	ACE_SOCK_Stream    stream;

	while(1)
	{
		if(-1 == connector.connect(stream,addr,&time_out))
		{
			ACE_DEBUG((LM_ERROR,"连接错误"));
			return -1;
		}
		char buf[2048];
		memset(buf,'\0',2048);
		ACE::read_n(ACE_STDIN,buf,2048);
		ACE_DEBUG((LM_DEBUG,"发送字符串是:%s\n",buf));
		stream.send_n(buf,sizeof(buf),&time_out);
		stream.recv_n(buf,2048,&time_out);
		ACE_DEBUG((LM_DEBUG,"接收的字符串是:%s\n",buf));
		stream.close();
	}
	ACE::fini();
	return 0;	
}*/ //end 练习一

//练习二 采用reactor模式建立的客户端
class CClientHandle:public ACE_Event_Handler
{
public:
	CClientHandle()
	{
		m_mb.reset();
	}
	~CClientHandle(){};

	virtual int handle_input(ACE_HANDLE fd)
	{
		ACE_UNUSED_ARG(fd);
		char buf[256];
		ssize_t recvSize;
		if((recvSize = m_stream.recv(buf,256,&time_out)) <=0 )
		{
			ACE_DEBUG((LM_DEBUG,"没有接收到数据，可能连接断开\n"));
			return -1;
		}
		ACE_DEBUG((LM_DEBUG,"接收到的数据是:%s\n",buf));

		m_mb.reset();
		strcpy(buf,"发送的数据！");
		memcpy(m_mb.wr_ptr(),buf,sizeof(buf));
		m_mb.wr_ptr(sizeof(buf));
		ACE_Reactor::instance()->schedule_wakeup(this,ACE_Event_Handler::WRITE_MASK);
		return 0;
	}
	virtual int handle_output(ACE_HANDLE fd )
	{
		if(m_mb.length()>0)
		{
			m_stream.send_n(&m_mb,&time_out);
		}
		ACE_Reactor::instance()->cancel_wakeup(this,ACE_Event_Handler::WRITE_MASK);
		return 0;
	}
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
	{
		m_stream.close();
		ACE_Reactor_Mask mask=0;
		int oldMaks=0;
		oldMaks = ACE_Reactor::instance()->mask_ops(this,mask,ACE_Reactor::GET_MASK);
		ACE_Reactor::instance()->remove_handler(this,mask|ACE_Event_Handler::DONT_CALL);
		return 0;
	}
	virtual ACE_HANDLE get_handle() const {return m_stream.get_handle();}
	ACE_SOCK_Stream& peer(){return m_stream;}
	void open(ACE_INET_Addr& addr)
	{
		if(-1 == m_connector.connect(m_stream,addr,&time_out))
		{
			ACE_DEBUG((LM_DEBUG,"建立连接失败\n"));
			return;
		}
		ACE::set_flags (m_stream.get_handle(), ACE_NONBLOCK);
		ACE_Reactor::instance()->register_handler(this,ACE_Event_Handler::READ_MASK|ACE_Event_Handler::WRITE_MASK);
	}
	void pushSendBuf()
	{
		m_mb.reset();
		m_mb.size(256);
		char buf[]="第一次发送数据\n" ;
		memcpy(m_mb.wr_ptr(),buf,sizeof(buf));
		m_mb.wr_ptr(sizeof(buf));
		ACE_Reactor::instance()->schedule_wakeup(this,ACE_Event_Handler::WRITE_MASK);
	}
private:
	ACE_SOCK_Stream m_stream;
	ACE_SOCK_Connector m_connector;
	ACE_Message_Block m_mb;
};

int main(int argc,char* argv[])
{
	ACE::init();
	ACE_Select_Reactor* seleRac=NULL;
	ACE_NEW_RETURN(seleRac,ACE_Select_Reactor(),-1);
	ACE_Reactor* reactor = 0;
	ACE_NEW_RETURN (reactor, ACE_Reactor (seleRac, true), -1);
	ACE_Reactor::instance(reactor);


	CClientHandle client;
	auto addr = ACE_INET_Addr(2452,"127.0.0.1");
	client.open(addr);
	client.pushSendBuf();
	ACE_Reactor::instance()->run_reactor_event_loop();
	ACE::fini();
	return 0;
}; //end 练习二

//练习三 采用proactor模式建立的客户端
/*class HA_Proactive_Service : public ACE_Service_Handler
{
public:
	~HA_Proactive_Service ()
	{
		if (this->handle () != ACE_INVALID_HANDLE)
			ACE_OS::closesocket (this->handle ());
	}

	virtual void open (ACE_HANDLE h, ACE_Message_Block& message_block)
	{
		this->handle (h);
		if (this->m_writer.open (*this) != 0 )
		{
			ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"),
				ACE_TEXT ("HA_Proactive_Service open")));
			delete this;
			return;
		}

		if (this->m_reader.open (*this) != 0 )
		{
			ACE_ERROR ((LM_ERROR, ACE_TEXT ("%p\n"),
				ACE_TEXT ("HA_Proactive_Service open")));
			delete this;
			return;
		}

		//伪造读结果，如果有接收到数据的话
		if(message_block.length()>0)
		{
			ACE_Message_Block* mb=message_block.duplicate();
			ACE_Asynch_Read_Stream_Result_Impl* result=NULL;
			result=ACE_Proactor::instance()->create_asynch_read_stream_result(this->proxy(),
				this->handle_,
				*mb,
				mb->length(),
				0);
			mb->wr_ptr(mb->wr_ptr()-message_block.length());
			result->complete(message_block.length(),1,0);
			delete result;
		}
		// 否则，通知底层，准备读取用户数据
		//创建一个消息块。这个消息块将用于从套接字中异步读 
		else
		{
			ACE_Message_Block *mb = 0;
			ACE_NEW_NORETURN (mb, ACE_Message_Block (1024));

			if (m_reader.read (*mb, mb->space () - 1) == -1)
			{
				delete mb;
				ACE_ERROR ((LM_ERROR, "%N:%l:open init read failed!"));
				return;
			}
		}
		//第一次发送一个数据
		ACE_Message_Block mb1(256);
		char buf[]="第一次发送的数据\n";
		memcpy(mb1.wr_ptr(),buf,sizeof(buf));
		mb1.wr_ptr(sizeof(buf));
		m_writer.write(mb1,mb1.length());
		return;
	}

	//异步写完成后会调用此函数
	//virtual void handle_write_stream
	//	(const ACE_Asynch_Write_Stream::Result &result)
	//{
	//	return;
	//}
	virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result &result)
	{
		result.message_block ().copy("");
		ACE_DEBUG ((LM_DEBUG, "********************\n"));
		ACE_DEBUG ((LM_DEBUG, "%s = %d\n", "bytes_to_read", result.bytes_to_read ()));
		ACE_DEBUG ((LM_DEBUG, "%s = %d\n", "handle", result.handle ()));
		ACE_DEBUG ((LM_DEBUG, "%s = %d\n", "bytes_transfered", result.bytes_transferred ()));
		ACE_DEBUG ((LM_DEBUG, "%s = %d\n", "act", (u_long) result.act ()));
		ACE_DEBUG ((LM_DEBUG, "%s = %d\n", "success", result.success ()));
		ACE_DEBUG ((LM_DEBUG, "%s = %d\n", "completion_key", (u_long) result.completion_key ()));
		ACE_DEBUG ((LM_DEBUG, "%s = %d\n", "error", result.error ()));
		ACE_DEBUG ((LM_DEBUG, "接收到的数据是：%s\n",result.message_block().rd_ptr()));
		ACE_DEBUG ((LM_DEBUG, "********************\n"));

		result.message_block().release();

		if (this->initiate_read_stream (result) == -1)
		{
			ACE_ERROR((LM_ERROR, "%N:%l:read stream failed!connection closed, remove it:%d\n", result.handle()));
			return;
		} 
		//发送一个回应
		ACE_Message_Block mb(256);
		char buf[]="客户端发送的数据\n";
		memcpy(mb.wr_ptr(),buf,sizeof(buf));
		mb.wr_ptr(sizeof(buf));
		m_writer.write(mb,mb.length());
	}

	int initiate_read_stream (const ACE_Asynch_Read_Stream::Result &result)
	{
		ACE_DEBUG((LM_TRACE, "%N:%l:TPTCPAsynchServerImpl::initiate_read_stream() "));
		//创建一个消息块。这个消息块将用于从套接字中异步读 
		ACE_Message_Block *mb = new ACE_Message_Block(1024);
		if (mb == NULL)
		{
			ACE_DEBUG((LM_ERROR, "%N:%l:can't allock ACE_Message_Block.  ")); 
			return -1;
		}
		if (m_reader.read (*mb, mb->space() - 1) == -1)
		{
			delete mb;
			ACE_ERROR_RETURN ((LM_ERROR, "%N:%l:rs->read() failed, clientID=%d ", result.handle()),  -1);
		}
		return 0;
	}
private:
	ACE_Asynch_Write_Stream m_writer;
	ACE_Asynch_Read_Stream  m_reader;
};

int main(int argc, char *argv[]) 
{
	ACE_INET_Addr addr(2452,"127.0.0.1"); 
	ACE_Asynch_Connector<HA_Proactive_Service> connector;
	connector.open();
	if (connector.connect(addr) == -1)
		return -1;

	ACE_Proactor::instance ()->proactor_run_event_loop();
	return 0; 
} 
*///end 练习 三

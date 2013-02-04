#include <stdio.h>
#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif 

#include <simpleserver.h>
#include <tsleep.h>
#include <telnetserver.h>
#include <telnetserversocket.h>
#include <linkedlist.h>
#include "timed.h"

#define CMD_PROMPT "falgor$ "

using namespace ssobjects;

class User
{
    protected: 
        TelnetServerSocket* m_psocket;
        Timed   m_timer;
        bool    m_bCameraUpdates;

    public:
        User(TelnetServerSocket* psock)
            : m_psocket(psock),m_timer(0),m_bCameraUpdates(false)
        {
        }

        TelnetServerSocket* socket() {return m_psocket;}

        bool update()
        {
            if(wantsCameraUpdates() && m_timer.update())
                return true;
            return false;
        }

        void setCameraUpdates(bool updates,unsigned long freq=0)
        {
            m_bCameraUpdates = updates;
            m_timer.setDelay(freq);
        }

        bool wantsCameraUpdates() {return m_bCameraUpdates;}
        
};

class ArthurServer : public TelnetServer
{
    protected:
        unsigned32 m_updates;
        LinkedList<User> m_userList;
        Timed m_timer;  //just used for debugging so we can see the server is active
        StopWatch m_stopwatch;

    public:
		enum {FREQUENCY = 33};    //in milliseconds - 33 is about 30 times per second 20 is about the maximum
		ArthurServer(SockAddr& saBind) : TelnetServer(saBind,FREQUENCY),m_timer(1000),m_stopwatch() 
        {
            m_stopwatch.start();
        }

    protected:
		void processSingleMsg(PacketMessage* pmsg)
        {
            TelnetServerSocket* psocket = (TelnetServerSocket*)pmsg->socket();
            PacketBuffer* ppacket = pmsg->packet();
            switch(ppacket->getCmd())
            {
                //One way to handle the message. Process and reply within the switch.
                case PacketBuffer::pcNewConnection:   onConnection(psocket);    break;
                case PacketBuffer::pcClosed:          onClosed(psocket);        break;
                case TelnetServerSocket::pcFullLine:  onFullLine(pmsg);         break;
            }
            DELETE_NULL(ppacket);   //IMPORTANT! The packet is no longer needed. You must delete it.
        }

        void syntaxError(TelnetServerSocket* psocket)
        {
            psocket->println("Sytax error");
        }

        //By the time onClosed is called, the socket is already closed.
        //Don't try to send anything to the socket. Just dispose of any
        //resources associated with it. 
        void onClosed(TelnetServerSocket* psocket)
        {
            printf("socket closed\n");
            User* puser = findUser(psocket); 
            m_userList.removeCurrent();
            assert(puser);
            delete puser;
        }

        void onFullLine(PacketMessage* pmsg)
        {
            TelnetServerSocket* psocket = (TelnetServerSocket*)pmsg->socket();
            char* pstring = (char*)pmsg->packet()->getBuffer();
            if(strlen(pstring) < 4)
                syntaxError(psocket);
            else
            {
                char cmd[5];
                strncpy(cmd,pstring,4);
                cmd[4] = NULL;  //null terminate
                printf("Got from client: cmd [%s] line [%s]\n",cmd,pstring);
                processCommand(psocket,cmd,pstring);
            }
        }

        void onConnection(TelnetServerSocket* psocket)
        {
    		printf("got connection\n");
        	psocket->println(CMD_PROMPT);
            User* puser = new User(psocket);
            m_userList.addTail(puser);
        }

        void idle(unsigned32 now)
        {
//            if(m_timer.update())
//                printf("%06lu tick\n",m_stopwatch.milliseconds());
            sendCameraUpdates((unsigned long)now);
        }

        void processCommand(TelnetServerSocket* psocket,const char* pcmd,char* pstring)
        {
            char* pparam = pstring+strlen(pcmd);
            User* puser = findUser(psocket);
            if(!strcasecmp(pcmd,"STOP"))
            {
                puser->setCameraUpdates(false);
                psocket->print("All updates stopped.\n");
            }
            else if(!strcasecmp(pcmd,"CAMC"))
            {
                int freq = atoi(pparam);
                puser->setCameraUpdates(true,freq);
                psocket->println("Camera updates now active");

            }
        }

        // Find the socket in the user list. Return the User pointer or NULL if not found. 
        User* findUser(TelnetServerSocket* psocket)
        {
            User* puser = m_userList.getHead();
            while(puser)
            {
                if(puser->socket() == psocket)
                    return puser;
                puser = m_userList.getNext();
            }
            return NULL;
        }

        //Send the camera information to whoever wants it
        void sendCameraUpdates(unsigned long now)
        {
            char buff[1024];
            const char* msg = "CAMC angle1,angle2,distance,x,y,orientation";
            snprintf(buff,sizeof buff,"%s,%lu\n",msg,now);  //check your docs, to make sure this automatically is null terminated

            User* puser = m_userList.getHead();
            while(puser)
            {
                if(puser->update())
                    puser->socket()->print(buff);
                puser = m_userList.getNext();
            }
            m_updates++;
        }
};

int main(int argc, char const *argv[])
{
	try 
	{
		unsigned16 wPort = 1180;

		SockAddr saBind((ULONG)INADDR_ANY,wPort);
		ArthurServer server(saBind);          
    	if(!SimpleServer::canBind(saBind))  // check if we can bind to this port
      		printf("Can't bind\n");       // should not throw from main after server constructed
        else
        {
            printf("Server on port %d\n",wPort);
            printf("You can run 'telnet localhost %u' from another termainal.\n",wPort);
            server.startServer();               // server will now listen for connections
            printf("server is finished.\n");
	    }
	} 
	catch (GeneralException& e)
	{
		printf("Got an error: %s", e.getErrorMsg());
	}
}

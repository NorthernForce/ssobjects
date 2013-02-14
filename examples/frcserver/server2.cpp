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

#include "/home/pi/OpenCV-2.4.3/modules/highgui/include/opencv2/highgui/highgui.hpp"
#include <iostream>
#include <vector>
#include <stdio.h>

#define CMD_PROMPT "falgor$ "

using namespace ssobjects;
using namespace cv;
using namespace std;

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

  bool editcurrent = false; //is true just after the user selects a pixel. when true, the program will get the rgb values of the selected pixel
  
  int thresh = 30; 
  int catnumx, catnumy; //x and y of latest pixel selected by the user
  int currentpix = 0; //number of selected pixels
  int bee[99], jee[99], are[99]; //rgb values of selected pixels (at the time of selection)

  bool findcenter = true;
  int runningx, runningy;
  float avrgx, avrgy; //for averaging; running total of x values, y values, and the calculated averages

  void my_mouse_callback( int event, int x, int y, int flags, void* param );

  VideoCapture capturen; 
  Mat frame; //mat of the original image
  Mat newframe; //mat of the threshholded (threshheld?) image
  

    public:
		enum {FREQUENCY = 33};    //in milliseconds - 33 is about 30 times per second 20 is about the maximum
		ArthurServer(SockAddr& saBind) : TelnetServer(saBind,FREQUENCY),m_timer(1000),m_stopwatch() 
        {
            m_stopwatch.start();
        }

    protected:

   void targetinginit ()
  {
    cout << "Left click to collect samples, right click to clear them.\n";
    cout << "'c' to toggle center finding.\n";
    cout << "Press q to quit, but only if you really mean it.\n";

    std::string arg = "0";
    VideoCapture capture(arg); 
    if (!capture.isOpened())
        capture.open(atoi(arg.c_str()));
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 240); //this line and the following line is necessary only for the raspberry pi. if you were to delete these on the pi, you would get timeout errors
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 320);
    if (!capture.isOpened()) {
      cerr << "Failed to open a video device or video file!\n" << endl;
    }
    capturen = capture;

    namedWindow("Original Image", CV_WINDOW_KEEPRATIO);
    namedWindow("Threshold Image", CV_WINDOW_KEEPRATIO);
    
       
    cvCreateTrackbar( "Threshold", "Threshold Image", &thresh, 100, NULL );
  }



int targetingprocess (float &azimuth, float &elevation, int &npix)
  {

    npix = 10;
            
      capturen >> newframe;

	    	    
	  if (editcurrent) //reads the rgb of the newly selected pixel
	      {
		int z = currentpix-1;
		bee[z] = newframe.at<cv::Vec3b>(catnumx,catnumy)[0];
		jee[z] = newframe.at<cv::Vec3b>(catnumx,catnumy)[1];
		are[z] = newframe.at<cv::Vec3b>(catnumx,catnumy)[2];
		editcurrent = false;
	      }
	      

	    int found = 0; //number of matches found (used for averaging)
	    runningx = 0;
	    runningy = 0;
	    for(int i = 0; i < frame.rows; i++)
	    {
	      for(int j = 0; j < frame.cols; j++)
		{
		  int thisbee = newframe.at<cv::Vec3b>(i,j)[0];
		  int thisjee = newframe.at<cv::Vec3b>(i,j)[1];
		  int thisare = newframe.at<cv::Vec3b>(i,j)[2];
		  int matches = 0;
		  for(int h = 0; h <= currentpix; h++)
		    {

		      if(catnumx+catnumy != 0) if ((thisbee<bee[h]+thresh && thisbee>bee[h]-thresh) && (thisjee<jee[h]+thresh && thisjee>jee[h]-thresh) && (thisare<are[h]+thresh && thisare>are[h]-thresh)) //if it's a match
			{
			  newframe.at<cv::Vec3b>(i,j)[0] = 255; //set to white
			  newframe.at<cv::Vec3b>(i,j)[1] = 255;
			  newframe.at<cv::Vec3b>(i,j)[2] = 255;
			  if (findcenter && matches==0) {runningx += i; runningy += j; found++;}
			  matches++;
			}
		     
		    }
		  if (matches == 0)
			{
			  newframe.at<cv::Vec3b>(i,j)[0] = 0;
			  newframe.at<cv::Vec3b>(i,j)[1] = 0;
			  newframe.at<cv::Vec3b>(i,j)[2] = 0;
			}
		}
		  
	    
	    }
	    if (findcenter)
	      {  
		if (found>0){avrgx = runningx/found; //average the x
		  avrgy = runningy/found;} //average the y
		runningx = 0;
		runningy = 0;
		Point thecenter = Point(avrgy, avrgx);
		ellipse(newframe, thecenter, Size(20,20), 0, 0, 360, Scalar(255,0,238), 2,8); //draw circle
	      }
	    imshow("Threshold Image", newframe);
	    capturen >> frame;
	    if (findcenter) { Point thecenter = Point(avrgy, avrgx); ellipse(frame, thecenter, Size(20,20), 0, 0, 360, Scalar(255,0,238), 2,8);}

            imshow("Original Image", frame);
	    IplImage ipl = frame;
	    cvSetMouseCallback( "Original Image", my_mouse_callback, (void*) &ipl); //get mouse input
	   
            char key = (char)waitKey(5); //get keyboard input
	    if (key == 'q') break;
	    if (key == 'c') {findcenter = !findcenter; runningx = 0; runningy = 0; avrgx = 0; avrgy = 0;}
	    azimuth = avrgx;
	    elevation = avrgy;
        return 0;
  }
	

  void my_mouse_callback( int event, int x, int y, int flags, void* param ){
	IplImage* image = (IplImage*) param;

	switch( event ){
	
		case CV_EVENT_LBUTTONDOWN:
		  catnumx = y;
		  catnumy = x;
		  currentpix++;
		  editcurrent = true;
		  break;

	        case CV_EVENT_RBUTTONDOWN:
		  for (int g = 0; g <= currentpix; g++)
		    {
		      catnumx = 0;
		      catnumy = 0;
		    }
		  currentpix = 0;
		  break;
	}
  }

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
            //const char* msg = "CAMC angle1,angle2,distance,x,y,orientation";
            const char* msg = "CAMC 12.3, 14.2, 20.1, 4, 5, 30.1";
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

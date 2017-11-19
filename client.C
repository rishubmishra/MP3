/* 
    File: simpleclient.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/11

    Simple client main program for MP2 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <vector>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <array>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <thread>

#include "reqchannel.H"
#include "bounded_buffer.H"
#include "semaphore.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/



string int2string(int number) {
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

//time functions
void print_time_diff(struct timeval * tp1, struct timeval * tp2) {
  /* Prints to stdout the difference, in seconds and museconds, between
     two timevals. 
  */

  long sec = tp2->tv_sec - tp1->tv_sec;
  long musec = tp2->tv_usec - tp1->tv_usec;
  if (musec < 0) {
    musec += 1000000;
    sec--;
  }
  printf(" [sec = %ld, musec = %ld] ", sec, musec);

}

long get_time_diff(struct timeval * tp1, struct timeval * tp2) {
  /* returns the time diff rather than print it
  */

  long sec = tp2->tv_sec - tp1->tv_sec;
  long musec = tp2->tv_usec - tp1->tv_usec;
  if (musec < 0) {
    musec += 1000000;
    sec--;
  }
  return musec;

}

//histograms
vector<int> *joeHist;
vector<int> *janeHist;
vector<int> *johnHist;



struct Request{
	string name; //who the request is for
	Bounded_buffer* response_buff; //where the req goes
	
	Request(string name, Bounded_buffer* response_buff ) : 
	name(name), response_buff(response_buff){}
};

 
//Thread parameters
struct ReqParams{
  string name; 
  int numofReqs; //num of data requests 
  Bounded_buffer* reqbuff;

  ReqParams(string name, int numofReqs, Bounded_buffer* reqbuff) : 
	name(name), numofReqs(numofReqs), reqbuff(reqbuff){}

};

struct StatParams{
  string name;
  int numofReqs;
  vector<int> *hist;
  Bounded_buffer* statbuff;
	

  StatParams(string name, int numofReqs, vector<int> *hist, Bounded_buffer* statbuff):
    name(name), numofReqs(numofReqs), hist(hist), statbuff(statbuff) {}
};

struct WorkParams{
  RequestChannel* chan;
  Bounded_buffer* buff;
  vector<Bounded_buffer*>* responses;
  WorkParams(RequestChannel* chan, Bounded_buffer* buff, vector<Bounded_buffer*>* responses):
    chan(chan), buff(buff), responses(responses) {}
};

//Thread Functions
void* reqFunc(void* arg){
  cout << "Request Function" << endl;
  /* ReqParams* args = (ReqParams*) arg;
  int numofReqs = args->numofReqs;
  ofstream file(args->file);
  int name = nameToid(args->name);

  int sentCount = 0;
  struct timeval start,end;

  //for each person send the # of alotted data requests
  gettimeofday(&start,0);
  while(sentCount < numofReqs){
    dataBuff[name].push("data");
    sentCount++;
  }
  gettimeofday(&end,0);

  long timeDiff = get_time_diff(&start,&end);
  file << name << ": " << timeDiff << endl;

  dataBuff[name].push("quit");
  pthread_exit(NULL); */
  
  ReqParams* req = (ReqParams*) arg;
  int count = req->numofReqs;
  string data = "data " + req->name;
  for (int i = 0; i < count; i++){
	req->reqbuff->push(data);
  }

} 

void* statFunc(void* arg){
  cout << "Stat function" << endl;
  /* StatParams* args = (StatParams*) arg;
  int name = nameToid(args->name);
  int numofReqs = args->numofReqs;
  ofstream file(args->file);
  int received = 0;
  time_t timer;

  //for every data request received we must output the data stats
  while(received < numofReqs){
    string data = statBuff[name].pop();
    timer = time(NULL);
    file << data << " at time: " << timer << endl;
    received++;
  }

  pthread_exit(NULL); */
  
  StatParams* args = (StatParams*) arg;
  int received = 0;
  while(received < args->numofReqs){
	  string str = args->statbuff->pop();
	  int data = stoi(str);
	  args->hist->push_back(data);
  }

}

void* workFunc(void* arg){
  cout << "Worker Function" << endl;
  //grabs data requests and forwards them to data server
  WorkParams* worker = (WorkParams*) arg;
  Bounded_buffer* buff = worker->buff;
  RequestChannel* chan = worker->chan;
  vector<Bounded_buffer*>* responses = worker->responses;
  //while there are any 
 /*  for (int i = 0; i < 3; i++){
    if (isAlive()){
      string dataReq = dataBuff[i].pop();
      string reply = chan->send_request(dataReq);

      if (reply == "bye"){
        livingChannels[i] = false;
      }
      else{statBuff[i].push(reply);}
    }

  } */
  
  while(true){
	  string str = buff->pop();
	  
	  if (str == "quit"){
		  chan->send_request("quit");
		  break;
	  }
	  
	  string data = "data " + str;
	  string reply = chan->send_request(data);
	  
	  if(data == "data Joe Smith"){
		responses->at(0)->push(reply);
	  }
	  else if(data == "data Jane Smith"){
		responses->at(1)->push(reply);
	  }
	  else if (data == "data John Doe"){
		  responses->at(2)->push(reply);
	  }
	  else{
		  break;
	  }  
  }
  chan->send_request("quit");

}

string makeHistogram(string name, vector<int> *data){
	string histogram = "Freq count for " 	+	name + ":\n";
	
	for (int i = 0; i < data->size(); i++){
		histogram	+= to_string(i) + ": " + to_string(data->at(i)) + "\n";
	}
	
	return histogram;
	
}




/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
  int n = 10; // # of requests per person
  int b = 50; // size of request buffer
  int w = 10; // # of worker threads

  //set each thread to alive(true)
  char optChar;
  while ((optChar = getopt(argc, argv, "n:b:w:")) != -1){
    switch(optChar){
      case 'n':
        n = atoi(optarg);
        break;
      case 'b':
        b = atoi(optarg);
        break;
      case 'w':
        w = atoi(optarg);
        break;
      default:
        cout << "incorrect input, the proper flags are:" << endl;
        cout << "-n <number of data requests per person" << endl;
        cout << "-b <size of bounded buffer between requests and worker threads>" << endl;
        cout << "-w <number of worker threads>" << endl;
        cout << "Ex: ./client -n 10 -b 50 -w 10" << endl;
        break;
    }
  }

  int pid = fork();
  if (pid == 0){
	struct timeval start,end;
	ofstream ofs;
	ofs.open("output.txt",ofstream::out | ofstream::app);
	cout << "Num of data request per person: " << n << endl;
	cout << "Size of bounded buffer between requests and worker threads: " << b << endl;
	cout << "Num of worker threads: " << w << endl; 
	
	cout << "CLIENT STARTED:" << endl;
	cout << "Establishing control channel... " << flush;
	RequestChannel* chan = new RequestChannel("control",RequestChannel::CLIENT_SIDE);
	cout << "done." << endl;
	cout << "-----------------------------------------------------------------------" << endl;
		
	
	
	gettimeofday(&start, NULL);

	ReqParams* rt_args[3];
	WorkParams* wt_args[w];
	StatParams* st_args[3];
	
	pthread_t worker[w];
	pthread_t stat[3];
	pthread_t request[3];
	
	vector<Bounded_buffer*> reqBuff;
	for (int i = 0; i < 3; i++){
		Bounded_buffer* bb = new Bounded_buffer(b);
		reqBuff.push_back(bb);
	}
	
	//request thread parameters
	ReqParams* ReqParamsJoe = new ReqParams("Joe Smith", n, reqBuff[0]);
	rt_args[0] = ReqParamsJoe;
	ReqParams* ReqParamsJane = new ReqParams("Jane Smith", n, reqBuff[1]);
	rt_args[1] = ReqParamsJane;
	ReqParams* ReqParamsJohn = new ReqParams("John Doe", n, reqBuff[2]);
	rt_args[2] = ReqParamsJohn;
	
	//statistic thread parameters
	StatParams* StatParamsJoe = new StatParams("Joe Smith", n, joeHist, reqBuff[0] );
	st_args[0] = StatParamsJoe;
	StatParams* StatParamsJane = new StatParams("Jane Smith" , n, janeHist, reqBuff[1]);
	st_args[1] = StatParamsJane;
	StatParams* StatParamsJohn = new StatParams("John Doe", n , johnHist, reqBuff[2]);
	st_args[2] = StatParamsJohn;
	
	
	
	
	//create request threads
	//cout << "creating request threads" << endl;
	for (int i = 0; i < 3; i++){
		pthread_create(&request[i], NULL, &reqFunc,(void *)&rt_args[i]);
	}
	
	//create worker threads
	//cout << "creating worker threads" << endl;
	vector<RequestChannel*> vec_WTchans;
	
	vector<Bounded_buffer*>* responses;
	Bounded_buffer* joeResponseBuff = new Bounded_buffer(b);
	responses->push_back(joeResponseBuff);
	Bounded_buffer* janeResponseBuff = new Bounded_buffer(b);
	responses->push_back(janeResponseBuff);
	Bounded_buffer* johnResponseBuff = new Bounded_buffer(b);
	responses->push_back(johnResponseBuff);
	
	for (int i = 0; i < w; i++){
		string newThread = chan->send_request("newthread");
		RequestChannel *wt_chan = new RequestChannel(newThread, RequestChannel::CLIENT_SIDE);
		vec_WTchans.push_back(wt_chan);
		wt_args[i] = new WorkParams(wt_chan, reqBuff[i], responses);
		pthread_create(&worker[i],NULL,&workFunc,(void *)&wt_args[i]);	
		
	}
	
	
	//creating stat threads
	cout << "creating stat threads" << endl;
	for (int i = 0; i < 3; i++){
		pthread_create(&stat[i],NULL,&statFunc,(void *)&st_args[i]);
	}
	
	cout << "threads created" << endl;
	cout << "-----------------------------------------------------------------------" << endl;
	
	/* //join request threads
	for(int i = 0; i < 3; i++){
		pthread_join(request[i],NULL);
	}
	
	
	//quit worker threads
	for(int i = 0; i < w; i++){
		vec_WTchans.at(i)->send_request("quit");
	}
	
	//join worker threads
	for (int i = 0; i < w; i++){
		pthread_join(worker[i],NULL);
	}
	
	//join stat threads
	for(int i = 0; i < 3; i++){
		pthread_join(stat[i],NULL);
	}

	gettimeofday(&end,NULL); */
	
	

	cout << "Joe's Histogram:" << endl << makeHistogram("Joe Smith", joeHist) << endl;
	cout << "John's Histogram:"	<< endl << makeHistogram("John Doe" ,johnHist) << endl;
	cout << "Jane's Histogram:"	<< endl << makeHistogram("Jane Smith" , janeHist) << endl;
	
	cout << endl << "Time to finish:" << to_string(get_time_diff(&start,&end)) << endl;
	
	
	
	ofs.close();
	usleep(1000000);
	string quit = chan->send_request("quit");
	cout << "Bye:" << quit << endl;
	
  }
  else if (pid != 0){
	execl("dataserver",NULL);
  }
 
}

/*   //push the buffers onto the vectors, one of each for each person
  for (int i = 0; i < 3; i++){
    dataBuff.push_back(Bounded_buffer(b));
    statBuff.push_back(Bounded_buffer(b));
  }

  cout << "buffers pushed onto vectors" << endl;

  ReqParams* rt_args[3];
  WorkParams* wt_args[w];


  cout << "setting request thread parameters" << endl;
  //request thread parameters
  ReqParams* ReqParamsJoe = new ReqParams("Joe Smith", n, "JoeSmith.txt");
  ReqParams* ReqParamsJane = new ReqParams("Jane Smith", n , "JaneSmith.txt");
  ReqParams* ReqParamsJohn = new ReqParams("John Doe", n, "JohnDoe.txt");
  cout << "request thread parameters set" << endl;

  cout << "setting statistic thread parameters" << endl;
  //statistic thread parameters
  StatParams* StatParamsJoe = new StatParams("Joe Smith", n, "JoeSmith.txt");
  StatParams* StatParamsJane = new StatParams("Jane Smith" , n, "JaneSmith.txt");
  StatParams* StatParamsJohn = new StatParams("John Smith", n , "JohnSmith.txt");
  cout << "statistic thread parameters set" << endl;

  



  vector<RequestChannel*> vec_WTchans;
  cout << "CLIENT STARTED:" << endl;
  cout << "Establishing control channel... " << flush;
  RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
  cout << "done." << endl;

  RequestChannel* wt_chan;
  for (int i = 0; i < w; i++){
	string newThread = chan.send_request("newthread");
    wt_chan = new RequestChannel(newThread, RequestChannel::CLIENT_SIDE);
    vec_WTchans.push_back(wt_chan);
    wt_args[i] = new WorkParams(wt_chan);
  }

  cout << "creating threads" << endl;
  pthread_t worker;

  pthread_t JoeStats;
  pthread_t JaneStats;
  pthread_t JohnStats;

  pthread_t JoeReqs;
  pthread_t JaneReqs;
  pthread_t JohnReqs;
  cout << "threads created" << endl;

  for (int i = 0; i < w; i++){
   pthread_create(&worker, NULL,workFunc, (void*)&wt_args[i]);
  }
  
  


  gettimeofday(&start, 0);

  pthread_create(&JoeStats, NULL, statFunc,(void*)StatParamsJoe);
  pthread_create(&JaneStats, NULL, statFunc,(void*)StatParamsJane);
  pthread_create(&JohnStats, NULL, statFunc,(void*)StatParamsJohn);

  pthread_create(&JoeReqs, NULL, reqFunc, (void*)ReqParamsJoe);
  pthread_create(&JaneReqs, NULL, reqFunc, (void*)ReqParamsJane);
  pthread_create(&JohnReqs, NULL, reqFunc, (void*)ReqParamsJohn);

  pthread_join(JoeStats,NULL);
  pthread_join(JaneStats,NULL);
  pthread_join(JohnStats,NULL);

  gettimeofday(&end,0);

  long completion = get_time_diff(&start,&end);

  } */


  

  

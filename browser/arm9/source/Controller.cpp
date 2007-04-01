#include "Wifi9.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "Controller.h"
#include "Document.h"
#include "View.h"
#include "URI.h"
#include "Client.h"

using namespace std;

Controller::Controller(string & uriString)
  : m_document(*(new Document(uriString))),
  m_view(*(new View(m_document, *this)))
{
  // split the URI into sections
  URI uri(uriString);
  if (uri.isFile()) {
    localFile(uri.fileName());
  } else {
    // http
    fetchHttp(uri);
  }
}

void Controller::mainLoop()
{
  m_view.mainLoop();
}

void Controller::keyPress()
{
}

void Controller::localFile(const string & fileName)
{
  ifstream uriFile;
  uriFile.open(fileName.c_str(), ios::in);
  // read the lot
  if (uriFile.is_open())
  {
    uriFile.seekg(0, ios::end);
    int size = uriFile.tellg();
    char * data = new char[size+2];
    uriFile.seekg(0, ios::beg);
    uriFile.read(data, size);
    data[size] = 0;
    m_document.setData(data, size);
    delete [] data;
  }
  uriFile.close();
}


// An implementation - prints out the bytes
class HttpClient: public nds::Client
{
  public:
    HttpClient(const char * ip, int port)
      : nds::Client(ip,port), m_total(0)
    {}

    // implement the pure virtual functions
    void handle(void * bufferIn, int amountRead)
    {
      char * buffer = (char*)bufferIn;
      buffer[amountRead] = 0;
      m_data.append(buffer);
      // write buffer to stdout
      // printf("%s",buffer);
    }
    
    bool finished() 
    {
      return false;
    }

    void finish() { }

    void debug(const char * s)
    {
       //printf("\ndebug:%s\n",s);
       cout << "debug:"<< s << endl;
    }

    // GET stuff
    void get(const URI & uri)
    {
      if (isConnected())
      {
        string s("GET ");
        s += uri.fileName();
        s += " HTTP/1.1\n";
        s += "Host:" + uri.server()+"\n";
        s += "Connection: close\n";
        s += "Accept-charset: ISO-8859-1,UTF-8\n";
        s += "Accept: text/html\n";
        s += "User-Agent: Homebrew Browser\n";
        s += "\n";
        write(s.c_str(), s.length());
      }
      m_data = "";
    }
    const string & data() const
    {
      return m_data;
    }
  private:
    int m_total;
    string m_data;


};

void Controller::fetchHttp(const URI & uri)
{
  nds::Wifi9::instance().connect();
  if (nds::Wifi9::instance().connected()) {
    // open a socket to the server.
    HttpClient client(uri.server().c_str(), 80);
    client.connect();
    client.get(uri);
    client.read();
    m_document.setData(client.data().c_str(), client.data().size());
  } else {
    char * woops = "Woops, wifi not done";
    m_document.setData(woops, strlen(woops));
  }
}


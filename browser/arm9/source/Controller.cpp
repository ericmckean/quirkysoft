#include "Wifi9.h"
#include <vector>
#include "Controller.h"
#include "Document.h"
#include "View.h"
#include "URI.h"
#include "Client.h"
#include "File.h"

using namespace std;

Controller::Controller()
  : m_document(*(new Document())),
  m_view(*(new View(m_document, *this)))
{
}

void Controller::doUri(const std::string & uriString)
{
  if (uriString.size()) {
    m_document.setUri(uriString);
    // split the URI into sections
    URI uri(uriString);
    if (uri.isFile()) {
      localFile(uri.fileName());
    } else {
      // http
      fetchHttp(uri);
    }
  }
}

void Controller::mainLoop()
{
  m_view.mainLoop();
}

void Controller::localFile(const string & fileName)
{
  nds::File uriFile;
  uriFile.open(fileName.c_str());
  // read the lot
  if (uriFile.is_open())
  {
    int size = uriFile.size();
    char * data = new char[size+2];
    uriFile.read(data);
    data[size] = 0;
    m_document.appendLocalData(data, size);
    m_document.setStatus(Document::LOADED);
    delete [] data;
  }
  uriFile.close();
}


// An implementation - prints out the bytes
class HttpClient: public nds::Client
{
  public:
    HttpClient(const char * ip, int port)
      : nds::Client(ip,port), m_total(0), m_finished(false)
    {}

    void setDocument(Document & d)
    {
      m_document = &d;
    }

    // implement the pure virtual functions
    void handle(void * bufferIn, int amountRead)
    {
      char * buffer = (char*)bufferIn;
      buffer[amountRead] = 0;
      m_document->appendData(buffer, amountRead);
    }
    
    void connectCallback() {
      printf("Connecting...\n");
    }
    void writeCallback() {
      printf("write...\n");
    }
    void readCallback() {
      m_document->setLoading(1);
    }

    bool finished() 
    {
      return m_finished;
    }

    void finish() { 
      m_document->setStatus(Document::LOADED);
    }

    void debug(const char * s)
    {
      //printf("\ndebug:%s\n",s);
      //m_document->setData(s, strlen(s));
      //cout << "debug:"<< s << endl;
    }

    // GET stuff
    void get(const URI & uri)
    {
      if (isConnected())
      {
        string s("GET ");
        s += uri.fileName();
        s += " HTTP/1.1\r\n";
        s += "Host:" + uri.server()+"\r\n";
        s += "Connection: close\r\n";
        s += "Accept-charset: ISO-8859-1,UTF-8\r\n";
        s += "Accept: text/html\r\n";
        s += "User-Agent: Homebrew Browser\r\n";
        s += "\r\n";
        write(s.c_str(), s.length());
        m_finished = false;
        m_uri = uri;
      }
      // reset the document for downloading
      m_document->reset();
    }
  private:
    int m_total;
    bool m_finished;
    Document * m_document;
    URI m_uri;
};

void Controller::fetchHttp(const URI & uri)
{
  nds::Wifi9::instance().connect();
  if (nds::Wifi9::instance().connected()) {
    // open a socket to the server.
    // FIXME - hardcoded 80 port.
    HttpClient client(uri.server().c_str(), 80);
    client.setDocument(m_document);
    client.connect();
    client.get(uri);
    client.read();

    URI docUri(m_document.uri());
    if (docUri != uri)
    {
      // redirected
      doUri(m_document.uri());
    }
  } else {
    char * woops = "Woops, wifi not done";
    m_document.appendData(woops, strlen(woops));
  }
}


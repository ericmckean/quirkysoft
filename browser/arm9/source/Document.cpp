#include "Document.h"
#include <wchar.h>

using namespace std;

Document::Document():
  m_data(0)
{
}

void Document::setUri(const std::string & uriString)
{
  m_uri = uriString;
}

const std::string & Document::uri() const
{
  return m_uri;
}

const char * Document::asText() const
{
  return m_data;
}

void Document::registerView(ViewI * view)
{
   vector<ViewI*>::iterator it = find(m_views.begin(), m_views.end(), view);
   if (it == m_views.end())
     m_views.push_back(view);
}

void Document::unregisterView(ViewI * view)
{
   vector<ViewI*>::iterator it = find(m_views.begin(), m_views.end(), view);
   if (it != m_views.end())
     m_views.erase(it);
}


void Document::setData(const char * data, int size)
{
  if (m_data) {
    delete [] m_data;
    m_data = 0;
  }

  if (size) {
    m_data = new char[size];
    bcopy(data, m_data, size);
  } else {
    const string s("Unable to connect");
    m_data = new char[s.length()+1];
    bcopy(s.c_str(), m_data, s.length());
  }
  for_each(m_views.begin(), m_views.end(), mem_fun(&ViewI::notify));
}

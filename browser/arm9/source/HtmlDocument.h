#ifndef HtmlDocument_h_seen
#define HtmlDocument_h_seen

#include "UnicodeString.h"
#include "HtmlParser.h"
#include <stack>
#include <list>

class HeaderParser;
class HtmlElement;

/** Parse the HTML tokens after the tokenisation phase.
 */
class HtmlDocument : public HtmlParser
{

  public:

    //! Constructor.
    HtmlDocument();
    /** Get the data contents.
     * @return reference to the data.
     */
    inline const UnicodeString & data() const;

    /** Get the current amount of data retrieved.
     * @return amount of data retrieved.
     */
    inline unsigned int dataGot() const;
    /** Set the amount of data. Used for chunked content that does not set the content-length header.
     * @param value the new amount of data retrieved (0?)
     */
    inline void setDataGot(unsigned int value);

    /** Reset the internal state of the parser. */
    void reset();

    /** Set the header parser object. This is required for changes to the content model or for 
     * parsing meta http-equiv tags.
     * @param headerParser the header parser instance to use.
     */
    inline void setHeaderParser(HeaderParser * headerParser);

    /** Get the root node of the document model.
     * @return The root node.
     */
    const HtmlElement * rootNode() const;

  protected:
    virtual void handleStartEndTag(const std::string & tag, const std::vector<Attribute*> & attrs);
    virtual void handleStartTag(const std::string & tag, const std::vector<Attribute*> & attrs);
    virtual void handleEndTag(const std::string & tag);
    virtual void handleData(unsigned int ucodeChar);

  private:

    static void setNewAttributes(HtmlElement * element, const std::vector<Attribute*> & attrs);

    enum TreeState
    {
      INITIAL,
      ROOT_ELEMENT,
      MAIN,
      MAIN_WAITING_TOKEN,
      TRAILING_END
    };

    enum InsertionMode
    {
      BEFORE_HEAD,
      IN_HEAD,
      AFTER_HEAD,
      IN_BODY,
      IN_TABLE,
      IN_CAPTION,
      IN_COLUMN_GROUP,
      IN_TABLE_BODY,
      IN_ROW,
      IN_CELL,
      IN_SELECT,
      AFTER_BODY,
      IN_FRAMESET,
      AFTER_FRAMESET
    };

    UnicodeString m_data;
    unsigned int m_dataGot;
    HeaderParser * m_headerParser;
    TreeState m_state;
    InsertionMode m_insertionMode;
    bool m_isFirst;
    //std::stack<HtmlElement*> m_openElements;
    std::list<HtmlElement*> m_openElements;
    std::list<HtmlElement*> m_activeFormatters;
    HtmlElement * m_head;
    HtmlElement * m_form;
    HtmlElement * m_currentNode;

    // end tag in main phase.
    void mainPhase(const std::string & tag);
    // start tag in main phase.
    void mainPhase(const std::string & tag, const std::vector<Attribute*> & attrs);

    // BEFORE_HEAD phase, start tag.
    void beforeHead(const std::string & tag, const std::vector<Attribute*> & attrs);
    // end tag.
    void beforeHead(const std::string & tag);
    // IN_HEAD phase, start tag.
    void inHead(const std::string & tag, const std::vector<Attribute*> & attrs);
    // end tag
    void inHead(const std::string & tag);
    // AFTER_HEAD phase, start tag.
    void afterHead(const std::string & tag, const std::vector<Attribute*> & attrs);
    // end tag
    void afterHead(const std::string & tag);
    // IN_BODY phase, start tag.
    void inBody(const std::string & tag, const std::vector<Attribute*> & attrs);
    // end tag
    void inBody(const std::string & tag);
    // AFTER_BODY phase, start tag.
    void afterBody(const std::string & tag, const std::vector<Attribute*> & attrs);
    // end tag
    void afterBody(const std::string & tag);

    bool inScope(const std::string & element);
    HtmlElement* activeFormatContains(const std::string & tagName);
    void removeFromActiveFormat(HtmlElement* element);
    void removeFromOpenElements(HtmlElement* element);

    void generateImpliedEndTags(const std::string & except="");
    bool isFormatting(HtmlElement * node);
    bool isPhrasing(HtmlElement * node);
    // disable copies
    HtmlDocument (const HtmlDocument&);
    const HtmlDocument& operator=(const HtmlDocument&);
};


// inline implementation
const UnicodeString & HtmlDocument::data() const
{
  return m_data;
}

unsigned int HtmlDocument::dataGot() const
{
  return m_dataGot;
}
void HtmlDocument::setDataGot(unsigned int value)
{
  m_dataGot = value;
}

void HtmlDocument::setHeaderParser(HeaderParser * headerParser)
{
  m_headerParser = headerParser;
}
#endif

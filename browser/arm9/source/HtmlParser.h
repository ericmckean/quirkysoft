#ifndef HtmlParser_h_seen
#define HtmlParser_h_seen

#include <vector>
#include <string>
/**
 * A parser for HTML. Consumes bytes and calls the handler functions for tags.
 */
class HtmlParser
{
  public:
    //! Types of encoding of a page.
    enum Encoding {
      UTF8_ENCODING, //!< char stream is parsed using UTF-8.
      ISO_ENCODING   //!< char stream is parsed as ISO-8859-1.
    };

    //! Attributes for an element.
    struct Attribute
    {
      std::string name;   //!< name of the attribute.
      std::string value;  //!< value, stripped of leading/trailing quotes.
    };
    //! Constructor.
    HtmlParser();
    //! Destructor.
    virtual ~HtmlParser();

    /** Feed data to the parser. Call this with any amount of data, the state is kept between feeds.
     * To reset the state, call setToStart().
     * @param data data to be parsed.
     * @param length the amount of data in bytes.
     */
    void feed(const char * data, unsigned int length);

    //! Reset the parser to the initial state.
    void setToStart();

    /** Set the encoding to use. The default is UTF-8.
     * @param enc the new encoding.
     */
    void setEncoding(Encoding enc);

    /** Get the current encoding. 
     * @return the current Encoding.
     */
    Encoding encoding() const;

    /** Set the current parsing to plain text mode.
     */
    void setPlainText();

  protected:
    enum ContentModel {
      PCDATA,
      RCDATA,
      CDATA,
      PLAINTEXT
    };

    friend class HtmlParserImpl;
    /** Called when a start-end tag is found.
     * @param tag the tag name (lowercase).
     * @param attrs a vector of attributes for the tag.
     **/
    virtual void handleStartEndTag(const std::string & tag, const std::vector<Attribute*> & attrs);
    /** Called when a start tag is found.
     * @param tag the tag name (lowercase).
     * @param attrs a vector of attributes for the tag.
     **/
    virtual void handleStartTag(const std::string & tag, const std::vector<Attribute*> & attrs);

    /** Called when an end tag is found.
     * @param tag the tag name (lowercase).
     */
    virtual void handleEndTag(const std::string & tag);

    /** Called for each "glyph" of data between tags.
     * @param ucodeChar unicode character index of the data.
     */
    virtual void handleData(unsigned int ucodeChar);

    void setContentModel(ContentModel newModel);

  private:
    //! Nothing to see here.
    class HtmlParserImpl & m_details;
};
#endif

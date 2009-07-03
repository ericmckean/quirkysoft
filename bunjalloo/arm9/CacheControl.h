#ifndef CacheControler_h_seen
#define CacheControler_h_seen

#include <string>

class CacheControl
{
  public:
    /**
     * Set relative seconds since the last time a value was changed.  This may
     * seem a little weird, but it means the CacheControl itself doesn't have
     * to keep track of time, someone else can just tell it how long has
     * passed.
     *
     * @param time relative time in seconds that has gone by since we last got a cache-control header
     */
    void setSeconds(unsigned int time);

    /**
     * Set the cache-control header value.
     * @param value the cache-control header value that needs parsing
     */
    void setCacheControl(const std::string &value);

    /**
     * Set the expires header. This is not implemented.
     * @param value a date representing when the page expires
     */
    void setExpires(const std::string &value);

    /**
     * Find out if we should be caching the page based on what we know.
     * @returns true if we should cache the page
     */
    bool shouldCache() const;
  private:
};

#endif

#ifndef DESKA_MOCK_STREAMBUFFER
#define DESKA_MOCK_STREAMBUFFER

#include <stdexcept>
#include <streambuf>
#include <string>
#include <queue>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <boost/test/test_tools.hpp>

/** @short Internal representation of expectations for the MockStreamBuffer */
struct MockStreamEvent
{
    typedef enum {READ, WRITE, READ_EOF} Direction;
    Direction mode_;
    std::string data_;
    std::string::size_type position_;
    MockStreamEvent(const Direction mode, const std::string &data):
        mode_(mode), data_(data), position_(0)
    {
    }
};

class MockStreamBufferError: public std::runtime_error
{
public:
    MockStreamBufferError(const std::string &what):
        std::runtime_error(what)
    {
    }
};

/** @short A streambuf implementation for mocking of iostreams

This streambuf is intended to be used in unit testing as the underlying streambuf instance for iostreams. Use the
expectRead(), expectWrite() and expectReadEof() to simulate the flow of the data (and the ordering of all operations).

Due to the way the iostreams are architected, a sync request has to be issued by the ostream associated with this streambuf
in order to actually check that the written data match what is expected.  This can be enforced either by issuing a sync() after
each completed write to this buffer, or by simply setting the buffer size to be one.
*/
class MockStreamBuffer : public std::streambuf, public boost::noncopyable
{

public:
    typedef int handle_type;

    /** @short Construct a mocked streambuf

    @arg bufsize The size of internal buffers, both for reading and writing
    */
    explicit MockStreamBuffer(std::size_t bufsize)
        :
        bufsize_(bufsize),
        read_buf_(new char[bufsize]),
        write_buf_(new char[bufsize]),
        use_test_on_throw_(false)
    {
        BOOST_ASSERT(bufsize_ > 0);

        setp(write_buf_.get(), write_buf_.get() + bufsize_);
    }

    /** @short Expect a read request and satisfy it with this data */
    void expectRead(const std::string &s)
    {
        events_.push(MockStreamEvent(MockStreamEvent::READ, s));
    }

    /** @short Expect a write request for this data */
    void expectWrite(const std::string &s)
    {
        events_.push(MockStreamEvent(MockStreamEvent::WRITE, s));
    }

    /** @short Expect a read request and respond with EOF to that */
    void expectReadEof()
    {
        events_.push(MockStreamEvent(MockStreamEvent::READ_EOF, std::string()));
    }

    /** @short Returns whether all requested operations were in fact undertaken */
    bool consumedEverything()
    {
        return events_.empty();
    }

    /** @short Use boost::test macros for reporting error during throw */
    void useBoostTestOnThrow()
    {
        use_test_on_throw_ = true;
    }

protected:
    /**
     * Reads new data
     *
     * This operation is called by input methods when there is no more
     * data in the input buffer. The function fills the buffer with new
     * data, if available.
     *
     * \pre All input positions are exhausted (gptr() >= egptr()).
     * \post The input buffer has new data, if available.
     * \returns traits_type::eof() if a read error occurrs or there are
     *          no more data to be read. Otherwise returns
     *          traits_type::to_int_type(*gptr()).
     */
    virtual int_type underflow()
    {
        BOOST_ASSERT(gptr() >= egptr());

        bool ok;
        ssize_t cnt = real_read(read_buf_.get(), bufsize_);
        ok = (cnt != -1 && cnt != 0);

        if (!ok)
            return traits_type::eof();
        else
        {
            setg(read_buf_.get(), read_buf_.get(), read_buf_.get() + cnt);
            return traits_type::to_int_type(*gptr());
        }
    }

    /**
     * Makes room in the write buffer for additional data.
     *
     * This operation is called by output methods when there is no more
     * space in the output buffer to hold a new element. The function
     * first flushes the buffer's contents to disk and then clears it to
     * leave room for more characters. The given \a c character is
     * stored at the beginning of the new space.
     *
     * \pre All output positions are exhausted (pptr() >= epptr()).
     * \post The output buffer has more space if no errors occurred
     *       during the write to disk.
     * \post *(pptr() - 1) is \a c.
     * \returns traits_type::eof() if a write error occurrs. Otherwise
     *          returns traits_type::not_eof(c).
     */
    virtual int_type overflow(int c)
    {
        BOOST_ASSERT(pptr() >= epptr());

        if (sync() == -1)
            return traits_type::eof();

        if (!traits_type::eq_int_type(c, traits_type::eof()))
        {
            traits_type::assign(*pptr(), c);
            pbump(1);
        }

        return traits_type::not_eof(c);
    }

    /**
     * Flushes the output buffer to disk.
     *
     * Synchronizes the systembuf buffers with the contents of the file
     * associated to this object through the native file handle. The
     * output buffer is flushed to disk and cleared to leave new room
     * for more data.
     *
     * \returns 0 on success, -1 if an error occurred.
     */
    virtual int sync()
    {
        ssize_t cnt = pptr() - pbase();

        bool ok;
        ok = real_write(pbase(), cnt) == cnt;

        if (ok)
            pbump(-cnt);
        return ok ? 0 : -1;
    }

    /** @short Read data from the expectations queue into an internal buffer */
    ssize_t real_read(char_type *buf, std::size_t count)
    {
        //std::cerr << "real_read for " << count << std::endl;
        if (count == 0) {
            return 0;
        }

        if (events_.empty())
            throw_("real_read: no read expected now");

        if (events_.front().mode_ == MockStreamEvent::READ_EOF) {
            events_.pop();
            //std::cerr << "real_read: EOF" << std::endl;
            return 0;
        }

        if (events_.front().mode_ != MockStreamEvent::READ)
            throw_("real_read: unexpected read");

        const std::string &currentStr = events_.front().data_;
        std::string::size_type &position = events_.front().position_;
        std::string::const_iterator it = currentStr.begin() + position;
        BOOST_ASSERT(it <= currentStr.end());

        std::size_t num = std::min(count, currentStr.size() - position);
        //std::cerr << "real_read asked for " << count << ", will read " << num << std::endl;
        std::string::const_iterator end = it + num;
        /*std::string message;
        message.resize(num);
        std::string::iterator dbg = message.begin();*/
        while (it < end) {
            //*dbg++ = *it;
            *buf++ = *it++;
        }
        //std::cerr << "Read |" << message << "|" << std::endl;

        if (it == currentStr.end()) {
            events_.pop();
        } else {
            position += num;
        }
        return num;
    }

    /** @short Write data from the expectations queue into an internal buffer */
    ssize_t real_write(const char_type *buf, std::size_t count)
    {
        if (count == 0)
            return 0;

        if (events_.empty())
            throw_("real_write: nothing expected now");

        if (events_.front().mode_ != MockStreamEvent::WRITE)
            throw_("real_write: unexpected write");

        std::string out;
        out.resize(count);
        std::copy(buf, buf + count, out.begin());

        BOOST_ASSERT(events_.front().position_ < events_.front().data_.size());
        eq_or_throw_(out, events_.front().data_, "real_write: value mismatch", events_.front().position_);
        events_.front().position_ += count;
        if (events_.front().position_ == events_.front().data_.size()) {
            events_.pop();
        }
        return count;
    }

    void throw_(const std::string &message)
    {
        if (use_test_on_throw_) {
            BOOST_FAIL(message);
        }
        throw MockStreamBufferError(message);
    }

    void eq_or_throw_(const std::string &snippet, const std::string &original, const std::string &message,
                      const std::string::size_type offset)
    {
        if (snippet == original.substr(offset, snippet.size()))
            return;

        if (use_test_on_throw_) {
            std::string fake = original;
            fake.replace(offset, std::string::npos, snippet, 0, std::string::npos);
            // In order to show context, we will compare the actually processed data ("fake") with all of the
            // expected data, including part of it which wasn't transmitted yet.
            // This is meant to make debugging easier.
            BOOST_CHECK_EQUAL(fake, original);
            // But then we'll print it as a sequence again, now with correct numbers:
            BOOST_CHECK_EQUAL_COLLECTIONS(fake.begin(), fake.end(),
                                          original.begin(), original.begin() + std::min(fake.size(), original.size()));
        }
        throw MockStreamBufferError(message);
    }


private:
    /**
     * Internal buffer size used during read and write operations.
     */
    std::size_t bufsize_;

    /**
     * Internal buffer used during read operations.
     */
    boost::scoped_array<char> read_buf_;

    /**
     * Internal buffer used during write operations.
     */
    boost::scoped_array<char> write_buf_;

    std::queue<MockStreamEvent> events_;

    bool use_test_on_throw_;
};

#endif // DESKA_MOCK_STREAMBUFFER

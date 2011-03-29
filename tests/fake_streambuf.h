#ifndef DESKA_MOCK_STREAMBUF
#define DESKA_MOCK_STREAMBUF

#include <streambuf>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

#include <string>
#include <iostream>

/** @short */
class MockStreamBuffer : public std::streambuf, public boost::noncopyable
{

public:
    typedef int handle_type;

    explicit MockStreamBuffer(std::size_t bufsize = 1)
        :
        bufsize_(bufsize),
        read_buf_(new char[bufsize]),
        write_buf_(new char[bufsize])
    {
        BOOST_ASSERT(bufsize_ > 0);

        setp(write_buf_.get(), write_buf_.get() + bufsize_);
    }

    virtual ~MockStreamBuffer()
    {
        sync();
    }

protected:
    /**
     * Reads new data from the native file handle.
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
        // FIXME: read here
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

    ssize_t real_read(char_type *buf, std::size_t count)
    {
        std::size_t num = std::min(count, static_cast<std::size_t>(input_.end() - it_));
        //std::cerr << "real_read asked for " << count << ", will read " << num << std::endl;
        std::string::iterator end = it_ + num;
        std::string message;
        message.resize(num);
        std::string::iterator dbg = message.begin();
        while (it_ < end) {
            *dbg++ = *it_;
            *buf++ = *it_++;
        }
        std::cerr << "Read |" << message << "|" << std::endl;
        return num;
    }

    ssize_t real_write(const char_type *buf, std::size_t count)
    {
        std::string out;
        out.resize(count);
        std::copy(buf, buf + count, out.begin());
        std::cerr << "Wrote |" << out << "|" << std::endl;
        return count;
    }

public:
    void setInput(const std::string &s)
    {
        input_ = s;
        it_ = input_.begin();
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

    std::string input_;
    std::string::iterator it_;
};

#endif // DESKA_MOCK_STREAMBUF

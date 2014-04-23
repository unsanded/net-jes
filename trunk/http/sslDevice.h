/*
 * got this code almost literally from http://stackoverflow.com/questions/3668128/how-to-create-a-boost-ssl-iostream
 * 
 */
#ifndef SSL_DEVICE_H
#define SSL_DEVICE_H
#include <boost/asio/ssl.hpp>


using namespace boost::asio;
//
// IOStream device that speaks SSL but can also speak non-SSL
//
class ssl_iostream_device : public boost::iostreams::device<boost::iostreams::bidirectional> {
public:
    ssl_iostream_device(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& _stream, bool _use_ssl ) : stream(_stream)
    {
        use_ssl = _use_ssl;
        need_handshake = _use_ssl;
    }

    void handshake(ssl::stream_base::handshake_type role)
    {
        if (!need_handshake) return;
        need_handshake = false;
        stream.handshake(role);
    }
    std::streamsize read(char* s, std::streamsize n)
    {
        handshake(ssl::stream_base::server); // HTTPS servers read first
        if (use_ssl) return stream.read_some(boost::asio::buffer(s, n));
        return stream.next_layer().read_some(boost::asio::buffer(s, n));
    }
    std::streamsize write(const char* s, std::streamsize n)
    {
        handshake(ssl::stream_base::client); // HTTPS clients write first
        if (use_ssl) return boost::asio::write(stream, boost::asio::buffer(s, n));
        return boost::asio::write(stream.next_layer(), boost::asio::buffer(s, n));
    }
private:
    bool need_handshake;
    bool use_ssl;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& stream;
	

};


#endif

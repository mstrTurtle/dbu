class HTTP_Handler
// = TITLE
// Implements the HTTP protocol
// (synchronous threaded version).
//
// = DESCRIPTION
// This class is called by a
// thread in the Thread Pool.
{
public:
    void open (Socket_Stream *client)
    {
        HTTP_Request request;

        // Store reference to client.
        client_ = client;

        // Synchronously read the HTTP request
        // from the network connection and
        // parse it.
        client_->recv (request);

        parse_request (request);

    }

    void parse_request (HTTP_Request &request)
    {
        // Switch on the HTTP command type.
        switch (request.command ())
        {
        // Client is requesting a file.
        case HTTP_Request::GET:
            // Memory map the requested file.
            Mem_Map input_file;
            input_file.map (request.filename());

            // Synchronously send the file
            // to the client. Block until the
            // file is transferred.
            client_->send (input_file.data (),
            input_file.size ());
            break;

        // Client is storing a file at
        // the server.
        case HTTP_Request::PUT:
        // ...

        }

    }

private:
    // Socket endpoint.
    Socket_Stream *client_;

    // ...

};
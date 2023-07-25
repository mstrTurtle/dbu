class HTTP_Handler :
public Reactor::Event_Handler
// = TITLE
// Implements the HTTP protocol
// (synchronous reactive version).
//
// = DESCRIPTION
// The Event_Handler base class
// defines the hooks for
// handle_input()/handle_output().
//
// = PATTERN PARTICIPANTS
// Reactor = Reactor
// Event Handler = HTTP_Handler
{
public:
    void open (Socket_Stream *client)
    {
        // Initialize state for request
        request_.state_ = INCOMPLETE;

        // Store reference to client.
        client_ = client;

        // Register with the reactor for reading.
        reactor_->register_handler
            (client_->handle (),
            this,
            Reactor::READ_MASK);

    }

    // This is called by the Reactor when
    // we can read from the client handle.
    void handle_input (void)
    {
        int result = 0;

        // Non-blocking read from the network
        // connection.
        do
            result = request_.recv (client_->handle ());
        while (result != SOCKET_ERROR
            && request_.state_ == INCOMPLETE);

        // No more progress possible,
        // blocking will occur
        if (request_.state_ == INCOMPLETE
            && errno == EWOULDBLOCK)
            reactor_->register_handler
                (client_->handle (),
                this,
                Reactor::READ_MASK);
        else
            // We now have the entire request
            parse_request ();

    }

    void parse_request (void)
    {

        // Switch on the HTTP command type.
        switch (request_.command ()) {
        // Client is requesting a file.
        case HTTP_Request::GET:
            // Memory map the requested file.
            file_.map (request_.filename ());

            // Transfer the file using Reactive I/O.
            handle_output ();
        break;

        // Client is storing a file at
        // the server.
        case HTTP_Request::PUT:
            // ...

        }

    }

    void handle_output (void)
    {
        // Asynchronously send the file
        // to the client.
        if (client_->send (file_.data (),
            file_.size ())
            == SOCKET_ERROR
            && errno == EWOULDBLOCK)
            // Register with reactor...
        else
            // Close down and release resources.
            handle_close ();
    }

private:
    // Set at initialization.
    Reactor *reactor_;

    // Memory-mapped file_;
    Mem_Map file_;

    // Socket endpoint.
    Socket_Stream *client_;

    // HTTP Request holder.
    HTTP_Request request_;

};
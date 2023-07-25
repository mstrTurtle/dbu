/**
  * @file     	a.cpp
  * @author   	xiaojiayu@scutech.com
  * @version	V4.01
  * @date    	07-DEC-2017 
  * @brief   	Universal Synchronous/Asynchronous Receiver/Transmitter 
  * @attention
  *  This file is part of OST.                                                  \n                                                                  
  *  This program is free software; you can redistribute it and/or modify 		\n     
  *  it under the terms of the GNU General Public License version 3 as 		    \n   
  *  published by the Free Software Foundation.                               	\n 
  *  You should have received a copy of the GNU General Public License   		\n      
  *  along with OST. If not, see <http://www.gnu.org/licenses/>.       			\n  
  *  Unless required by applicable law or agreed to in writing, software       	\n
  *  distributed under the License is distributed on an "AS IS" BASIS,         	\n
  *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  	\n
  *  See the License for the specific language governing permissions and     	\n  
  *  limitations under the License.   											\n
  *   																			\n
  * @htmlonly 
  * <span style="font-weight: bold">History</span> 
  * @endhtmlonly 
  * Version|Auther|Date|Describe
  * ------|----|------|-------- 
  * V3.3|Jones Lee|07-DEC-2017|Create File
  * <h2><center>&copy;COPYRIGHT 2017 WELLCASA All Rights Reserved.</center></h2>
  */  

#include "ace/ACE.h"
#include "ace/SOCK.h"
#include "ace/Proactor.h"

class HTTP_Handler
: public Proactor::Event_Handler
// = TITLE
// Implements the HTTP protocol
// (asynchronous version).
//
// = PATTERN PARTICIPANTS
// Proactive Initiator = HTTP_Handler
// Asynch Op = Network I/O
// Asynch Op Processor = OS
// Completion Dispatcher = Proactor
// Completion Handler = HTPP_Handler
{
public:
    void open (Socket_Stream *client)
    {
        // Initialize state for request
        request_.state_ = INCOMPLETE;

        // Store reference to client.
        client_ = client;

        // Initialize asynch read stream
        stream_.open (*this,
        client_->handle (),
        proactor_);

        // Start read asynchronously.
        stream_.read (request_.buffer (),
        request_.buffer_size ());

    }

    // This is called by the Proactor
    // when the asynch read completes
    void handle_read_stream
        (u_long bytes_transferred)
    {
        if (request_.enough_data
            (bytes_transferred))
            parse_request ();
        else
            // Start reading asynchronously.
            stream_.read (request_.buffer (),
                request_.buffer_size ());
    }

    void parse_request (void)
    {
        // Switch on the HTTP command type.
        switch (request_.command ()) {
            // Client is requesting a file.
        case HTTP_Request::GET:
            // Memory map the requested file.
            file_.map (request_.filename ());

            // Start writing asynchronously.
            stream_.write (file_.buffer (),
                file_.buffer_size ());

            break;

            // Client is storing a file
            // at the server.
        case HTTP_Request::PUT:
            // ...
            ;
        }

    }

    void handle_write_stream
    (u_long bytes_transferred)
    {
        if (file_.enough_data
            (bytes_transferred))
            // Success....
        else
            // Start another asynchronous write
            stream_.write (file_.buffer (),
            file_.buffer_size ());
    }

private:
    // Set at initialization.
    Proactor *proactor_;

    // Memory-mapped file_;
    Mem_Map file_;

    // Socket endpoint.
    Socket_Stream *client_;

    // HTTP Request holder
    HTTP_Request request_;

    // Used for Asynch I/O
    Asynch_Stream stream_;

};
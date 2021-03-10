#include <iostream>
#include <chrono>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif // handles compile warning about which version of WIN32 to use


#define ASIO_STANDALONE  // as we don't need boost_asio
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

std::vector<char> vBuffer(1 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout << "\n\nRead " << length << " bytes\n\n";

				for (int i = 0; i < length; i++)
				{
					std::cout << vBuffer[i];
				}

				GrabSomeData(socket);
			}
		}
	);
}


int main()
{

	asio::error_code ec;


	// create a context - essentially the platform specific interface.
	asio::io_context context;


	// give some fake tasks to asio so the context doesn't finish.
	asio::io_context::work idleWork(context);

	// start the context in its own thread.
	std::thread threadContext = std::thread([&]() { context.run(); });



	// get the address of somewhere we wish to connect to.
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);


	// create a socket, the context will deliver the implementation
	asio::ip::tcp::socket socket(context);

	// tell socket to try and connect
	socket.connect(endpoint, ec);

	if (!ec) {
		std::cout << "Connected!\n";
	}
	else {
		std::cout << "Failed to connect to address:\n" << ec.message() << "\n";
	}

	
	if (socket.is_open())
	{

		// works, but program terminates before doing something. so let async_read_some prime the context before actually sending some request.
		GrabSomeData(socket);


		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";
		
		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
		
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(2000ms);
	}
	
	system("pause");
	return 0;
}
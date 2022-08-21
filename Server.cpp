#include "libwebsocket/WebsocketServer.h"

int main(int argc, char** args)
{
	if (argc < 2)
	{
		std::cerr << "Usage " << args[0] << " address port" << std::endl;
		std::cerr << "Default port 50000 is used if omitted" << std::endl;
		return -1;
	}

	std::string address(args[1]);

	int port = 50000;
	if (argc > 2)
	{
		port = std::stoi(std::string(args[2]));
	}

	socket::init();

	{
		websocketServer ws;
		ws.run(address, port);

		//wait for a connection
		while (!ws.hasConnections())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		while (ws.hasConnections())
		{
			/**/
			{
				websocketMessage* m = new websocketMessage();
				m->type = FRAME_BINARY;
				m->buf.resize(32 * 24 * sizeof(float));

				for (int c = 0; c < 32 * 24; ++c)
				{
					float v = rand() / (float)RAND_MAX;
					*(float*)(m->buf.data() + c * sizeof(float)) = v;
				}

				ws.broadcastMessage(m);
			}
			/**/

			/**
			message m;
			m.type = FRAME_TEXT;

			std::string message = "hello client!";

			m.buf.insert(m.buf.begin(), message.begin(), message.end());

			thisPtr->sendWebsocketMessage(thisPtr->s, m);
			/**/

			std::vector<websocketMessage*> messages;
			ws.receiveMessages(messages);

			if (messages.size() > 0)
			{
				bool close = false;
				for (auto m : messages)
				{
					if (m->type == FRAME_CLOSE)
					{
						close = true;
					}

					if (close)
					{
						free(m);
						continue;
					}

					m->buf.push_back('\0');
					std::cout << "Message received: " << m->buf.data() << std::endl;
					free(m);
				}

				if (close)
				{
					ws.close();
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	socket::shutdown();

	return 0;
}

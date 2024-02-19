#include "libwebsocket/WebsocketServer.h"

#ifndef _WIN32
#include <MLX90640/MLX90640_API.h>

#include <cmath>

#define MLX_CHECK(x) \
	{ int error = 0; \
	error = (x); \
	if(error) printf("Error in " #x ": %i\n", error); }\

#define MLX_I2C_ADDR 0x33
#endif

#include <stdlib.h>
#include <sys/wait.h>
#include <fstream>

int main(int argc, char** args)
{
	if (argc < 2)
	{
		std::cerr << "Usage " << args[0] << " address port refreshRate" << std::endl;
		std::cerr << "Default port 50000 and refreshRate 4HZ is used if omitted" << std::endl;
		return -1;
	}

	std::string address(args[1]);

	int port = 50000;
	if (argc > 2)
	{
		port = std::stoi(std::string(args[2]));
	}

	int inputRefreshRate = 4;
	if(argc > 3)
	{
		inputRefreshRate = std::stoi(std::string(args[3]));
		if(inputRefreshRate < 1)
		{
			inputRefreshRate = 1;
		}

		if(inputRefreshRate > 64)
		{
			inputRefreshRate = 64;
		}
	}

	float temperature = 0.0f, pressure = 0.0f, humidity = 0.0f;
	{ //Get env data using python script
		pid_t pid;
		if((pid = fork()) < 0)
		{
			return -1; //fork failed
		}
		else if(pid == 0)
		{
			//child process
			execlp("python3", "python3", "getEnvData.py");
		}
		else
		{
			//parent process, wait for child to complete
			int status;
			wait(&status);
			if(status != 0)
			{
				return status;
			}

			std::ifstream file("envDataOutput.txt");
			std::string str;
			file >> temperature >> str;
			file >> pressure >> str;
			file >> humidity >>  str;

			//std::cout << temperature << std::endl
			//	<< pressure << std::endl
			//	<< humidity << std::endl;
		}
	}

#ifndef _WIN32
	paramsMLX90640 mlx90640;
	std::chrono::microseconds frame_time;
	{ //Init MLX90640 thermal camera
		std::cout << "Initializing MLX90640 thermal camera" << std::endl;
		MLX_CHECK(MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0));
		MLX_CHECK(MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0));

		//1hz  0b001 baud 400k
		//2hz  0b010
		//4hz  0b011
		//8hz  0b100
		//16hz 0b101 baud 1000k
		//32hz 0b110
		//64hz 0b111
		MLX_CHECK(MLX90640_SetRefreshRate(MLX_I2C_ADDR, std::log2(inputRefreshRate)+1));

		MLX_CHECK(MLX90640_SetChessMode(MLX_I2C_ADDR));

		static uint16_t eeMLX90640[832];
		MLX_CHECK(MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640));
		MLX_CHECK(MLX90640_ExtractParameters(eeMLX90640, &mlx90640));

		int refresh_rate = 1 << (MLX90640_GetRefreshRate(MLX_I2C_ADDR)-1);
		frame_time = std::chrono::microseconds(1000000 / refresh_rate);
		std::cout << "Refresh rate: " << refresh_rate << std::endl;

	}
#endif

        float emissivity = 0.86; //0.78 for roasted coffee
	std::cout << "Emissivity: " << emissivity << std::endl;

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
			auto start = std::chrono::system_clock::now();

#ifdef _WIN32
			{
				websocketMessage* m = new websocketMessage();
				m->type = FRAME_BINARY;
				m->buf.resize(32 * 24 * sizeof(float) + sizeof(float) * 4); //add float for Ta, temp, pressure, humidity

				for (int c = 0; c < 32 * 24; ++c)
				{
					float v = rand() / (float)RAND_MAX;
					*(float*)(m->buf.data() + c * sizeof(float)) = v;
				}

				float Ta = 24.0f;
				memcpy(m->buf.data() + 32 * 24 * sizeof(float), &Ta, sizeof(Ta));
				memcpy(m->buf.data() + 32 * 24 * sizeof(float) + sizeof(float), &temperature, sizeof(temperature));
				memcpy(m->buf.data() + 32 * 24 * sizeof(float) + sizeof(float) * 2, &pressure, sizeof(pressure));
				memcpy(m->buf.data() + 32 * 24 * sizeof(float) + sizeof(float) * 3, &humidity, sizeof(humidity));

				ws.broadcastMessage(m);
			}
#else
			{ //Read thermal camera values
				uint16_t frame[834];
				MLX90640_GetFrameData(MLX_I2C_ADDR, frame);
				float eTa = MLX90640_GetTa(frame, &mlx90640);
				static float mlx90640To[768];

				MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);
				MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &mlx90640);
				MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &mlx90640);

				websocketMessage* m = new websocketMessage();
				m->type = FRAME_BINARY;
				m->buf.resize(32 * 24 * sizeof(float) + sizeof(float) * 4); //add float for Ta, temp, pressure, humidity
				memcpy(m->buf.data(), mlx90640To, 768 * sizeof(float));
				memcpy(m->buf.data() + 768 * sizeof(float), &eTa, sizeof(eTa));
				memcpy(m->buf.data() + 768 * sizeof(float) + sizeof(float), &temperature, sizeof(temperature));
                                memcpy(m->buf.data() + 768 * sizeof(float) + sizeof(float) * 2, &pressure, sizeof(pressure));
                                memcpy(m->buf.data() + 768 * sizeof(float) + sizeof(float) * 3, &humidity, sizeof(humidity));
				ws.broadcastMessage(m);
			}
#endif

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

#ifdef _WIN32
			std::this_thread::sleep_for(std::chrono::microseconds(1000));
#else
			auto end = std::chrono::system_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                        auto offset_usec = std::chrono::microseconds(850); //offset added to make sure frames are ready in time
                        std::this_thread::sleep_for(std::chrono::microseconds(frame_time - elapsed + offset_usec));
#endif
		}
	}

	socket::shutdown();

	return 0;
}

/*
	Original author of the starter code
	Tanzir Ahmed
	Department of Computer Science & Engineering
	Texas A&M University
	Date: 2/8/20

	Please include your Name, UIN, and the date below
	Name:
	UIN:
	Date:
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;

	string filename = "";
	int maxMsg = MAX_MESSAGE;
	bool newChannel = false;
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1)
	{
		switch (opt)
		{
		case 'p':
			p = atoi(optarg);
			break;
		case 't':
			t = atof(optarg);
			break;
		case 'e':
			e = atoi(optarg);
			break;
		case 'f':
			filename = optarg;
			break;
		case 'm':
			maxMsg = atoi(optarg);
			break;
		case 'c':
			newChannel = true;
			break;
		}
	}

	// start server and pass flags
	char mstr[32];
	snprintf(mstr, sizeof(mstr), "%d", maxMsg);
	char *cmd1[] = {(char *)"./server", (char *)"-m", mstr, nullptr};

	pid_t child = fork();
	if (child == -1)
	{
		return 1;
	}

	if (child == 0)
	{
		// In child, start the server
		execvp(cmd1[0], cmd1);
		return 0;
	}

	FIFORequestChannel *controlChannel = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
	FIFORequestChannel *chan = controlChannel;

	if (newChannel)
	{
		MESSAGE_TYPE m = NEWCHANNEL_MSG;
		controlChannel->cwrite(&m, sizeof(MESSAGE_TYPE));
		char reply[30];
		controlChannel->cread(&reply, sizeof(reply));
		FIFORequestChannel *newFIFOChannel = new FIFORequestChannel(reply, FIFORequestChannel::CLIENT_SIDE);
		chan = newFIFOChannel;
	}

	// initial data point request
	// char buf[maxMsg]; // 256
	vector<char> buf(maxMsg);
	datamsg x(p, t, e);

	memcpy(buf.data(), &x, sizeof(datamsg));
	chan->cwrite(buf.data(), sizeof(datamsg)); // question
	double reply;
	chan->cread(&reply, sizeof(double)); // answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;

	// Initialize ostringstream
	ostringstream oss;

	if (t == 0)
	{
		double pointCount = 0;

		string outputName = "received/x1.csv";

		FILE *file = fopen(outputName.c_str(), "w");
		if (file == nullptr)
		{
			throw runtime_error("Failed to open output file");
		}

		for (size_t i = 0; i < 1000; i++)
		{
			datamsg newPts(p, pointCount, 1);
			memcpy(buf.data(), &newPts, sizeof(datamsg));
			chan->cwrite(buf.data(), sizeof(datamsg));
			double ecg1;
			chan->cread(&ecg1, sizeof(double));

			datamsg newPts2(p, pointCount, 2);
			memcpy(buf.data(), &newPts2, sizeof(datamsg));
			chan->cwrite(buf.data(), sizeof(datamsg));
			double ecg2;
			chan->cread(&ecg2, sizeof(double));

			oss.str("");
			oss << pointCount << "," << ecg1 << "," << ecg2;

			fwrite(oss.str().c_str(), sizeof(char), strlen(oss.str().c_str()), file);
			fwrite("\n", sizeof(char), 1, file);
			pointCount += 0.004;
		}
		fclose(file);
	}

	// sending a request for full file size
	if (filename != "")
	{
		filemsg fm(0, 0);

		int len = sizeof(filemsg) + (filename.size() + 1);
		char *buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), filename.c_str());
		chan->cwrite(buf2, len); // I want the file length;

		delete[] buf2;

		// Read full file size
		__int64_t fileSizeLeft;
		chan->cread(&fileSizeLeft, sizeof(__int64_t));

		// Open new file
		oss.str("");
		oss << "received/" << filename;
		string outputName = oss.str();

		ofstream ofs(outputName, ios::binary);
		if (!ofs.is_open())
		{
			throw runtime_error("Failed to open output file");
		}

		int offsetCount = 0;
		while (fileSizeLeft > 0)
		{
			int requestSize;

			if (fileSizeLeft > maxMsg)
			{
				fileSizeLeft -= maxMsg;
				requestSize = maxMsg;
			}
			else
			{
				requestSize = fileSizeLeft;
				fileSizeLeft = 0;
			}

			// Send request for binary data
			filemsg fileMessage(maxMsg * offsetCount, requestSize);
			char *buf2 = new char[len];
			memcpy(buf2, &fileMessage, sizeof(filemsg));
			strcpy(buf2 + sizeof(filemsg), filename.c_str());
			chan->cwrite(buf2, len); // I want some binary data;

			delete[] buf2;
			offsetCount++;

			// Receieve binary data from server
			vector<char> binaryData(fileMessage.length);
			int count = chan->cread(binaryData.data(), fileMessage.length);
			ofs.write(binaryData.data(), count);
		}

		ofs.close();
	}

	// closing the channel
	MESSAGE_TYPE m = QUIT_MSG;
	chan->cwrite(&m, sizeof(MESSAGE_TYPE));
	delete chan;

	if (newChannel)
	{
		controlChannel->cwrite(&m, sizeof(MESSAGE_TYPE));
		delete controlChannel;
	}
}

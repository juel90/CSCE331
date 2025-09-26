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

	pid_t child = fork();
	if (child == -1)
	{
		return 1;
	}

	if (child == 0)
	{
		// In child, start the server
		execlp("./server", NULL);
		return 0;
	}

	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1)
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
		}
	}

	FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

	// example data point request
	char buf[MAX_MESSAGE]; // 256
	datamsg x(p, t, e);

	memcpy(buf, &x, sizeof(datamsg));
	chan.cwrite(buf, sizeof(datamsg)); // question
	double reply;
	chan.cread(&reply, sizeof(double)); // answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;

	// sending a request for full file size
	filemsg fm(0, 0);
	ostringstream oss;
	oss << p << ".csv";
	string fname = oss.str();

	int len = sizeof(filemsg) + (fname.size() + 1);
	char *buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len); // I want the file length;

	delete[] buf2;

	// Read full file size
	__int64_t fileSizeLeft;
	chan.cread(&fileSizeLeft, sizeof(__int64_t));

	// Open new file
	oss.clear();
	oss << "x" << fname;
	string outputName = oss.str();
	FILE *file = fopen(outputName.c_str(), "wb");

	if (file == nullptr)
	{
		perror("Error opening file");
		return 1;
	}

	cout << " Full size is " << fileSizeLeft << " bytes" << endl;
	int offsetCount = 0;
	while (fileSizeLeft > 0)
	{
		int requestSize;

		if (fileSizeLeft > MAX_MESSAGE)
		{
			fileSizeLeft -= MAX_MESSAGE;
			requestSize = MAX_MESSAGE;
		}
		else
		{
			requestSize = fileSizeLeft;
			fileSizeLeft = 0;
		}

		// Send request for binary data
		cout << "Offset is " << MAX_MESSAGE * offsetCount << " bytes, requesting " << requestSize << " bytes.";
		filemsg fileMessage(MAX_MESSAGE * offsetCount, requestSize);
		char *buf2 = new char[len];
		memcpy(buf2, &fileMessage, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		chan.cwrite(buf2, len); // I want some binary data;

		delete[] buf2;
		offsetCount++;

		// Receieve binary data from server
		__int64_t binaryData;
		chan.cread(&binaryData, sizeof(__int64_t));
		cout << "Received data " << binaryData << endl;
	}

	fclose(file);

	// closing the channel
	MESSAGE_TYPE m = QUIT_MSG;
	chan.cwrite(&m, sizeof(MESSAGE_TYPE));
}

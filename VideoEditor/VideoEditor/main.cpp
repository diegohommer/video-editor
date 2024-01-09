#include "configs.h"

// Callback function for the trackbar
void onTrackbar(int pos, void* userdata) {
	int* valuePtr = static_cast<int*>(userdata);
	*valuePtr = pos;
}

int main(int argc, char** argv)
{
	int camera = 0;
	int value = 0;
	bool alt = false;
	bool hasTrackbar = false;
	bool resetTrackbar = false;
	bool initializedRecording = false;
	cv::VideoWriter output;

	VideoCapture cap;
	std::vector<bool> configs(11,false); 
	std::vector<bool> alts(3, false);
	print_configs();
	// open the default camera, use something different from 0 otherwise;
	// Check VideoCapture documentation.
	if (!cap.open(camera))
		return 0;
	for (;;)
	{
		Mat frame, edited_frame;
		cap >> frame;
		cap >> edited_frame;

		// Introduce a delay and check for key events
		int key = cv::waitKey(1); // 1 ms delay

		// Check if a key is pressed
		if (key != -1) 
		{
			// Update active effects using pressed key
			update_configs(configs, alts, key, &resetTrackbar);
		}
		// Check and apply active effects to frame
		check_configs(configs, &edited_frame, value, alts);


		if (frame.empty()) break; // end of video stream

		imshow("Original frame", frame);

		namedWindow("Edited_Frame", WINDOW_AUTOSIZE);
		// Conditional Trackbar display (if 'gaussian' or 'brightness' or 'contrast')
		if (configs[0] || configs[3] || configs[4])
		{
			if (!hasTrackbar)
			{
				createTrackbar("Value", "Edited_Frame", NULL, 100, onTrackbar, &value);
				hasTrackbar = true;
			}
			if (resetTrackbar)
			{
				setTrackbarPos("Value", "Edited_Frame", 0);
				resetTrackbar = false;
				value = 0;
			}
		}else
		{
			// If trackbar still exists and none of the options that use it are active
			// the program deletes the trackbar
			if (hasTrackbar)
			{
				destroyWindow("Edited_Frame");
				namedWindow("Edited_Frame", WINDOW_AUTOSIZE);
				hasTrackbar = false;
				value = 0;
			}
		}
		imshow("Edited_Frame", edited_frame);


		// Recording logic
		if (configs[10])
		{
			if (!initializedRecording)
			{
				// Initialize MJPG recording 
				int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
				std::string file_name = "recording.avi";
				int fps = 5;
				Size frame_size(640, 480);
				output.open(file_name, fourcc, fps, frame_size);
				initializedRecording = true;
				std::cout << "\nStarted recording";
			}
			output.write(edited_frame);
		}else if (initializedRecording)
		{
			// End and save recording file
			output.release();
			initializedRecording = false;
			std::cout << "\nFinished recording\n";
		}


		if (waitKey(1) == 27) break; // stop capturing by pressing ESC
	}
	cap.release(); // release the VideoCapture object
	return 0;
}
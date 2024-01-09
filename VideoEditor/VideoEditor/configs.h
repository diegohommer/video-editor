#include <opencv2/opencv.hpp>
using namespace cv;

void print_configs()
{
	std::cout << "G or g : Gaussian blur (Blurring)\n";
	std::cout << "E or e : Edges (Canny)\n";
	std::cout << "S or s : gradient (Sobel)\n";
	std::cout << "B or b : Brightness enhancement\n";
	std::cout << "C or c : Contrast enhancement\n";
	std::cout << "N or n : Negative\n";
	std::cout << "Y or y : toggle graYscale\n";
	std::cout << "H or h : toggle resize to Half of each dimension\n";
	std::cout << "R or r : Rotate video\n";
	std::cout << "F or f : Flip video\n";
	std::cout << "V or v : toggle Video recording\n";
	std::cout << "1 : Switch between positive and negative bias in brightness enhancement\n";
	std::cout << "2 : Switch between clockwise and counterclockwise rotation\n";
	std::cout << "3 : Switch between vertical and horizontal mirroring\n";

}

void update_configs(std::vector<bool>& configs, std::vector<bool>& alts, int key, bool *reset_trackbar)
{
	switch (key)
	{
		// Gaussian blur option
		case 'G':
		case 'g':
			configs[0] = !configs[0];
			configs[3] = false;
			configs[4] = false;
			(*reset_trackbar) = true;
			break;

		// Edge detection option
		case 'E':
		case 'e':
			configs[1] = !configs[1];
			break;

		// Sobel option
		case 'S':
		case 's':
			configs[2] = !configs[2];
			break;

		// Brightness adjust option
		case 'B':
		case 'b':
			configs[3] = !configs[3];
			configs[0] = false;
			configs[4] = false;
			(*reset_trackbar) = true;
			break;

		// Contrast adjust option
		case 'C':
		case 'c':
			configs[4] = !configs[4];
			configs[0] = false;
			configs[3] = false;
			(*reset_trackbar) = true;
			break;

		// Negative option
		case 'N':
		case 'n':
			configs[5] = !configs[5];
			break;

		// greYscale option
		case 'Y':
		case 'y':
			configs[6] = !configs[6];
			break;

		// Redimension option
		case 'H':
		case 'h':
			configs[7] = !configs[7];
			break;

		// Rotate option
		case 'R':
		case 'r':
			configs[8] = !configs[8];
			break;

		// Flip option
		case 'F':
		case 'f':
			configs[9] = !configs[9];
			break;

		// Video recording option
		case 'V':
		case 'v':
			configs[10] = !configs[10];
			break;

		// Alt interpretation of brighness (positive - false or negative - true)
		case '1':
			alts[0] = !alts[0];
			break;

		// Alt interpretation of rotation (clockwise - false or counterclockwise - true)
		case '2':
			alts[1] = !alts[1];
			break;

		// Alt interpretation of mirroring (vertical or horizontal)
		case '3':
			alts[2] = !alts[2];
			break;
	}
}

void check_configs(std::vector<bool>& configs, Mat* edited_frame, int trackbar_value, std::vector<bool>& alts)
{
	// Convert to greyscale option
	if (configs[6])
	{
		Mat grayscaleImage;
		cvtColor(*edited_frame, grayscaleImage, COLOR_BGR2GRAY);
		cvtColor(grayscaleImage, *edited_frame, COLOR_GRAY2BGR);
	}

	// Video redimension option
	if (configs[7])
	{
		Mat resized_frame;

		// Halfs height and width
		int newWidth = (*edited_frame).cols / 2;
		int newHeight = (*edited_frame).rows / 2;

		// Get a resized (halfed) version of the edited frame
		resize(*edited_frame, resized_frame, Size(newWidth, newHeight));

		*edited_frame = resized_frame;
	}


	// Video 90º rotation option
	if (configs[8])
	{
		Mat rotated_frame;

		if (!alts[1])
		{
			// Clockwise rotation
			rotate(*edited_frame, rotated_frame, ROTATE_90_CLOCKWISE);
		}
		else
		{
			// Counterclockwise rotation
			rotate(*edited_frame, rotated_frame, ROTATE_90_COUNTERCLOCKWISE);
		}

		*edited_frame = rotated_frame;
	}

	// Video flipping option
	if (configs[9])
	{
		Mat mirror_frame;

		if (!alts[2])
		{
			// Horizontal flip 
			flip(*edited_frame, mirror_frame, 0);
		}
		else
		{
			// Vertical flip
			flip(*edited_frame, mirror_frame, 1);
		}

		*edited_frame = mirror_frame;
	}


	// Gaussian Blur option
	if (configs[0])
	{
		// Create a separate image for the output of the blurred image
		Mat gaussian;

		// Since GaussianBlur doesn't accept an even numbered kernel,
		// if the trackbar_value input is even we convert it to the
		// closest odd number (trackbar_value + 1)
		if (trackbar_value % 2 == 0)
		{
			trackbar_value = trackbar_value + 1;
		}

		// Declares the odd dimension kernel and generates the blurred frame
		Size kernel(trackbar_value, trackbar_value);
		GaussianBlur(*edited_frame, gaussian, kernel,100,100);

		// Copy the blurred frame to the original edited_frame
		*edited_frame = gaussian;
	}


	// Edge detection option
	if (configs[1])
	{
		// Create a separate image for the output of Canny
		Mat edges;
		Canny(*edited_frame, edges, 50, 80);

		// Copy the edges frame to the original edited_frame
		cvtColor(edges, *edited_frame, COLOR_GRAY2BGR);
	}


	// Sobel option
	if (configs[2])
	{
		// Apply Sobel operator to get both X and Y gradients
		Mat sobelX, sobelY;
		Sobel(*edited_frame, sobelX, CV_16S, 1, 0, 3);
		Sobel(*edited_frame, sobelY, CV_16S, 0, 1, 3);

		// Convert the gradient images to 8-bit unsigned format
		// (imshow expects pixel values between 0 and 255 for display)
		Mat sobelX8U, sobelY8U;
		convertScaleAbs(sobelX, sobelX8U);
		convertScaleAbs(sobelY, sobelY8U);

		// Combine X and Y gradients to obtain the final result
		Mat sobel;
		addWeighted(sobelX8U, 0.5, sobelY8U, 0.5, 0, sobel);

		*edited_frame = sobel;
	}


	// Adjust brightness option
	if (configs[3])
	{
		double bias = (double)trackbar_value;

		// If alts[0] flag is set to true, trackbar_value refers to negative bias
		if (alts[0])
		{
			bias = -bias;
		}

		Mat brightAdjusted;
		edited_frame->convertTo(brightAdjusted, -1, 1.0, bias);
		*edited_frame = brightAdjusted;
	}


	// Adjust contrast option
	if (configs[4])
	{
		double alpha = (double)trackbar_value;

		// Scaling factor for alpha (if alpha < 10 reduces contrast)
		alpha /= 10;

		Mat contrastAdjusted;
		edited_frame->convertTo(contrastAdjusted, -1, alpha, 0);
		*edited_frame = contrastAdjusted;
	}


	// Adjust contrast option
	if (configs[5])
	{
		Mat negative;
		edited_frame->convertTo(negative, -1, -1, 255);
		*edited_frame = negative;
	}
}
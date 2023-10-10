#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

// Define global variables for trackbar values
int lowerHue = 30;
int upperHue = 43;
int lowerSaturation = 75;
int upperSaturation = 255;
int lowerValue = 100 ;
int upperValue = 255;

Mat image;  // Global variable for the image
Mat segmentedImage;  // Global variable for the segmented image

// Callback function for HSV trackbars
void onTrackbar(int, void*) {
    // Convert the image to HSV color space
    Mat hsvImage;
    cvtColor(image, hsvImage, COLOR_BGR2HSV);

    // Define HSV ranges based on trackbar values
    Scalar lowerField(lowerHue, lowerSaturation, lowerValue);
    Scalar upperField(upperHue, upperSaturation, upperValue);

    // Create a mask for the playing field
    Mat fieldMask, teamAMask, teamBMask;
    inRange(hsvImage, lowerField, upperField, fieldMask);

    // Create a mask for Team A and Team B (similar to your code)
    inRange(hsvImage, lowerField, upperField, fieldMask);

    // Combine the masks and apply to the original image
    Mat teamAOutput = Mat::zeros(image.size(), image.type());
    Mat teamBOutput = Mat::zeros(image.size(), image.type());
    // Set appropriate colors for Team A and Team B's output
    teamAOutput.setTo(Scalar(0, 0, 0), teamAMask);
    teamBOutput.setTo(Scalar(0, 0, 0), teamBMask);

    segmentedImage = teamAOutput + teamBOutput;
    segmentedImage.setTo(Scalar(0, 255, 0), fieldMask);

    // Display the segmented image with updated HSV values
    imshow("Segmented Image", segmentedImage);
}

int main() {
    // Load the specific image (im1.jpg) and process it
    std::string imagePath = "Images/im13.jpg";  // Path to your image
    image = imread(imagePath);  // Remove "Mat" here
    if (image.empty()) {
        std::cerr << "Image not found: " << imagePath << std::endl;
        return -1;
    }

    // Create windows for displaying images
    namedWindow("Segmented Image", WINDOW_NORMAL);

    // Create trackbars for adjusting HSV values
    createTrackbar("L Hue", "Segmented Image", &lowerHue, 180, onTrackbar);
    createTrackbar("L Saturation", "Segmented Image", &lowerSaturation, 255, onTrackbar);
    createTrackbar("L Value", "Segmented Image", &lowerValue, 255, onTrackbar);
    createTrackbar("U Hue", "Segmented Image", &upperHue, 180, onTrackbar);
    createTrackbar("U Saturation", "Segmented Image", &upperSaturation, 255, onTrackbar);
    createTrackbar("U Value", "Segmented Image", &upperValue, 255, onTrackbar);
    
    // Initial call to the callback function to display the segmented image
    onTrackbar(0, 0);

    // Wait for user to press a key
    waitKey(0);

    return 0;
}
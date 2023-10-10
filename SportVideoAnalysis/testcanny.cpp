#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <cerrno>

using namespace cv;

// Define global variables for trackbar values
int cannyThreshold1Original = 90;
int cannyThreshold2Original = 200;

int cannyThreshold1Segmented = 85;
int cannyThreshold2Segmented = 101;

int cannyThreshold1playersOnly = 90;
int cannyThreshold2playersOnly = 200;

Mat image;  // Global variable for the image
Mat segmentedImage;  // Global variable for the segmented image
Mat playerMask;     // Invert the field mask to get the player mask
Mat playersOnlyImage; // Global variable for the playersOnly image

// Callback function for the trackbar of the original image
void onTrackbarOriginal(int, void*) {
    // Apply Canny edge detection to the original image
    Mat cannyEdgesBeforeSegment;
    cvtColor(image, cannyEdgesBeforeSegment, COLOR_BGR2GRAY);
    GaussianBlur(cannyEdgesBeforeSegment, cannyEdgesBeforeSegment, Size(5, 5), 1.5);
    Canny(cannyEdgesBeforeSegment, cannyEdgesBeforeSegment, cannyThreshold1Original, cannyThreshold2Original);

    // Display the image with Canny edges
    imshow("Canny Edges Original", cannyEdgesBeforeSegment);
}

// Callback function for the trackbar of the segmented image
void onTrackbarSegmented(int, void*) {
    // Apply Canny edge detection to the segmented image
    Mat cannyEdgesAfterSegment;
    cvtColor(segmentedImage, cannyEdgesAfterSegment, COLOR_BGR2GRAY);
    GaussianBlur(cannyEdgesAfterSegment, cannyEdgesAfterSegment, Size(5, 5), 1.5);
    Canny(cannyEdgesAfterSegment, cannyEdgesAfterSegment, cannyThreshold1Segmented, cannyThreshold2Segmented);

    // Display the image with Canny edges
    imshow("Canny Edges Segmented", cannyEdgesAfterSegment);
}

// Callback function for the trackbar of the playersOnly image
void onTrackbarplayersOnly(int, void*) {
    // Apply Canny edge detection to the original image
    Mat cannyEdgesplayersOnly;
    cvtColor(playersOnlyImage, cannyEdgesplayersOnly, COLOR_BGR2GRAY);
    GaussianBlur(cannyEdgesplayersOnly, cannyEdgesplayersOnly, Size(5, 5), 1.5);
    Canny(cannyEdgesplayersOnly, cannyEdgesplayersOnly, cannyThreshold1playersOnly, cannyThreshold2playersOnly);

    // Display the image with Canny edges
    imshow("Canny Edges playersOnly", cannyEdgesplayersOnly);
}

int main() {
    // Load the specific image (im1.jpg) and process it
    std::string imagePath = "Images/im15.jpg";  // Path to your image
    image = imread(imagePath);
    if (image.empty()) {
        std::cerr << "Image not found: " << imagePath << std::endl;
        return -1;
    }

    // Convert the image to HSV color space
    Mat hsvImage;
    cvtColor(image, hsvImage, COLOR_BGR2HSV);
    
    Scalar lowerField(30, 75, 100);
    Scalar upperField(43, 255, 255);
    Scalar lowerTeamAShirt(95, 38, 50);
    Scalar upperTeamAShirt(140, 255, 255);
    Scalar lowerTeamAShort(95, 38, 50);
    Scalar upperTeamAShort(140, 255, 255);
    Scalar lowerTeamASocks(95, 38, 50);
    Scalar upperTeamASocks(140, 255, 255);
    Scalar lowerTeamBShirt(0, 0, 90);
    Scalar upperTeamBShirt(180, 30, 255);
    Scalar lowerTeamBShort(0, 0, 0);
    Scalar upperTeamBShort(180, 255, 55);
    Scalar lowerTeamBSocks(0, 0, 90);
    Scalar upperTeamBSocks(180, 30, 255);

    // Create output channels for Team A and Team B
    Mat teamAOutput = Mat::zeros(image.size(), image.type());
    Mat teamBOutput = Mat::zeros(image.size(), image.type());

    // Create masks for different segments
    Mat fieldMask, teamAMask, teamBMask;
    inRange(hsvImage, lowerField, upperField, fieldMask);
    inRange(hsvImage, lowerTeamAShirt, upperTeamAShirt, teamAMask);
    teamAOutput.setTo(Scalar(0, 0, 255), teamAMask);
    inRange(hsvImage, lowerTeamAShort, upperTeamAShort, teamAMask);
    teamAOutput.setTo(Scalar(0, 0, 255), teamAMask);
    inRange(hsvImage, lowerTeamASocks, upperTeamASocks, teamAMask);
    teamAOutput.setTo(Scalar(0, 0, 255), teamAMask);
    inRange(hsvImage, lowerTeamBShirt, upperTeamBShirt, teamBMask);
    teamBOutput.setTo(Scalar(255, 0, 0), teamBMask);
    inRange(hsvImage, lowerTeamBShort, upperTeamBShort, teamBMask);
    teamBOutput.setTo(Scalar(255, 0, 0), teamBMask);
    inRange(hsvImage, lowerTeamBSocks, upperTeamBSocks, teamBMask);
    teamBOutput.setTo(Scalar(255, 0, 0), teamBMask);

    // Combine the team outputs and the field mask
    segmentedImage = teamAOutput + teamBOutput;
    segmentedImage.setTo(Scalar(0, 255, 0), fieldMask);

    // Create a white border around each team's segment
    int borderSize = 5;  // Adjust the border size as needed
    Mat teamADilated, teamBDilated;
    dilate(teamAOutput, teamADilated, Mat(), Point(-1, -1), borderSize);
    dilate(teamBOutput, teamBDilated, Mat(), Point(-1, -1), borderSize);

    Mat teamABorder = teamADilated - teamAOutput;
    Mat teamBBorder = teamBDilated - teamBOutput;

    teamAOutput.setTo(Scalar(255, 255, 255), teamABorder);
    teamBOutput.setTo(Scalar(255, 255, 255), teamBBorder);

    segmentedImage = teamAOutput + teamBOutput;
    segmentedImage.setTo(Scalar(0, 255, 0), fieldMask);

    // Invert the field mask to get the player mask
    bitwise_not(fieldMask, playerMask);
    image.copyTo(playersOnlyImage, playerMask);
    
    // Create windows for displaying images
    namedWindow("Canny Edges Original", WINDOW_NORMAL);
    namedWindow("Canny Edges Segmented", WINDOW_NORMAL);
    namedWindow("Canny Edges playersOnly", WINDOW_NORMAL);

    // Create trackbars for adjusting Canny thresholds of the original image
    createTrackbar("Threshold 1", "Canny Edges Original", &cannyThreshold1Original, 255, onTrackbarOriginal);
    createTrackbar("Threshold 2", "Canny Edges Original", &cannyThreshold2Original, 255, onTrackbarOriginal);

    // Create trackbars for adjusting Canny thresholds of the segmented image
    createTrackbar("Threshold 1", "Canny Edges Segmented", &cannyThreshold1Segmented, 255, onTrackbarSegmented);
    createTrackbar("Threshold 2", "Canny Edges Segmented", &cannyThreshold2Segmented, 255, onTrackbarSegmented);

    // Create trackbars for adjusting Canny thresholds of the playersOnly image
    createTrackbar("Threshold 1", "Canny Edges playersOnly", &cannyThreshold1playersOnly, 255, onTrackbarplayersOnly);
    createTrackbar("Threshold 2", "Canny Edges playersOnly", &cannyThreshold2playersOnly, 255, onTrackbarplayersOnly);
    
    // Initial call to the callback functions to display the initial Canny edges
    onTrackbarOriginal(0, 0);
    onTrackbarSegmented(0, 0);
    onTrackbarplayersOnly(0, 0);

    waitKey(0);

    return 0;
}
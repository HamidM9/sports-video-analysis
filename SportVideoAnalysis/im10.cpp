#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <cerrno>

using namespace cv;

const double MIN_PLAYER_AREA = 1000; // Adjust as needed
const int MIN_CONTOUR_POINTS = 50;   // Adjust as needed

int im10() {
    // Load the specific image (im1.jpg) and process it
    std::string imagePath = "Images/im10.jpg";  // Path to your image
    Mat image = imread(imagePath);
    if (image.empty()) {
        std::cerr << "Image not found: " << imagePath << std::endl;
        return -1;
    }
    
    // Extract the number from the image name
    std::string imageName = imagePath.substr(imagePath.find_last_of('/') + 1); // Extract the filename
    int number = 0; // Initialize with a default value
    sscanf(imageName.c_str(), "im%d.jpg", &number); // Assuming the image name is in the format "imX.jpg"
    
    // Create the result directory based on the extracted number
    std::string resultDirName = "Result" + std::to_string(number);
    const char* resultDir = resultDirName.c_str();
    int dirStatus = mkdir(resultDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (dirStatus != 0 && errno != EEXIST) {
        std::cerr << "Failed to create '" << resultDirName << "' directory" << std::endl;
        return -1;
    }

    // Convert the image to HSV color space
    Mat hsvImage;
    cvtColor(image, hsvImage, COLOR_BGR2HSV);

    Scalar lowerField(0, 0, 130);
    Scalar upperField(180, 39, 255);
    Scalar lowerTeamAShirt(110, 50, 0);
    Scalar upperTeamAShirt(150, 255, 255);
    Scalar lowerTeamAShort(110, 50, 0);
    Scalar upperTeamAShort(150, 255, 255);
    Scalar lowerTeamASocks(110, 50, 0);
    Scalar upperTeamASocks(150, 255, 255);
    Scalar lowerTeamBShirt(23, 100, 150);
    Scalar upperTeamBShirt(35, 255, 255);
    Scalar lowerTeamBShort(113, 160, 132);
    Scalar upperTeamBShort(120, 190, 255);
    Scalar lowerTeamBSocks(23, 100, 150);
    Scalar upperTeamBSocks(35, 255, 255);

    // Create output channels for Team A and Team B
    Mat teamAOutput = Mat::zeros(image.size(), image.type());
    Mat teamBOutput = Mat::zeros(image.size(), image.type());

    Mat teamAMaskOutput = Mat::zeros(hsvImage.size(), CV_8UC1);
    Mat teamBMaskOutput = Mat::zeros(hsvImage.size(), CV_8UC1);

    // Create masks for different segments
    Mat fieldMask, teamAMask, teamBMask;
    inRange(hsvImage, lowerField, upperField, fieldMask);
    inRange(hsvImage, lowerTeamAShirt, upperTeamAShirt, teamAMask);
    teamAMaskOutput |= teamAMask;
    teamAOutput.setTo(Scalar(0, 0, 255), teamAMask);
    inRange(hsvImage, lowerTeamAShort, upperTeamAShort, teamAMask);
    teamAMaskOutput |= teamAMask;
    teamAOutput.setTo(Scalar(0, 0, 255), teamAMask);
    inRange(hsvImage, lowerTeamASocks, upperTeamASocks, teamAMask);
    teamAMaskOutput |= teamAMask;
    teamAOutput.setTo(Scalar(0, 0, 255), teamAMask);
    inRange(hsvImage, lowerTeamBShirt, upperTeamBShirt, teamBMask);
    teamBMaskOutput |= teamBMask;
    teamBOutput.setTo(Scalar(255, 0, 0), teamBMask);
    inRange(hsvImage, lowerTeamBShort, upperTeamBShort, teamBMask);
    teamBMaskOutput |= teamBMask;
    teamBOutput.setTo(Scalar(255, 0, 0), teamBMask);
    inRange(hsvImage, lowerTeamBSocks, upperTeamBSocks, teamBMask);
    teamBMaskOutput |= teamBMask;
    teamBOutput.setTo(Scalar(255, 0, 0), teamBMask);

    // Combine the team outputs and the field mask
    Mat segmentedImage = teamAOutput + teamBOutput;
    segmentedImage.setTo(Scalar(0, 255, 0), fieldMask);

        // Mask non-blue, non-red, and non-green areas in the original image
    Mat backgroundMask = segmentedImage.clone();
    
    // Iterate through each pixel
    for (int y = 0; y < backgroundMask.rows; y++) {
        for (int x = 0; x < backgroundMask.cols; x++) {
            cv::Vec3b pixel = backgroundMask.at<cv::Vec3b>(y, x);

            // Check if the pixel is black (all channels are 0)
            if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
                // Set black pixels to yellow (255, 255, 0)
                backgroundMask.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 0);
            } else {
                // Set other color pixels to black (0, 0, 0)
                backgroundMask.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
            }
        }
    }

    // Create a copy of the original image to SegmentedBackground
    Mat SegmentedBackground = image.clone();
    // Apply the fieldMask to the copy
    SegmentedBackground.setTo(Scalar(0, 0, 0), ~backgroundMask); // Invert the mask to zero out the non-field region
    std::string SegmentedBackgroundPath = resultDirName + "/SegmentedBackground.jpg";
    cv::imwrite(SegmentedBackgroundPath, SegmentedBackground);

        // Create a copy of the original image to SegmentedField
    Mat SegmentedField = image.clone();
    // Apply the fieldMask to the copy
    SegmentedField.setTo(Scalar(0, 0, 0), ~fieldMask); // Invert the mask to zero out the non-field region
    std::string SegmentedFieldPath = resultDirName + "/SegmentedField.jpg";
    cv::imwrite(SegmentedFieldPath, SegmentedField);

    // Create a copy of the original image to SegmentedTeamA
    Mat SegmentedTeamA = image.clone();
    // Apply the teamAOutput to the copy
    SegmentedTeamA.setTo(Scalar(0, 0, 0), ~teamAMaskOutput); // Invert the mask to zero out the non-teamA region
    std::string SegmentedTeamAPath = resultDirName + "/SegmentedTeamA.jpg";
    cv::imwrite(SegmentedTeamAPath, SegmentedTeamA);

    // Create a copy of the original image to SegmentedTeamB
    Mat SegmentedTeamB = image.clone();
    // Apply the teamBOutput to the copy
    SegmentedTeamB.setTo(Scalar(0, 0, 0), ~teamBMaskOutput); // Invert the mask to zero out the non-teamB region
    std::string SegmentedTeamBPath = resultDirName + "/SegmentedTeamB.jpg";
    cv::imwrite(SegmentedTeamBPath, SegmentedTeamB);

    // Apply Canny edge detection to the original image
    Mat cannyEdgesBeforeSegment;
    cvtColor(image, cannyEdgesBeforeSegment, COLOR_BGR2GRAY);
    GaussianBlur(cannyEdgesBeforeSegment, cannyEdgesBeforeSegment, Size(5, 5), 1.5);
    Canny(cannyEdgesBeforeSegment, cannyEdgesBeforeSegment, 0, 38);

    // Apply Canny edge detection to player segments
    Mat cannyEdgesAfterSegment;
    cvtColor(segmentedImage, cannyEdgesAfterSegment, COLOR_BGR2GRAY);
    GaussianBlur(cannyEdgesAfterSegment, cannyEdgesAfterSegment, Size(5, 5), 1.5);
    Canny(cannyEdgesAfterSegment, cannyEdgesAfterSegment, 0, 95);

    // Merge Canny edges of the original image and segmented image
    Mat mergedCannyEdges = cannyEdgesBeforeSegment + cannyEdgesAfterSegment;

    // Display the merged Canny edges
    namedWindow("Merged Canny Edges", WINDOW_NORMAL);
    imshow("Merged Canny Edges", mergedCannyEdges);

    // Display the images in separate windows
    namedWindow("Original Image", WINDOW_NORMAL);
    imshow("Original Image", image);

    namedWindow("Canny Edges Before Segment", WINDOW_NORMAL);
    imshow("Canny Edges Before Segment", cannyEdgesBeforeSegment);

    namedWindow("Segmented Image", WINDOW_NORMAL);
    imshow("Segmented Image", segmentedImage);
    std::string segmentedImagePath = resultDirName + "/segmentedImage.jpg";
    cv::imwrite(segmentedImagePath, segmentedImage);

    namedWindow("Canny Edges After Segment", WINDOW_NORMAL);
    imshow("Canny Edges After Segment", cannyEdgesAfterSegment);
    std::string cannyEdgesAfterSegmentPath = resultDirName + "/cannyEdgesAfterSegment.jpg";
    cv::imwrite(cannyEdgesAfterSegmentPath, cannyEdgesAfterSegment);

    // Invert the field mask to get the player mask
    Mat playerMask;
    bitwise_not(fieldMask, playerMask);

    // Apply the player mask to the original image to remove the playing field
    Mat playersOnlyImage;
    image.copyTo(playersOnlyImage, playerMask);

    namedWindow("Players Only Image", WINDOW_NORMAL);
    imshow("Players Only Image", playersOnlyImage);
    std::string playersOnlyImagePath = resultDirName + "/playersOnlyImage.jpg";
    cv::imwrite(playersOnlyImagePath, playersOnlyImage);

    // Overlay merged Canny edges on the original image
    Mat overlaidImage = image.clone();
    overlaidImage.setTo(Scalar(0, 255, 0), cannyEdgesAfterSegment);

    // Display the overlaid image
    namedWindow("Overlaid Image", WINDOW_NORMAL);
    imshow("Overlaid Image", overlaidImage);
    std::string overlaidImagePath = resultDirName + "/overlaidImage.jpg";
    cv::imwrite(overlaidImagePath, overlaidImage);

    // Overlay merged Canny edges on the original image
    Mat overlaidImageSeg = playersOnlyImage.clone();
    overlaidImageSeg.setTo(Scalar(0, 255, 0), cannyEdgesAfterSegment);

    // Display the overlaid image
    namedWindow("Overlaid Image Seg", WINDOW_NORMAL);
    imshow("Overlaid Image Seg", overlaidImageSeg);
    std::string overlaidImageSegPath = resultDirName + "/overlaidImageSeg.jpg";
    cv::imwrite(overlaidImageSegPath, overlaidImageSeg);

    // Apply morphological operations to connect the edges
    Mat morphKernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    morphologyEx(cannyEdgesBeforeSegment, cannyEdgesBeforeSegment, MORPH_CLOSE, morphKernel);

    // Find contours in the connected edge map
    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    findContours(cannyEdgesBeforeSegment, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Filter out small or irregular contours
    std::vector<std::vector<Point>> filteredContours;
    for (const auto &contour : contours) {
        if (contourArea(contour) > MIN_PLAYER_AREA && contour.size() > MIN_CONTOUR_POINTS) {
            filteredContours.push_back(contour);
        }
    }

    // Draw bounding rectangles around the player regions
    Mat playerDetectionResult = playersOnlyImage.clone();
    for (const auto &contour : filteredContours) {
        Rect playerRect = boundingRect(contour); // Use a different name for the variable
        rectangle(playerDetectionResult, playerRect, Scalar(0, 255, 0), 2);
    }

    // Display the player detection result
    namedWindow("Player Detection Result", WINDOW_NORMAL);
    imshow("Player Detection Result", playerDetectionResult);
    std::string playerDetectionResultPath = resultDirName + "/playerDetectionResult.jpg";
    cv::imwrite(playerDetectionResultPath, playerDetectionResult);

    // Create an image for player detection result
    Mat originalImageWithBoundingBoxes = image.clone(); // Clone the original image

    Scalar boundingBoxColor; // Define a color for the bounding boxes (e.g., Scalar(0, 255, 0) for green)

    // Create and open the output text file
    std::string BoundingBoxesPath = resultDirName + "/bounding_boxes.txt";
    std::ofstream outputFile(BoundingBoxesPath);

    // Loop through each filtered contour (player region)
    for (const auto &contour : filteredContours) {
        Rect playerRect = boundingRect(contour); // Use a different name for the variable
            // Check if this bounding box is directly inside another bounding box
        bool isInsideAnotherBox = false;
        for (const auto &otherContour : filteredContours) {
            if (&contour != &otherContour) { // Avoid comparing the same contour
                Rect otherRect = boundingRect(otherContour);
                if (playerRect.x >= otherRect.x &&
                    playerRect.y >= otherRect.y &&
                    (playerRect.x + playerRect.width) <= (otherRect.x + otherRect.width) &&
                    (playerRect.y + playerRect.height) <= (otherRect.y + otherRect.height)) {
                    isInsideAnotherBox = true;
                    break; // No need to check further
                }
            }
        }
        if (!isInsideAnotherBox) {
            Mat playerRegion = segmentedImage(playerRect); // Extract the region of interest
            // Count red and blue pixels in the player region
            int redPixelCount = 0;
            int bluePixelCount = 0;

            for (int y = 0; y < playerRegion.rows; ++y) {
                for (int x = 0; x < playerRegion.cols; ++x) {
                    Vec3b pixel = playerRegion.at<Vec3b>(y, x);
                    int blueValue = pixel[0];
                    int greenValue = pixel[1];
                    int redValue = pixel[2];

                    if (redValue > blueValue && redValue > greenValue) {
                        redPixelCount++;
                    } else if (blueValue > redValue && blueValue > greenValue) {
                        bluePixelCount++;
                    }
                }
            }

            // Determine the team label and bounding box color
            std::string playerTeam;
            int TeamID;
            if (redPixelCount > bluePixelCount) {
                playerTeam = "TeamA";
                TeamID = 2; // TeamA ID
                boundingBoxColor = Scalar(0, 0, 255); // Red
            } else if (bluePixelCount > redPixelCount) {
                playerTeam = "TeamB";
                TeamID = 1; // TeamB ID
                boundingBoxColor = Scalar(255, 0, 0); // Blue
            }

            // Write the bounding box coordinates (x, y) and width and height (w, h) to the output file and TeamID
            outputFile << playerRect.x << " " << playerRect.y << " " << playerRect.width << " " << playerRect.height<< " " << TeamID << "\n";

            // Draw a semi-transparent filled rectangle over the player's bounding box
            Mat overlay = playerDetectionResult(playerRect).clone();
            rectangle(overlay, Point(0, 0), overlay.size(), boundingBoxColor, -1); // Filled rectangle
            addWeighted(overlay, 0.2, playerDetectionResult(playerRect), 0.8, 0, playerDetectionResult(playerRect));

            // Draw the bounding rectangle and label on the player detection result
            rectangle(playerDetectionResult, playerRect, boundingBoxColor, 2);
            putText(playerDetectionResult, playerTeam, Point(playerRect.x, playerRect.y - 10),
                    FONT_HERSHEY_SIMPLEX, 0.5, boundingBoxColor, 2);

            // Draw a semi-transparent filled rectangle over the player's bounding box on the original image
            overlay = originalImageWithBoundingBoxes(playerRect).clone();
            rectangle(overlay, Point(0, 0), overlay.size(), boundingBoxColor, -1); // Filled rectangle
            addWeighted(overlay, 0.2, originalImageWithBoundingBoxes(playerRect), 0.8, 0, originalImageWithBoundingBoxes(playerRect));

            // Draw the bounding rectangle on the original image
            rectangle(originalImageWithBoundingBoxes, playerRect, boundingBoxColor, 2);
            putText(originalImageWithBoundingBoxes, playerTeam, Point(playerRect.x, playerRect.y - 10),
                    FONT_HERSHEY_SIMPLEX, 0.5, boundingBoxColor, 2);
        }
    }

    // Display the player detection result with labels and overlays
    namedWindow("Player Detection Result with Labels", WINDOW_NORMAL);
    imshow("Player Detection Result with Labels", playerDetectionResult);
    std::string playerDetectionResultWithLabelsPath = resultDirName + "/playerDetectionResultWithLabels.jpg";
    cv::imwrite(playerDetectionResultWithLabelsPath, playerDetectionResult);

    // Display the original image with bounding boxes, labels, and overlays
    namedWindow("Original Image with Bounding Boxes and Labels", WINDOW_NORMAL);
    imshow("Original Image with Bounding Boxes and Labels", originalImageWithBoundingBoxes);
    std::string originalImageWithBoundingBoxesPath = resultDirName + "/originalImageWithBoundingBoxes.jpg";
    cv::imwrite(originalImageWithBoundingBoxesPath, originalImageWithBoundingBoxes);
        
    waitKey(0);

    return 0;
}
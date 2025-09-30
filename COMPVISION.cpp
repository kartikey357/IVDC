#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

// Define the region of interest as a trapezoid based on image dimensions
Mat region_of_interest(const Mat& image) {
    // Define a mask of the same size as the image
    Mat mask = Mat::zeros(image.size(), CV_8UC1);

    // Get image dimensions
    int height = image.rows;
    int width = image.cols;

    // Define the vertices of the trapezoid for the ROI.
    // These coordinates are normalized and scaled for a typical road scene.
    Point vertices[4];
    vertices[0] = Point(width * 0.45, height * 0.65); // Top-left
    vertices[1] = Point(width * 0.55, height * 0.65); // Top-right
    vertices[2] = Point(width * 1.0, height * 1.0);  // Bottom-right
    vertices[3] = Point(width * 0.0, height * 1.0);  // Bottom-left

    const Point* ppt[1] = { vertices };
    int npt[] = { 4 };

    // Fill the polygonal region in the mask with white (255)
    fillPoly(mask, ppt, npt, 1, Scalar(255));

    // Apply the mask to the input image (bitwise AND operation)
    Mat masked_image;
    bitwise_and(image, image, masked_image, mask);

    return masked_image;
}

// Fit lines to the lane markings and extrapolate them
void fit_and_extrapolate_lanes(const Mat& image, const vector<Vec4i>& lines, Mat& line_image, vector<Point>& road_polygon_points) {
    
    // Separate lines into left and right lanes based on slope
    vector<Point> left_lane_points;
    vector<Point> right_lane_points;

    // The horizontal center of the ROI helps distinguish left/right lines
    int center_x = image.cols / 2;

    for (const auto& line : lines) {
        // Line coordinates: (x1, y1, x2, y2)
        int x1 = line[0];
        int y1 = line[1];
        int x2 = line[2];
        int y2 = line[3];

        // Calculate slope (to avoid division by zero in vertical lines)
        if (x2 - x1 == 0) continue;
        double slope = (double)(y2 - y1) / (x2 - x1);

        // Filter out near-horizontal or near-vertical lines
        if (abs(slope) < 0.3 || abs(slope) > 0.8) continue;

        // Group lines: Negative slope is typically the left lane (top-left to bottom-right)
        // Positive slope is typically the right lane (top-right to bottom-left)
        if (slope < 0 && x1 < center_x && x2 < center_x) {
            left_lane_points.push_back(Point(x1, y1));
            left_lane_points.push_back(Point(x2, y2));
        } else if (slope > 0 && x1 > center_x && x2 > center_x) {
            right_lane_points.push_back(Point(x1, y1));
            right_lane_points.push_back(Point(x2, y2));
        }
    }

    // Extrapolate and draw the lines (Linear Regression)
    // The final lane points will be stored here: [pt_bottom, pt_top]
    Point left_line[2], right_line[2];
    int height = image.rows;
    int y_min = height * 0.65; // Upper limit of the line
    int y_max = height;        // Bottom limit of the line (bottom of the image)

    // Helper function to fit and extrapolate
    auto fit_line = [&](const vector<Point>& points, Point line_pts[2]) {
        if (points.size() > 0) {
            Vec4f line_params;
            fitLine(points, line_params, DIST_L2, 0, 0.01, 0.01);

            double vx = line_params[0]; // Unit vector x-component
            double vy = line_params[1]; // Unit vector y-component
            double x0 = line_params[2]; // Point on the line x-coord
            double y0 = line_params[3]; // Point on the line y-coord

            // Calculate x-coordinates for y_min and y_max
            line_pts[0].y = y_max; // Bottom point
            line_pts[0].x = cvRound(((line_pts[0].y - y0) * vx / vy) + x0);
            
            line_pts[1].y = y_min; // Top point
            line_pts[1].x = cvRound(((line_pts[1].y - y0) * vx / vy) + x0);

            // Draw the line on the separate line image
            line(line_image, line_pts[0], line_pts[1], Scalar(0, 255, 0), 5);
        }
    };

    // Fit left and right lanes
    fit_line(left_lane_points, left_line);
    fit_line(right_lane_points, right_line);

    // Build the road polygon points:
    // Left bottom -> Left top -> Right top -> Right bottom
    road_polygon_points.push_back(left_line[0]);
    road_polygon_points.push_back(left_line[1]);
    road_polygon_points.push_back(right_line[1]);
    road_polygon_points.push_back(right_line[0]);
}

// Main function for the lane isolation pipeline
Mat lane_isolation_pipeline(const Mat& input_img) {
    // 1. Preprocessing (Grayscale and Blur)
    Mat gray, blurred;
    cvtColor(input_img, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(5, 5), 0);

    // 2. Canny Edge Detection
    Mat edges;
    Canny(blurred, edges, 50, 150); // Tune thresholds as needed

    // 3. Region of Interest (ROI) Selection
    Mat roi_edges = region_of_interest(edges);

    // 4. Hough Transform
    vector<Vec4i> lines;
    // Probabilistic Hough Transform is less computationally intensive and gives line segments
    HoughLinesP(roi_edges, lines, 1, CV_PI / 180, 50, 40, 5);

    // Prepare an image to draw the detected lines (for visualization/debugging)
    Mat line_image = Mat::zeros(input_img.size(), CV_8UC3);
    
    // Vector to store the four points of the final road polygon
    vector<Point> road_polygon_points;

    // 5. Lane Fitting and Extrapolation
    fit_and_extrapolate_lanes(input_img, lines, line_image, road_polygon_points);

    // 6. Binary Mask Generation (The core requirement)
    // Create a new image initialized to WHITE (CV_8UC1 for binary)
    Mat binary_mask = Mat(input_img.size(), CV_8UC1, Scalar(255));

    if (road_polygon_points.size() == 4) {
        // Define the polygon for the road area
        const Point* ppt[1] = { road_polygon_points.data() };
        int npt[] = { (int)road_polygon_points.size() };

        // Fill the road polygon area with BLACK (0)
        // Road area (between boundaries) = BLACK
        // Surrounding regions (background) = WHITE
        fillPoly(binary_mask, ppt, npt, 1, Scalar(0));
    } else {
        cerr << "Could not determine valid lane lines for polygon filling. Returning white mask." << endl;
    }

    // Return the final binary mask
    return binary_mask;
}

int main() {
    // NOTE: Replace "road_image.jpg" with the actual path to your input image.
    // Ensure the image path is correct relative to the executable.
    Mat input_image = imread("road_image.jpg");

    if (input_image.empty()) {
        cerr << "Error: Could not read the image. Please ensure 'road_image.jpg' exists and is a valid image file." << endl;
        // Create a placeholder image to continue the simulation
        input_image = Mat(540, 960, CV_8UC3, Scalar(100, 100, 100)); 
        putText(input_image, "PLACEHOLDER: Load your image here", Point(100, 270), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    }

    // Run the pipeline
    Mat isolated_mask = lane_isolation_pipeline(input_image);

    // Display results
    namedWindow("1. Input Image", WINDOW_AUTOSIZE);
    imshow("1. Input Image", input_image);

    namedWindow("2. Lane Isolated Binary Mask", WINDOW_AUTOSIZE);
    imshow("2. Lane Isolated Binary Mask", isolated_mask);

    // Wait for a key press
    waitKey(0);

    return 0;
}

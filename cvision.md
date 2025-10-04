		  Computer Vision
QnA

1.	Path-finding algorithms are used to determine the optimal route between two points in a space, often represented as a graph or grid. These algorithms are
2.	 crucial in autonomous navigation, robotics, and game development.
Example: A* (A-Star) Algorithm
A* is a best-first search algorithm that finds the shortest path using both actual cost and heuristic estimates.

______________________________________________________________________________________________________________________________________________________________


2.  Challenges in Poor Weather:
- Rain, fog, snow obscure lane markings.
- Reflections and low contrast reduce visibility.
- Cameras may blur or misinterpret features.
Why It's Not Enough:
- Lane detection relies heavily on visual cues.
- In poor weather, these cues degrade, leading to unsafe decisions.
🛠️ Solutions to Overcome This:
1-Sensor Fusion:
- Combine camera with LiDAR, radar, and IMU.
- LiDAR detects road edges even in fog.
- Radar penetrates rain and snow.
2-Semantic Segmentation:
- Use deep learning to classify road regions.
- Works better than edge detection in noisy conditions.
3-Map-Based Localization:
- Use HD maps and GPS to infer lane positions.
- Helps when visual input is unreliable.
4-Temporal Filtering:
- Use past frames to predict current lane position.
- Kalman filters or particle filters smooth noisy data.
5-Thermal Cameras:
- Detect heat signatures of vehicles and road boundaries.
- Useful in low-light or foggy conditions.

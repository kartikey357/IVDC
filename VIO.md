	     Visual-Inertial Odometry (VIO)
📍 Question 1: GPS-Denied Tunnel Scenario
a. How does the VIO system propagate state using only IMU?
- When the camera is blinded (e.g., fog), the system relies solely on the Inertial Measurement Unit (IMU).
- The IMU provides linear acceleration and angular velocity at high frequency.
- These measurements are integrated over time using motion models (e.g., constant velocity or acceleration) to estimate:
- Position
- Velocity
- Orientation
- This process is known as dead reckoning, typically handled by an Extended Kalman Filter (EKF) or factor graph optimization.
b. What happens to uncertainty and drift?
- IMU-only propagation leads to accumulated error due to sensor noise and bias.
- Uncertainty increases over time, especially in position and yaw.
- Drift occurs because small errors compound without visual corrections.
- The system’s confidence in its state estimate decreases, reflected in a growing covariance matrix.
c. When vision returns, how does the system re-align to correct accumulated drift?
- Visual features are re-detected and matched to previous keyframes.
- The system performs loop closure or re-localization using:
- Bundle adjustment
- Pose graph optimization
- This corrects accumulated drift by aligning the current estimate with known landmarks.
- The filter’s uncertainty is reduced, and the trajectory is updated to reflect corrected pose.
d. Explore the libraries used in VIO
⦁	VINS-Mono - Nonlinear optimization-based monocular VIO with loop closure and relocalization.
⦁	ROVIO - EKF-based VIO using direct photometric error minimization.
⦁	OKVIS - Keyframe-based VIO supporting stereo and monocular input.
⦁	ORB-SLAM3 - SLAM system with VIO support, loop closure, and map reuse.

____________________________________________________________________________________________________________________________________


Question 2: Scale Ambiguity in Indoor Drone VIO
a. How does the system resolve the scale ambiguity using the IMU?
- Monocular vision lacks absolute scale (e.g., 1m vs 10m).
- IMU provides metric acceleration, allowing the system to infer real-world motion scale.
- By fusing IMU data with visual motion (e.g., feature displacement), the system estimates scale.
- This fusion is done via optimization or filtering, aligning visual and inertial trajectories.
b. In what kinds of drone motions does the system fail to figure out the scale?
- Pure rotational motion: No translational movement means scale cannot be inferred.
- Constant velocity with no acceleration: IMU lacks excitation to estimate scale.
- Hovering or slow drift: Minimal motion leads to poor observability of scale.
🧠 How to detect and deal with it:
- Monitor observability metrics or covariance growth.
- Inject deliberate motion (e.g., small accelerations) to excite the system.
- Use external cues (e.g., barometer, altimeter) or switch to stereo vision if available.
c. How do popular libraries deal with scale drift?
⦁	VINS-Mono - Tightly coupled optimization with IMU to maintain scale consistency. 
⦁	ROVIO - EKF tracks scale uncertainty and corrects using photometric error.
⦁	ORB-SLAM3 - Performs loop closure and map reuse to correct scale drift.



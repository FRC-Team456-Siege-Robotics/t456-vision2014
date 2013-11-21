Training and Tutorial Examples
------------------------------

<td>Folders</td>
* hello - Hello world code example
* simple_camera - Display image frames from webcam (python)
* filter - Grab image from camera and filter
* threshold - Threshold target pixels from camera image
* contours - Extract contour lines from around target objects and select target
* kinect - Sample code to connect to Kinect and make a depth map movie

<td>Image processing steps</td>
1. Grab image from camera
2. Split colors (red, green, blue) in image into HSV (hue, sat, value)
3. Threshold (seperate) target colors from everything else
4. Identify target shapes
5. Calculate target coordinate and distance
6. Send information to webservice 

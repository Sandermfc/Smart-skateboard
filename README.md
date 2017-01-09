******************************************************************************************************
******************************************************************************************************
*************************************Smart-skateboard*************************************************
******************************************************************************************************
******************************************************************************************************

This project aims to strap a gyroscope / accelerometer (MPU6050) to a skateboard to take in acceleration / orientation data.
This data is then used to calculate things like distance travelled in different axes (height for example), orientation, previous orientations, speed, etc.
This data is used to be able to detect different kinds of tricks (an ollie for example) and attribute a score based on the data recieved. For example, an ollie that achieves a height of 1.3m would get a better score than one which only gets 1.1m .

To go into more detail ..
******************************************************************************************************
We run our application through a loop which:
  - reads new data.
  - Executes the next tester functor(s) in the decision tree.
  - If success, continue in the decision tree.
  - If failure, loop back and test the same function (up to a certain limit so as not to get stuck).
  - If we get to the bottom of the tree, attribute a score and restart at the root.
  
All of the output for our program is sent over bluetooth (simple ASCII) to be read by any bluetooth capable device (a smartphone for example).

In the future ..
******************************************************************************************************

In the future, the goal would be to implement a machine learning algorithm, then "train" the board on what would be considered "good" tricks. The board would then be able to decide for itself what margins of success to use and what tricks are better than others to be able to better attribute scores.

Currently, all of these margins have been hand tested and are hard coded as constants.

#include "pullDataXG.h"
#include <math.h>

int offSetPitch=0; // starting point for yaw pitch roll
int offSetYaw=0;
int offSetRoll=0;
float offSetX=0;
float offSetY=0;
float offSetZ=0;
int counter = 0; // do functions this number of times unti give true

//Baseline orientation (not the same as offset, which is to calibrate at a universal 0 point)
//These values are used in initialise() to determine if we start on an incline.
int baseYaw=0;
int basePitch=0;
int baseRoll=0;

float prevHeight = 0;
float prevVelocity = 0;
unsigned long prevTime = 0;
acceleration prevAccel;

float maxHeight = 0;
double currentHeight = 0;
const int frontLiftAngle = 35;

//pullDataXG.h deals with the accelerometer/gyro pins
float calculateVelocity();
float addToHeight();
class state
{
  public:
  orientation myOrientation;  //xyz coordinates
  acceleration myAcceleration;
  float velocity; //velocity (speed) in the y  (upward) axis only
  unsigned long t;
  
  bool getData()
  {
    //get the current time
    //Read the orientation and accelerations on xyz axes
    allData myData = pullStuff(); //yaw, pitch, roll, xyz accelerations TODOOOO
    if(myData.o.yaw == 0 && myData.o.pitch == 0 && myData.o.roll == 0 && myData.a.x == 0 && myData.a.y == 0 && myData.a.z == 0)
    {
      Serial.print(F("got an error, returning false"));
      return false;
    }
    myOrientation = myData.o;
    myAcceleration = myData.a;
    //myAcceleration.x-=offSetX;
    //myAcceleration.y-=offSetY;
    //map acceleration to 2G interval (-19.6 to +19.6)
    myAcceleration.x/= 1671.8367346939;
    myAcceleration.y/= 1671.8367346939;
    myAcceleration.z/= 1671.8367346939;


    //Serial.print(F("myAcceleration.z = "));
    //Serial.println(myAcceleration.z);
    //Serial.print(F("After offset = "));
    myAcceleration.z-=offSetZ;
    Serial.print(F("Zaccel = "));
    Serial.println(myAcceleration.z);
    //Serial.println(myAcceleration.z);
    //Serial.print(F("With an offset of "));
    //Serial.print(offSetZ);

    //Serial.print("x ");
    //Serial.println(myAcceleration.x);
    //Serial.print("y ");
    //Serial.println(myAcceleration.y);
    //Serial.print("z ");
    //Serial.println(myAcceleration.z);
    //Serial.print("yaw ");
    //Serial.println(myOrientation.yaw);
    Serial.print("pitch ");
    Serial.println(myOrientation.pitch);
    /*Serial.print("roll ");
    Serial.println(myOrientation.roll);
    Serial.print("x ");
    Serial.println(myAcceleration.x);
    Serial.print("y ");
    Serial.println(myAcceleration.y);
    Serial.print("z ");
    Serial.println(myAcceleration.z);*/

    //at instant 0, previous speed is set to 0 by the initialise function
    //using previous speed, time between the last reading and this one, and the upward acceleration, find the current speed.
    //vitesseFinale = vitesseInitiale + acceleration*temps
    t = micros();
    currentHeight += addToHeight();
    //Serial.println(t);
    velocity = calculateVelocity();
    Serial.print("current Height ");
    Serial.println(currentHeight);
    /*Serial.print("velociratpor ");
    Serial.println(velocity);*/
    
    //Set the prev**** variables for the next time around
    prevTime = t;
    prevVelocity = velocity;
    prevAccel = myAcceleration;
    return true;
  }
  /*int calculateHeight(int ultraSoundNum) // math function to calculate max height
  {
    return ultraSound[ultraSoundNum]*sqrt(abs(1 - pow(sin(myOrientation.pitch - offSetPitch - basePitch), 2) - pow(sin(myOrientation.roll - offSetRoll - baseRoll), 2)));
  }*/
}data;

float calculateVelocity()
{
  //Serial.println("CALCULATE VEL FUNC ");
  if((data.myAcceleration.z < 0.15 && data.myAcceleration.z > -0.15) && (data.myOrientation.pitch < 20 && data.myOrientation.pitch > -20))
  {
    Serial.println("still");
    return 0.0;
  }
  
  float x = prevVelocity;
  //Serial.print(F("Velocity1 "));
  //Serial.println(x);
  double fdeltaT = (data.t - prevTime)*0.000004;
  //Serial.print("t 1 ");
  //Serial.println(data.t);
  //Serial.print("t 2 ");
  //Serial.println(prevTime);
  //Serial.print("fdeltaT ");
  //Serial.println(fdeltaT);
  x += (data.myAcceleration.z)*fdeltaT; //TODO make sure its in seconds
  //Serial.print(F("Velocity2 "));
  //Serial.print(x);
  return x;
}
float addToHeight()
{
  //calculate distance travelled over 1 tick, adds this value to currentHeight
  //distance = initialVelocity*time + 1/2(accelerationY)*time^2
  
  unsigned long deltaT = data.t - prevTime;
  /*Serial.print(data.t);
  Serial.print(F("-"));
  Serial.print(prevTime);
  Serial.print(F(" = "));
  Serial.print(deltaT);*/
  float fdeltaT = deltaT * 0.000004; //Make sure its in seconds
  //Serial.print(F("fdeltaT = "));
  //Serial.println(fdeltaT);
  float accelerationAverage = (data.myAcceleration.z + prevAccel.z)/2; //average of previous acceleration and current one
  //accelerationAverage -= offSetY;
  /*Serial.println(F("()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()"));
  Serial.print(prevVelocity);
  Serial.print(F("*"));
  Serial.print(fdeltaT);
  Serial.print(F(" + 0.5*"));
  Serial.print(accelerationAverage);
  Serial.print(F("*"));
  Serial.print(fdeltaT);
  Serial.print(F("^2 = "));*/
  float answer = prevVelocity*fdeltaT + 0.5*accelerationAverage*(fdeltaT*fdeltaT);
  //Serial.println(answer);
  return answer;
}

typedef bool(*functor)(void);

bool initialise()
{
  currentHeight = 0;
  maxHeight = 0;
  prevHeight=0;
  //these values are substracted from the angle found
  //this makes it so that when we start on an incline, that point becomes (0,0,0) until the end of the trick.
  baseYaw = data.myOrientation.yaw; //frontOfTable-1 is the most recently read value.
  basePitch = data.myOrientation.pitch;
  baseRoll = data.myOrientation.roll;
  //Get all the data at this instant into a struct
  data.getData();

  //Make sure that upwards acceleration is 0 before trying to check for tricks
  for(int i=0; i<10; i++)
  {
    data.getData();
    if(data.myAcceleration.z < 1 && data.myAcceleration.z > -1)
    {
      //we good
    }
    else
      return false;
  }
  return true;
}

bool front; //this represents the first "side" to be lifted, used in frontLift and backLift (see frontLift for utilisation)

bool frontLift()
{
  if(data.myOrientation.pitch > frontLiftAngle) //TODO
  {
    front = 0; //0 if angle is positive
    Serial.println("00000000000000000000000000000000000000000000000000000000000");
    return true;
  }
  else if(data.myOrientation.pitch <  -1*frontLiftAngle) //TODO
  {
    front = 1; //1 if angle is negative
    Serial.println("11111111111111111111111111111111111111111111111111111111111");
    return true;
  }
  return false;
}
bool backLift()
{
  if(currentHeight > 1)// TODO 
  {
    Serial.println(F("+++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
    return true;
  }
 return false;
}

bool getMaxHeight()
{
  if(prevHeight > currentHeight)
  {
    maxHeight = prevHeight;
    Serial.print("hfhabd ");
    Serial.println(maxHeight);
    return true;
  }
  else 
  {
    prevHeight = currentHeight;
  }
  return false;
}

bool landed() 
{
  const float marginAccel = 0.15;
  const float marginAngleP = 10.0;
  if((-1*marginAccel < data.myAcceleration.z  && data.myAcceleration.z < marginAccel) && (-1*marginAngleP < data.myOrientation.pitch) && (data.myOrientation.pitch < marginAngleP))
  { 
    Serial.print("//////////////////////////////////////////////////////");
    delay(2000);//TODO
    return true;
  }
  return false;
}

class node
{
  public:
  node* child[10];
  functor arr[10];
  int sizeOf;
  int sizeOfC;
  node(){
  }
  void node1() {
    sizeOf = 0;
    sizeOfC = 0;
  }
  void node2(functor f)
  {
    arr[sizeOf] = f;
    sizeOf++;
  }
  void pushChild(node* c)
  {
    child[sizeOfC] = c;
    sizeOfC++;
  }
  int test()
  {
    bool funcVal = false;
    for (int i = 0; i < sizeOf; i++)
    {
      funcVal = arr[i]();
      if (funcVal == true)
      {
        return i; //test i returned true;
      }
    }
    return -1; //none are true;
  }
  node* getChild(int childNumber)
  {
    if(childNumber < sizeOfC)
    {
      return child[childNumber];
    }
      
    else
    {
      return NULL;
    }
      
  }
}*root;

node* curr;

void setup() {
  //Baud and pin setup
  Serial.begin(115200); //print to serial monitor
  //Serial.println(F("1"));
  /*pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(pressIn1, INPUT);
  pinMode(pressIn2, INPUT);*/
  
  //create decision tree
  //root
  root = (node*) malloc (sizeof(*root)); //create the root node
  root->node1();
  root->node2(initialise);          //pass it the function pointers
  
  node* onReady = (node*) malloc (sizeof(*onReady));
  root->pushChild(onReady);
  onReady->node1();
  onReady->node2(frontLift); //pass it the function pointers
  
  //first side of the skateboard has been lifted
  node* oneLifted = (node*) malloc (sizeof(*oneLifted));//create oneLifted node
  onReady->pushChild(oneLifted); //0, point root to this node
  oneLifted->node1();
  oneLifted->node2(backLift); //pass it the function pointers

  node* bothLifted = (node*) malloc (sizeof(*bothLifted));//create oneLifted node
  oneLifted->pushChild(bothLifted);
  bothLifted->node1();
  bothLifted->node2(getMaxHeight);

  node* descending = (node*) malloc (sizeof(*descending));
  bothLifted->pushChild(descending);
  descending->node1();
  descending->node2(landed);
  
  //Serial.println(F("prepXG"));
  prepareXG(); //calls the initialise function in the pullDataXG.h header
  for(int i =0; i< 1000; i++) // do it a 100 times to stabilize the values
  {
    data.getData();
  }
  delay(2000); 
  data.getData();
  offSetPitch = data.myOrientation.pitch; // set offset
  offSetRoll = data.myOrientation.roll;
  offSetYaw = data.myOrientation.yaw;
  offSetX = data.myAcceleration.x;
  offSetY = data.myAcceleration.y;
  offSetZ = data.myAcceleration.z;
  curr = root;
  prevTime = micros(); //important for the very first acceleration calculation, otherwise prevTime = 0 and deltaT = 200s, distance becomes a lot and messes with future readings for a while
  prevVelocity = 0;
  prevAccel.x = prevAccel.y = prevAccel.z = 0;
}

void loop() {
  Serial.println("loop");
  //Get all the data at this instant into a struct
  data.getData();
  //Test all conditions
  int nextChild = curr->test();
  if(nextChild != -1)
  {
    counter = 0;
    curr = curr->getChild(nextChild);
    
    if(curr == NULL)
    {
      curr = root;
    }
    while(!data.getData()){Serial.print(F("Trying to get data again ..................................."));} //getData until there was not a buffer error
  }
  else
  {
    if(counter == 100) // do the functions 5 times unless they return true
    {
      counter = 0;
      Serial.print(F("FAILED"));
      //delay(2000);
      curr = root; //retourne au debut
    }
     else
      counter++;
  }
}



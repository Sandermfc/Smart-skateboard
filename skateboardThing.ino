#include "pullDataXG.h"
#include <math.h>

int offSetPitch; // starting point for yaw pitch roll
int offSetYaw;
int offSetRoll;
int counter = 0; // do functions this number of times unti give true

//Baseline orientation (not the same as offset, which is to calibrate at a universal 0 point)
//These values are used in initialise() to determine if we start on an incline.
int baseYaw=0;
int basePitch=0;
int baseRoll=0;

float maxHeight = 0;
float currentHeight = 0;
int frontLiftAngle = 40;

//pullDataXG.h deals with the accelerometer/gyro pins
int frontOfTable = 0; // newest value in table
float calculateVelocity(int pos);
float addToHeight(int pos);
class state
{
  public:
  //int ultraSound[2];
  orientation myOrientation;  //xyz coordinates
  acceleration myAcceleration;
  int absoluteHeight1;
  int absoluteHeight2;
  float velocity; //velocity (speed) in the y  (upward) axis only
  unsigned long t;
  //more stuff for gyroscope?

  void getData()
  {
    //get the current time
    t = micros();
    
    //Read the orientation and accelerations on xyz axes
    allData myData = pullStuff(); //yaw, pitch, roll, xyz accelerations
    myOrientation = myData.o;
    myAcceleration = myData.a;

    //at instant 0, previous speed is set to 0 by the initialise function
    //using previous speed, time between the last reading and this one, and the upward acceleration, find the current speed.
    //speed = (prevSpeed
    //vitesseFinale = vitesseInitiale + acceleration*temps
    currentHeight += addToHeight(frontOfTable - 1);
    velocity = calculateVelocity(frontOfTable-1);
    
    
  }
  /*int calculateHeight(int ultraSoundNum) // math function to calculate max height
  {
    return ultraSound[ultraSoundNum]*sqrt(abs(1 - pow(sin(myOrientation.pitch - offSetPitch - basePitch), 2) - pow(sin(myOrientation.roll - offSetRoll - baseRoll), 2)));
  }*/
};

const int numSavedStates = 6;
state states[numSavedStates];

float calculateVelocity(int pos)
{
   return states[(frontOfTable-2)%numSavedStates].velocity + states[pos].myAcceleration.y*((states[pos].t - states[(frontOfTable-2)%numSavedStates].t)*0.000004); //TODO make sure height accel is y
}
float addToHeight(int pos)
{
  //calculate distance travelled over 1 tick, adds this value to currentHeight
  //distance = initialVelocity*time + 1/2(accelerationY)*time^2
  unsigned long deltaT = states[pos].t - states[(frontOfTable-2)%numSavedStates].t;
  deltaT *= 0.000004;
  float accelerationAverage = (states[pos].myAcceleration.y + states[(frontOfTable-2)%numSavedStates].myAcceleration.y)/2; //average of previous acceleration and current one
  return states[(frontOfTable-2)%numSavedStates].velocity*deltaT + 0.5*accelerationAverage*(deltaT*deltaT);
}

//This helps get a baseline height
int baseHeight1;
int baseHeight2;

typedef bool(*functor)(void);

bool initialise()
{
  currentHeight = 0;
  maxHeight = 0;
  //these values are substracted from the angle found
  //this makes it so that when we start on an incline, that point becomes (0,0,0) until the end of the trick.
  baseYaw = states[(frontOfTable-1)%numSavedStates].myOrientation.yaw; //frontOfTable-1 is the most recently read value.
  basePitch = states[(frontOfTable-1)%numSavedStates].myOrientation.pitch;
  baseRoll = states[(frontOfTable-1)%numSavedStates].myOrientation.roll;
  //Get all the data at this instant into a struct
  state temp;
  temp.getData();
  //set the circular tables front to this state (and increment front)
  states[frontOfTable++] = temp;
  //set the front of the table
  frontOfTable%=numSavedStates;

  //Make sure that upwards acceleration is 0 before trying to check for tricks
  for(int i=0; i<10; i++)
  {
    temp.getData();
    if(temp.myAcceleration.y < 1 && temp.myAcceleration.y > -1)
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
  if(states[(frontOfTable-1)%numSavedStates].myOrientation.pitch > frontLiftAngle) //TODO
  {
    front = 0; //0 if angle is positive
    return true;
  }
  else if(states[(frontOfTable-1)%numSavedStates].myOrientation.pitch < 360 - frontLiftAngle) //TODO
  {
    front = 1; //1 if angle is negative
    return true;
  }
  return false;
}
bool backLift()
{
  if(!firstUltrasound)//if first to lift was 2 check abs height 1
  {
    if((states[(frontOfTable+1)%numSavedStates].absoluteHeight1+2) < (states[frontOfTable].absoluteHeight1-2)) // +-2 sensitivity
    {
      Serial.println(F("+++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
      return true;
    }
  }
  else
  {
    if((states[(frontOfTable+1)%numSavedStates].absoluteHeight2+2) < (states[frontOfTable].absoluteHeight2-2)) // +-2 sensitivity
    {
      Serial.println(F("+++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
      return true;
    }
  }
  return false;
}

bool getMaxHeight()
{
  
  return false;
}

bool landed() 
{
  if((states[frontOfTable].absoluteHeight1 -2 > baseHeight1 && states[frontOfTable].absoluteHeight1 +2 < baseHeight1) && ((states[frontOfTable].absoluteHeight2 -2 > baseHeight2 && states[frontOfTable].absoluteHeight2 +2 < baseHeight2))) // if skateboad is as close to the ground as when it satrted, we can say it landed
  {
    Serial.print("//////////////////////////////////////////////////////");
    delay(2000);
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

  /*node* descending = (node*) malloc (sizeof(*descending));
  bothLifted->pushChild(descending);
  descending->node1();
  descending->node2(landed);*/
  
  //Serial.println(F("prepXG"));
  prepareXG(); //calls the initialise function in the pullDataXG.h header
  state temp;
  for(int i =0; i< 100; i++) // do it a 100 times to stabilize the values
  {
    temp.getData();
  }
  delay(2000); 
  temp.getData();
  offSetPitch = temp.myOrientation.pitch; // set offset
  offSetRoll = temp.myOrientation.roll;
  offSetYaw = temp.myOrientation.yaw;
  curr = root;
}

void loop() {
  state temp;
  Serial.println("loop");
  //Get all the data at this instant into a struct
  temp.getData();
  //set the circular tables front to this state (and increment front)
  states[frontOfTable++] = temp;
  //set the front of the table
  frontOfTable%=numSavedStates;

  //Test all conditions
  int nextChild = curr->test();
  if(nextChild != -1)
  {
    curr = curr->getChild(nextChild);
    
    if(curr == NULL)
    {
      curr = root;
    }
    temp.getData();
    states[frontOfTable++] = temp;
    frontOfTable%=numSavedStates;
  }
  else
  {
    if(counter == 100) // do the functions 5 times unless they return true
    {
      counter = 0;
      curr = root; //retourne au debut
    }
     else
      counter++;
  }
}



#include "pullDataXG.h"
#include <math.h>

int offSetPitch; // starting point for yaw pitch roll
int offSetYaw;
int offSetRoll;
int counter = 0; // do functions this number of times unti give true

const int trigPin1 = 3; //ultrasound 1
const int echoPin1 = A3;
const int trigPin2 = 4; //ultrasound 2
const int echoPin2 = A4;

const int pressIn1 = A5;
const int pressIn2 = A6;

//Baseline orientation (not the same as offset, which is to calibrate at a universal 0 point)
//These values are used in initialise() to determine if we start on an incline.
int baseYaw=0;
int basePitch=0;
int baseRoll=0;

//pullDataXG.h deals with the accelerometer/gyro pins

class state
{
  public:
  int ultraSound[2];
  orientation myOrientation;  //xyz coordinates
  int absoluteHeight1;
  int absoluteHeight2;

  //more stuff for gyroscope?

  void getData()
  {
    //get pulse length of 1st ultrasound sensor and convert to distance
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin1, LOW);
    int duration = pulseIn(echoPin1, HIGH); //get raw value
    ultraSound[0] = duration/58.2;//calculate distance with the ultrasound raw value
    if(ultraSound[0] > 300) // if over 300, faulty value
    {
      ultraSound[0] = 0;
    }
    
    //get pulse length of 2nd ultrasound sensor and convert to distance
    digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin2, LOW);
    duration = pulseIn(echoPin2, HIGH);
    ultraSound[1] = duration/58.2;
    if(ultraSound[1] > 300)
    {
      ultraSound[1] = 0;
    }
    //Read the orientation (done directly after ultrasound as we want them to be IDEALLY read at the same time)
    myOrientation = pullStuff(); //yaw - pitch - roll

    absoluteHeight1 = calculateHeight(0);  
    absoluteHeight2 = calculateHeight(1);
    
  }
  int calculateHeight(int ultraSoundNum) // math function to calculate max height
  {
    return ultraSound[ultraSoundNum]*sqrt(abs(1 - pow(sin(myOrientation.pitch - offSetPitch - basePitch), 2) - pow(sin(myOrientation.roll - offSetRoll - baseRoll), 2)));
  }
};

const int numSavedStates = 6;
state states[numSavedStates];
int frontOfTable = 0; // newest value in table

//This helps to see which of the two sensors is triggered first (so which is the front/back)
bool firstUltrasound; //0 for the first one, 1 for the other

//This helps get a baseline height
int baseHeight1;
int baseHeight2;

int maxHeight = 0;

typedef bool(*functor)(void);

bool initialise()
{
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

  baseHeight1 = states[(frontOfTable-1)%numSavedStates].absoluteHeight1;
  baseHeight2 = states[(frontOfTable-1)%numSavedStates].absoluteHeight2;
  return true;
}

bool frontLift()
{
  bool flag1=false;
  bool flag2=false;
  //check if either of the ultrasound sensors increases abruptly
  if((states[(frontOfTable+1)%numSavedStates].absoluteHeight1)+10 < (states[frontOfTable].absoluteHeight1)-10){ // +-10 to make less sensitive
    flag1 = true;
    }
  if((states[(frontOfTable+1)%numSavedStates].absoluteHeight2)+10 < (states[frontOfTable].absoluteHeight2)-10){ // +-10 to make less sensitive
    flag2 = true;
  }
  if(!flag1 && !flag2) 
    return false;
  else //one lifted
  {
    if(flag1 && flag2) //both increased, then take the highest as front
    {
      if(states[(frontOfTable-1)%numSavedStates].absoluteHeight1 > states[(frontOfTable-1)%numSavedStates].absoluteHeight2)
      {
        firstUltrasound = 0;
      }
      else
      {
        firstUltrasound = 1;
      }
    }
    else if(flag1) //si ultrason1 est celui qui a incrementer, alors on le sauve comme le "devant"
      firstUltrasound = 0;
    else
      firstUltrasound = 1;
  }
  Serial.println("---------------------------------------------------------------------------------------------------");
  
  
 return true;
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
  if(firstUltrasound == 0)
  {
    if(states[frontOfTable].absoluteHeight1 < states[(frontOfTable-1)%numSavedStates].absoluteHeight1) // compare newest with 2nd newest, if lower, means your starting to come back down, so calculate height
    {
      if((states[frontOfTable].absoluteHeight1 - states[(frontOfTable-1)%numSavedStates].absoluteHeight1) < 50) // if difference is too bad, faulty value so ont do anything with it
      {
        maxHeight = states[(frontOfTable-1)%numSavedStates].absoluteHeight1;   
        Serial.println(maxHeight);
        return true;
      }
      
    }
  }
  else{
    if(states[frontOfTable].absoluteHeight2 < states[(frontOfTable-1)%numSavedStates].absoluteHeight2) // compare newest with 2nd newest, if lower, means your starting to come back down, so calculate height
    {
      if((states[frontOfTable].absoluteHeight2 - states[(frontOfTable-1)%numSavedStates].absoluteHeight2) < 50) // if difference is too bad, faulty value so ont do anything with it
      {
        maxHeight = states[(frontOfTable-1)%numSavedStates].absoluteHeight2; 
        Serial.println(maxHeight);
        return true;
      }
    }
  }
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
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(pressIn1, INPUT);
  pinMode(pressIn2, INPUT);
  
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
    if(counter == 5) // do the functions 5 times unless they return true
    {
      counter = 0;
      curr = root; //retourne au debut
    }
     else
      counter++;
  }
}



#include "pullDataXG.h"
#include <math.h>

int offSetPitch;
int offSetYaw;
int offSetRoll;

const int trigPin1 = 3;
const int echoPin1 = A3;
const int trigPin2 = 4;
const int echoPin2 = A4;

const int pressIn1 = A5;
const int pressIn2 = A6;

const int marginOfError = 2;

//Baseline orientation (not the same as offset, which is to calibrate at a universal 0 point)
//These values are used in initialise() to determine if we start on an incline.
int baseYaw=0;
int basePitch=0;
int baseRoll=0;

//pullDataXG.h deals with the accelerometer/gyro pins

class state
{
  public:
  int ultraSound[2];            //how to read:
  //pulsein(pinNum, HIGH);
  //Returns the length of time it received a HIGH pulse
  //to convert that to a distance, you have to do:
  //distance = (duration / 2) / 29.1;         //
  //boolean pressurePlate1;     //analog, lots of force applied = HIGH;
  //boolean pressurePlate2;     //analog, lots of force applied = HIGH;
  orientation myOrientation;  //xyz coordinates
  int absoluteHeight1;
  int absoluteHeight2;

  //more stuff for gyroscope?

  void getData()
  {
   // Serial.println("getData()");
    //get pulse length of 1st ultrasound sensor and convert to distance
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin1, LOW);
    int duration = pulseIn(echoPin1, HIGH);
    ultraSound[0] = duration/58.2;
    if(ultraSound[0] > 500)
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
    if(ultraSound[1] > 500)
    {
      ultraSound[1] = 0;
    }
    //Read the orientation (done directly after ultrasound as we want them to be IDEALLY read at the same time)
    myOrientation = pullStuff(); //yaw - pitch - roll

    absoluteHeight1 = calculateHeight(0);  
    absoluteHeight2 = calculateHeight(1);
    Serial.print(absoluteHeight1);
    Serial.print(absoluteHeight2);
/*
    //if pressure plate 1 is pressed enough, then pressurePlate1 = true
    if(analogRead(pressIn1) == HIGH) //pressure plate 1 pressed?
      pressurePlate1 = true;
    else
      pressurePlate1 = false;

    //if pressure plate 2 is pressed enough, then pressurePlate2 = true
    if(analogRead(pressIn2) == HIGH) //pressure plate 2 pressed?
      pressurePlate2 = true;
    else
      pressurePlate2 = false;
*/
  }
  int calculateHeight(int ultraSoundNum)
  {
    return ultraSound[ultraSoundNum]*sqrt(abs(1 - pow(sin(myOrientation.pitch - offSetPitch - basePitch), 2) - pow(sin(myOrientation.roll - offSetRoll - baseRoll), 2)));
  }
};

const int numSavedStates = 6;
state states[numSavedStates];
int frontOfTable = 0;

//This helps to see which of the two sensors is triggered first (so which is the front/back)
bool firstUltrasound; //0 for the first one, 1 for the other
//bool firstPressure;   //0 for the first one, 1 for the other

//This helps get a baseline height
int baseHeight1;
int baseHeight2;

int maxHeight = 0;

typedef bool(*functor)(void);

bool initialise()
{
  Serial.println(F("ini"));
  //these values are substracted from the angle found
  //this makes it so that when we start on an incline, that point becomes (0,0,0) until the end of the trick.
  baseYaw = states[(frontOfTable-1)%numSavedStates].myOrientation.yaw; //frontOfTable-1 is the most recently read value.
  basePitch = states[(frontOfTable-1)%numSavedStates].myOrientation.pitch;
  baseRoll = states[(frontOfTable-1)%numSavedStates].myOrientation.roll;
  Serial.println(F("tia"));
  //Get all the data at this instant into a struct
  state temp;
  temp.getData();
  Serial.println(F("sat"));
  //set the circular tables front to this state (and increment front)
  states[frontOfTable++] = temp;
  Serial.println(F("ion"));
  //set the front of the table
  frontOfTable%=numSavedStates;

  baseHeight1 = states[(frontOfTable-1)%numSavedStates].absoluteHeight1;
  baseHeight2 = states[(frontOfTable-1)%numSavedStates].absoluteHeight2;
  Serial.println(F("Initialise returned true"));
  return true;
}

bool frontLift()
{
  Serial.println(F("FRONT"));
  bool flag1=false;
  bool flag2=false;
  const int buffer = 2; //TODO, play around with this value to reduce the ammount of false positives
                        //Keeping in mind the margin of error on the ultrasound sensors is approximately +-2
  //check if either of the ultrasound sensors increases abruptly
  Serial.print(F("absoluteHeight1 = "));
  Serial.println(states[(frontOfTable)%numSavedStates].absoluteHeight1);
  if((states[(frontOfTable+1)%numSavedStates].absoluteHeight1)+3 < (states[frontOfTable].absoluteHeight1)-3){
    flag1 = true;
    Serial.println(states[frontOfTable].absoluteHeight1);
    Serial.println(states[(frontOfTable+1)%numSavedStates].absoluteHeight1);
    Serial.println(F("flag1 true"));
    }
  if((states[(frontOfTable+1)%numSavedStates].absoluteHeight2+3) < (states[frontOfTable].absoluteHeight2)-3){
    flag2 = true;
    Serial.println(states[frontOfTable].absoluteHeight2);
    Serial.println(states[(frontOfTable+1)%numSavedStates].absoluteHeight2);
    Serial.println(F("flag2 true"));
  }
  /*for(int i = (frontOfTable+1)%numSavedStates; i!= frontOfTable;++i,i%=numSavedStates)
  {
    Serial.print(F("Abrupt"))
    //pour chaque capteur ultrason, si on decremente d'un etat a un autre: on a pas fait un lift sur ce capteur
    if(states[(i+1)%numSavedStates].absoluteHeight1 <= states[i].absoluteHeight1)
        flag1 = true;
    if(states[(i+1)%numSavedStates].absoluteHeight2 <= states[i].absoluteHeight2) 
        flag2 = true;
  }    */
  if(!flag1 && !flag2) //si les 2 capteur n'on pas incrementer tout au long return false
    return false;
  else //sinon, sauve celui qui a ete lever (ce sera le "devant"), puis return true;
  {
    if(flag1 && flag2) //si les 2 capteur ont incrementer
    {
      //si les 2 on augmenter pour une raison ou une autre, alors le "devant" sera celui avec la plus grande hauteur
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
  Serial.println(F("BACK"));
  int av1 = 0;
  int av2 = 0;
  
  if(!firstUltrasound)//if first to lift was 2 check abs height 1
  {
    if((states[(frontOfTable+1)%numSavedStates].absoluteHeight1) < (states[frontOfTable].absoluteHeight1)-1)
    {
      Serial.println(F("back return true"));
      delay(1000);
      return true;
    }
  }
  else
  {
    if((states[(frontOfTable+1)%numSavedStates].absoluteHeight2) < (states[frontOfTable].absoluteHeight2)-1)
    {
      Serial.println(F("back return true"));
      delay(1000);
      return true;
    }
  }
  Serial.println("backLift returned false");
  return false;
}

bool getMaxHeight()
{
  Serial.println(F("HEIGHT"));
  
  if(states[(frontOfTable-1)%numSavedStates].absoluteHeight1 < states[(frontOfTable-2)%numSavedStates].absoluteHeight1 && states[(frontOfTable-1)%numSavedStates].absoluteHeight2 < states[(frontOfTable-2)%numSavedStates].absoluteHeight2)
  {
    if(states[(frontOfTable-1)%numSavedStates].absoluteHeight1 > states[(frontOfTable-1)%numSavedStates].absoluteHeight2)
    {
      maxHeight = states[(frontOfTable-1)%numSavedStates].absoluteHeight1; 
    }
    else
    {
      maxHeight = states[(frontOfTable-1)%numSavedStates].absoluteHeight2; 
    }
    //Serial.print("getMaxHeight returned true with ");
    //Serial.println(maxHeight);
    return true;
  }
  return false;
}

bool landed()
{
  Serial.println(F("LANDED"));
  if(states[(frontOfTable-1)%numSavedStates].absoluteHeight1 < states[(frontOfTable-1)%numSavedStates].absoluteHeight2+1 && states[(frontOfTable-1)%numSavedStates].absoluteHeight1 > states[(frontOfTable-1)%numSavedStates].absoluteHeight2-1)
  {
    //Serial.print("Landed is true");
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
    //Serial.print("Asize = ");
    arr[sizeOf] = f;
    Serial.println(F("mm"));
    sizeOf++;
    //tests.push_back(f);
    //Serial.print(F("Bsize = "));
    //Serial.println(tests.size());
  }
  void pushChild(node* c)
  {
    child[sizeOfC] = c;
    sizeOfC++;
  }
  int test()
  {
      Serial.println(F("Test()"));
      Serial.println(sizeOf);
      Serial.println("dumb");
      for (int i = 0; i < sizeOf; i++)
      {
        //Serial.println(F("dumb2"));
        //Serial.print(&tests[i]());
        bool funcVal = arr[i]();
       // bool funcVal = (*tests[0])(); // TODO crashing here bb
        Serial.print(F("funcVal"));
        if (funcVal == true)
        {
          Serial.println(F(" Worked fine"));
          return i; //test i returned true;
        }
        else
        {
          //Serial.print("Test ");
          //Serial.print((int)i);
          Serial.println(F(" didnt work"));
        }
      }
      Serial.println(F("dumb3"));
      return -1; //none are true;
  }
  node* getChild(int childNumber)
  {
    if(childNumber < sizeOfC)
    {
      //Serial.println((int)child[childNumber]);
      return child[childNumber];
    }
      
    else
    {
      //Serial.println("returning NULL from getCHild");
      return NULL;
    }
      
  }
}*root;

node* curr;

void setup() {
  //Baud and pin setup
  //Serial.println(freeMemory(),DEC);
  Serial.begin(115200); //print to serial monitor
  Serial.println(F("1"));
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(pressIn1, INPUT);
  pinMode(pressIn2, INPUT);
  
  //create decision tree
  
  
  //root
  root = (node*) malloc (sizeof(*root)); //create the root node
  Serial.print(F("3"));
  root->node1();
  root->node2(initialise);          //pass it the function pointers
  Serial.print(F("5"));

  node* onReady = (node*) malloc (sizeof(*onReady));
  Serial.print("6");
  root->pushChild(onReady);
  Serial.print("7");
  onReady->node1();
  onReady->node2(frontLift); //pass it the function pointers
  Serial.print("8");
  
  //first side of the skateboard has been lifted
  node* oneLifted = (node*) malloc (sizeof(*oneLifted));//create oneLifted node
  Serial.print("9");
  onReady->pushChild(oneLifted); //0, point root to this node
  Serial.println("10");
  oneLifted->node1();
  oneLifted->node2(backLift); //pass it the function pointers
  Serial.println("10");
/*
  node* bothLifted = (node*) malloc (sizeof(*bothLifted));//create oneLifted node
  oneLifted->child.push_back(bothLifted);
  bothLifted->node1();
  bothLifted->node2(&getMaxHeight);

  node* descending = (node*) malloc (sizeof(*descending));
  bothLifted->child.push_back(descending);
  descending->node1();
  descending->node2(&landed);*/
  Serial.println(F("prepXG"));
  prepareXG(); //calls the initialise function in the pullDataXG.h header
  state temp;
  for(int i =0; i< 100; i++)
  {
    Serial.print(i);
    temp.getData();
  }
  delay(2000);
  temp.getData();
  offSetPitch = temp.myOrientation.pitch;
  offSetRoll = temp.myOrientation.roll;
  offSetYaw = temp.myOrientation.yaw;
  
  //Serial.println(F("end of setup"));
  curr = root;
}

void loop() {
  Serial.println(F("Start of loop"));
  state temp;
  Serial.println(F("Start of loop2"));
  //Get all the data at this instant into a struct
  temp.getData();
  Serial.println(F("Start of loop3"));
  //set the circular tables front to this state (and increment front)
  states[frontOfTable++] = temp;
  Serial.println(F("Start of loop4"));
  //set the front of the table
  frontOfTable%=numSavedStates;
  Serial.println(F("Start of loop5"));

  //Test all conditions
  int nextChild = curr->test();
  Serial.println(F("Start of loop6"));
  if(nextChild != -1)
  {
    Serial.println(F("1Something returned true"));
    curr = curr->getChild(nextChild);
    
    if(curr == NULL)
    {
      curr = root;
      Serial.println(F("Got to a leaf node, going back to root"));
    }
    /*for(int i = (frontOfTable+1)%numSavedStates; i != frontOfTable; i++,i%=numSavedStates)//get brand new value after test==true
    {
      temp.getData();
      states[i] = temp;
    }*/
    temp.getData();
    states[frontOfTable++] = temp;
    frontOfTable%=numSavedStates;
  }
  else
  {
    Serial.println(F("2Something returned false"));
    curr = root; //retourne au debut
    //Serial.println("All of the tests returned false.");
  }
}



#include "pullDataXG.h"
#include <math.h>

template<typename Data>
class vector {
  size_t d_size; // Stores no. of actually stored objects
  size_t d_capacity; // Stores allocated capacity
  Data *d_data; // Stores data
public:
  vector() : 
  d_size(0), d_capacity(0), d_data(0) {
  }; // Default constructor
  vector(vector const &other) : 
  d_size(other.d_size), d_capacity(other.d_capacity), d_data(0) {
    d_data = (Data *)malloc(d_capacity * sizeof(Data));
    memcpy(d_data, other.d_data, d_size * sizeof(Data));
  }; // Copy constuctor
  /*void empty()
  {
    d_size = 0;
    d_capacity = 0;
    d_data = 0;
  }*/
  ~vector() {
    free(d_data);
  }; // Destructor
  vector &operator=(vector const &other) {
    free(d_data);
    d_size = other.d_size;
    d_capacity = other.d_capacity;
    d_data = (Data *)malloc(d_capacity * sizeof(Data));
    memcpy(d_data, other.d_data, d_size * sizeof(Data));
    return *this;
  }; // Needed for memory management
  void push_back(Data const &x) {
    if (d_capacity == d_size) resize();
    d_data[d_size++] = x;
  }; // Adds new value. If needed, allocates more space
  size_t size() const {
    return d_size;
  }; // Size getter
  Data const &operator[](size_t idx) const {
    return d_data[idx];
  }; // Const getter
  Data &operator[](size_t idx) {
    return d_data[idx];
  }; // Changeable getter
private:
  void resize() {
    d_capacity = d_capacity ? d_capacity * 2 : 1;
    Data *newdata = (Data *)malloc(d_capacity * sizeof(Data));
    memcpy(newdata, d_data, d_size * sizeof(Data));
    free(d_data);
    d_data = newdata;
  };// Allocates double the old space
};
float offSetPitch;
float offSetYaw;
float offSetRoll;

const int trigPin1 = 3;
const int echoPin1 = A3;
const int trigPin2 = 4;
const int echoPin2 = A4;

const int pressIn1 = A5;
const int pressIn2 = A6;

const int marginOfError = 2;

//Baseline orientation (not the same as offset, which is to calibrate at a universal 0 point)
//These values are used in initialise() to determine if we start on an incline.
float baseYaw=0;
float basePitch=0;
float baseRoll=0;

//pullDataXG.h deals with the accelerometer/gyro pins

struct state
{
  int ultraSound[2];            //how to read:
  //pulsein(pinNum, HIGH);
  //Returns the length of time it received a HIGH pulse
  //to convert that to a distance, you have to do:
  //distance = (duration / 2) / 29.1;         //
  boolean pressurePlate1;     //analog, lots of force applied = HIGH;
  boolean pressurePlate2;     //analog, lots of force applied = HIGH;
  orientation myOrientation;  //xyz coordinates
  float absoluteHeight1;
  float absoluteHeight2;

  //more stuff for gyroscope?

  void getData()
  {
    Serial.println("getData()");
    //get pulse length of 1st ultrasound sensor and convert to distance
    /*digitalWrite(trigPin1, LOW);
    digitalWrite(trigPin1, HIGH);
    digitalWrite(trigPin1, LOW);
    float duration = pulseIn(echoPin1, HIGH);
    ultraSound[0] = duration/58.2;
    Serial.print("ultraSound1 = ");
    Serial.println(ultraSound[0]);
    if(ultraSound[2] > 500)
    {
      ultraSound[2] = 0;
    }*/
    
    //get pulse length of 2nd ultrasound sensor and convert to distance
    digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin2, LOW);
    int duration = pulseIn(echoPin2, HIGH);
    ultraSound[1] = duration/58.2;
    Serial.print("ultraSound2 = ");
    Serial.println(ultraSound[1]);
    if(ultraSound[1] > 500)
    {
      ultraSound[1] = 0;
    }
    //Read the orientation (done directly after ultrasound as we want them to be IDEALLY read at the same time)
    myOrientation = pullStuff(); //yaw - pitch - roll
    Serial.println(myOrientation.yaw - offSetYaw);
    Serial.println(myOrientation.pitch - offSetPitch);
    Serial.println(myOrientation.roll - offSetRoll);
/*
    absoluteHeight1 = calculateHeight(0);
    Serial.print("Absolute height 1 = ");
    Serial.println(absoluteHeight1);
    */
    absoluteHeight2 = calculateHeight(1);
    Serial.print("Absolute height 2 = ");
    Serial.println(absoluteHeight2);
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
  float calculateHeight(int ultraSoundNum)
  {
    Serial.println("calculateHeight");
    //TODO, sometimes we do sqrt of a negative value.
    //Gotta like do absolute value or like 90degrees - the value we got or something. I dont know.
    Serial.println("ULTRASOUNDSHIT");
    Serial.println(ultraSound[ultraSoundNum]);
    
    return ultraSound[ultraSoundNum]*sqrt(abs(1 - pow(sin(myOrientation.pitch - offSetPitch - basePitch), 2) - pow(sin(myOrientation.roll - offSetRoll - baseRoll), 2)));
  }
};


const int numSavedStates = 10;
state states[numSavedStates];
int frontOfTable = 0;

//This helps to see which of the two sensors is triggered first (so which is the front/back)
boolean firstUltrasound; //0 for the first one, 1 for the other
boolean firstPressure;   //0 for the first one, 1 for the other

//This helps get a baseline height
float baseHeight1;
float baseHeight2;

int maxHeight = 0;

struct node
{
  vector<node*> child;
  vector<bool(*)()> tests;
  node() {
  }
  void node2(bool(*f)())
  {
    this->tests.push_back(f);
  }
  void node3(vector<bool(*)()> listOfTests)
  {
    //Serial.print("before");
    tests = listOfTests;
    //Serial.print("After");
  }
  int test()
  {
      for (int i = 0; i < tests.size(); i++)
      {
        bool funcVal = tests[i]();
        if (funcVal == true)
        {
          Serial.print("Test ");
          Serial.print((int)i);
          Serial.println(" Worked fine");
          return i; //test i returned true;

        }
        else
        {
          Serial.print("Test ");
          Serial.print((int)i);
          Serial.println(" didnt work");
        }
      }
      return -1; //none are true;
  }
  node* getChild(int childNumber)
  {
    return child[childNumber];
  }
}
*root;


bool initialise()
{
  //these values are substracted from the angle found
  //this makes it so that when we start on an incline, that point becomes (0,0,0) until the end of the trick.
  baseYaw = states[frontOfTable-1].myOrientation.yaw; //frontOfTable-1 is the most recently read value.
  basePitch = states[frontOfTable-1].myOrientation.pitch;
  baseRoll = states[frontOfTable-1].myOrientation.roll;
  
  //Get all the data at this instant into a struct
  state temp;
  temp.getData();
  //set the circular tables front to this state (and increment front)
  states[frontOfTable++] = temp;
  //set the front of the table
  frontOfTable%=numSavedStates;

  baseHeight1 = states[frontOfTable-1].absoluteHeight1;
  baseHeight2 = states[frontOfTable-1].absoluteHeight2;
  
  return true;
}

bool frontLift()
{
  bool flag1=false;
  bool flag2=false;
  const int buffer = 2; //TODO, play around with this value to reduce the ammount of false positives
                        //Keeping in mind the margin of error on the ultrasound sensors is approximately +-2
  //check if either of the ultrasound sensors increases abruptly
  for(int i = (frontOfTable+1)%numSavedStates; i!= frontOfTable;++i%numSavedStates)
  {
    //pour chaque capteur ultrason, si on decremente d'un etat a un autre: on a pas fait un lift sur ce capteur
    if(states[(i+1)%numSavedStates].absoluteHeight1 <= states[i].absoluteHeight1)
        flag1 = true;
    if(states[(i+1)%numSavedStates].absoluteHeight2 <= states[i].absoluteHeight2) 
        flag2 = true;
  }    
  if(flag1 && flag2) //si les 2 capteur n'on pas incrementer tout au long return false
    return false;
  else //sinon, sauve celui qui a ete lever (ce sera le "devant"), puis return true;
  {
    if(!flag1 && !flag2) //si les 2 capteur ont incrementer
    {
      //si les 2 on augmenter pour une raison ou une autre, alors le "devant" sera celui avec la plus grande hauteur
      if(states[frontOfTable-1].absoluteHeight1 > states[frontOfTable-1].absoluteHeight2)
      {
        firstUltrasound = 0;
      }
      else
      {
        firstUltrasound = 1;
      }
    }
    if(!flag1) //si ultrason1 est celui qui a incrementer, alors on le sauve comme le "devan"
      firstUltrasound = 0;
    else
      firstUltrasound = 1;
  }
 return true;
}
bool backLift()
{
  for(int i = (frontOfTable+1)%numSavedStates; i!= frontOfTable;++i%numSavedStates)
  {
    if(firstUltrasound)
    {
      if(states[(i+1)%numSavedStates].absoluteHeight1 <= states[i].absoluteHeight1)
          return false;
    }
    else
    {
      if(states[(i+1)%numSavedStates].absoluteHeight2 <= states[i].absoluteHeight2) 
          return false;
    }
  }
  return true;
}

bool getMaxHeight()
{
  if(states[(frontOfTable-1)%numSavedStates].absoluteHeight1 < states[(frontOfTable-2)%numSavedStates].absoluteHeight1 && states[(frontOfTable-1)%numSavedStates].absoluteHeight2 < states[(frontOfTable-2)%numSavedStates].absoluteHeight2)
  {
    if(states[(frontOfTable-1)%numSavedStates].absoluteHeight1 > states[(frontOfTable-1)%numSavedStates].absoluteHeight2)
      maxHeight = states[(frontOfTable-1)%numSavedStates].absoluteHeight1; 
    else
      maxHeight = states[(frontOfTable-1)%numSavedStates].absoluteHeight2; 
  }
}

bool landed()
{
  if(states[(frontOfTable-1)%numSavedStates].absoluteHeight1 < states[(frontOfTable-1)%numSavedStates].absoluteHeight2+1 && states[(frontOfTable-1)%numSavedStates].absoluteHeight1 > states[(frontOfTable-1)%numSavedStates].absoluteHeight2-1)
  {
    return true;
  }
}

void setup() {
  //Baud and pin setup
  Serial.begin(9600); //print to serial monitor
  Serial.print("1");
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(pressIn1, INPUT);
  pinMode(pressIn2, INPUT);

 
  
  //create decision tree
  
  Serial.print("2");
  
  //root
  root = (node*) malloc (sizeof(*root)); //create the root node
  Serial.print("3");
  root->node2(&initialise);          //pass it the function pointers
  Serial.print("5");

  node* onReady = (node*) malloc (sizeof(*onReady));
  Serial.print("6");
  root->child.push_back(onReady);
  Serial.print("7");
  onReady->node2(&frontLift); //pass it the function pointers
  Serial.print("8");
  
  //first side of the skateboard has been lifted
  node* oneLifted = (node*) malloc (sizeof(*oneLifted));//create oneLifted node
  Serial.print("9");
  onReady->child.push_back(oneLifted); //0, point root to this node
  Serial.print("10");
  oneLifted->node2(&backLift); //pass it the function pointers

  
  node* bothLifted = (node*) malloc (sizeof(*bothLifted));//create oneLifted node
  oneLifted->child.push_back(bothLifted);
  bothLifted->node2(&getMaxHeight);

  node* descending = (node*) malloc (sizeof(*descending));
  bothLifted->child.push_back(descending);
  descending->node2(&landed);
  
  prepareXG(); //calls the initialise function in the pullDataXG.h header
  state temp;
  for(int i =0; i< 100; i++)
  {
    temp.getData();
  }
  temp.getData();
  offSetPitch = temp.myOrientation.pitch;
  offSetRoll = temp.myOrientation.roll;
  offSetYaw = temp.myOrientation.yaw;
  Serial.println(offSetPitch);
  Serial.println(offSetRoll);
  Serial.println(offSetYaw);
  delay(5000);
  Serial.println("end of setup");
  
}

node* curr = root;

void loop() {
  //Serial.println("Start of loop");
  state temp;

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
  }
  else
  {
    //Serial.println("All of the tests returned false.");
  }
}



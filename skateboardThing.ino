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
  ~vector() {
    free(d_data);
    d_size = 0; //Had to add this in as it kept old size after freeing.
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

const int xIn = A0;
const int yIn = A1;
const int zIn = A2;
//more stuff for gyro?

const int trigPin1 = 2;
const int echoPin1 = A3;
const int trigPin2 = 3;
const int echoPin2 = A4;

const int pressIn1 = A5;
const int pressIn2 = A6;

const int marginOfError = 2;

struct state
{
  int ultraSound[2];            //how to read:
  //pulsein(pinNum, HIGH);
  //Returns the length of time it received a HIGH pulse
  //to convert that to a distance, you have to do:
  //distance = (duration / 2) / 29.1;         //
  boolean pressurePlate1;     //analog, lots of force applied = HIGH;
  boolean pressurePlate2;     //analog, lots of force applied = HIGH;
  int x;                      //Output acceleration on X axis as analog voltage between 0V and 5V
  int y;                      //Output acceleration on Y axis as analog voltage between 0V and 5V
  int z;                      //Output acceleration on Z axis as analog voltage between 0V and 5V

  //more stuff for gyroscope?

  void getData()
  {
    Serial.println("getData()");
    
    //get pulse length of 1st ultrasound sensor and convert to distance
    digitalWrite(trigPin1, LOW);
    digitalWrite(trigPin1, HIGH);
    digitalWrite(trigPin1, LOW);
    int duration = pulseIn(echoPin1, HIGH);
    ultraSound[0] = duration/58.2;
    Serial.print("ultraSound1 = ");
    Serial.println(ultraSound[0]);
    
    //get pulse length of 2nd ultrasound sensor and convert to distance
    digitalWrite(trigPin2, LOW);
    digitalWrite(trigPin2, HIGH);
    digitalWrite(trigPin2, LOW);
    duration = pulseIn(echoPin2, HIGH);
    ultraSound[1] = duration/58.2;
    Serial.print("ultraSound2 = ");
    Serial.println(ultraSound[1]);
    
    //Read ACCELERATIONS (done directly after ultrasound as we want them to be IDEALLY read at the same time)
    x = analogRead(xIn);
    y = analogRead(yIn);
    z = analogRead(zIn);

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

    //TODO, convert to an orientation, these are only accelerations

  }
  int calculateHeight1()
  {
    //TODO
    return 0; //placeholder
  }
  int calculateHeight2()
  {
    //TODO
    return 0;
  }
};


const int numSavedStates = 10;
state states[numSavedStates];
int frontOfTable = 0;

//This helps to see which of the two sensors is triggered first (so which is the front/back)
boolean firstUltrasound; //0 for the first one, 1 for the other
boolean firstPressure;   //0 for the first one, 1 for the other

//This helps get a baseline height
int baseHeight;

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
    tests = listOfTests;
  }
  int test()
  {
    while (true)
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
  }
  node* getChild(int childNumber)
  {
    return child[childNumber];
  }
}
*root;


bool initialise()
{
  //TODO implement a getHeight function in struct state (take into account orientation and do pythagore to get absolute height
  /*int x = states[0];
  int average=x;
  for(int i=1; i< numSavedStates; i++)
  {
    if(x <= states[i] - marginOfError || x >= states[i] + marginOfError) //if states stays relatively the same (within margin of error) for numSavedStates iterations
    {
      return false;
    }
    total+=states[i];
  }
  average/=numSavedStates;
  baseHeight = average;*/
  return true;
}

bool frontLift()
{
  const int buffer = 2; //TODO, play around with this value to reduce the ammount of false positives
                        //Keeping in mind the margin of error on the ultrasound sensors is approximately +-2
  //check if either of the ultrasound sensors increases abruptly
  if(states[frontOfTable-1].ultraSound[0]+buffer+marginOfError < states[frontOfTable].ultraSound[0])
  {
    Serial.print("ultrasoundSensor 1 increased abruptly");
    firstUltrasound = 0; //this sensor was the first to lift (useful for backLift function)
    return true;
  }
  if(states[frontOfTable-1].ultraSound[1]+buffer+marginOfError < states[frontOfTable].ultraSound[1])
  {
    Serial.print("ultraSound sensor 2 increased abruptly");
    firstUltrasound = 1; //this sensor was the first to lift (useful for backLift function)
    return true;
  }
  return false;
}
bool backLift()
{
  //the first ultrasound sensor to get "triggered" by frontLift() is saved in the bool global variable "firstUltrasound" (0 for ultraSound[0] and 1 for ultraSound[1])
  //check to see if the OTHER one passes the height of the first
  //as in: return true if the height of ultraSound[1] > height of ultraSound[0]
  //Once it gets there, save the height and use that to calculate the score.
  //TODO, make sure to do pythagore with the accelerometer to calculate height from the ground and not distance to nearest object like it is now.
  
  return false;
}

void setup() {
  //Baud and pin setup
  Serial.begin(9600); //print to serial monitor
  //Serial.print("1");
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(pressIn1, INPUT);
  pinMode(pressIn2, INPUT);
  pinMode(xIn, INPUT);
  pinMode(yIn, INPUT);
  pinMode(zIn, INPUT);

  //create decision tree
  vector<bool(*)()> funcs;
  
  //Serial.print("2");
  
  //root
  root = (node*) malloc (sizeof(*root)); //create the root node
  //Serial.print("3");
  funcs.push_back(&frontLift); //0
  //Serial.print("4");
  root->node3(funcs);          //pass it the function pointers
  //Serial.print("5");
  //free memory in funcs
  funcs.~vector();
  //funcs.vector();
  
  //first side of the skateboard has been lifted
  node* oneLifted = (node*) malloc (sizeof(*oneLifted));//create oneLifted node
  Serial.print("6");
  root->child.push_back(oneLifted); //0, point root to this node
  Serial.print("7");
  funcs.push_back(&backLift); //0
  Serial.print("8");
  oneLifted->node3(funcs); //pass it the function pointers
  Serial.print("9");
  //funcs.~vector();
  //funcs.vector();
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

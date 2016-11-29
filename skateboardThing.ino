
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


//more stuff for gyro?

const int xIn = A0;
const int yIn = A1;
const int zIn = A2;

const int trigPin1 = 2;
const int echoPin1 = A3;
const int trigPin2 = 3;
const int echoPin2 = A4;

const int pressIn1 = A5;
const int pressIn2 = A6;

struct state
{
  int ultraSound1;            //how to read:
  //pulsein(pinNum, HIGH);
  //Returns the length of time it received a HIGH pulse
  //to convert that to a distance, you have to do:
  //distance = (duration / 2) / 29.1;
  int ultraSound2;            //
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
    ultraSound1 = duration/58.2;
    Serial.println("ultraSound1 = ");

    //get pulse length of 2nd ultrasound sensor and convert to distance
    digitalWrite(trigPin2, LOW);
    digitalWrite(trigPin2, HIGH);
    digitalWrite(trigPin2, LOW);
    duration = pulseIn(echoPin2, HIGH);
    ultraSound2 = duration/58.2;
    Serial.println("ultraSound2 = ");

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

    //Read ACCELERATIONS
    x = analogRead(xIn);
    y = analogRead(yIn);
    z = analogRead(zIn);

    //TODO, convert to an orientation, these are only accelerations

  }
};

struct node
{
  vector<node*> child;
  vector<bool(*)()>* tests;
  node() {
  }
  void node2(bool(*f)())
  {
    this->tests.push_back(f);
  }
  void node3(vector<bool(*)()>* listOfTests)
  {
    tests = listOfTests;
    return;
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

bool frontLift()
{
  //check if either of the ultrasound sensors increases abruptly

  if(true)
    return true;
  else
    return false;

}
bool backLift()
{
  //check if either of the ultrasound sensors increases abruptly
  //return false; //placeholder
  return false;
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
  pinMode(xIn, INPUT);
  pinMode(yIn, INPUT);
  pinMode(zIn, INPUT);

  //create decision tree
  vector<bool(*)()> funcs;
  Serial.print("2");
  root = (node*) malloc (sizeof(*root)); //create the root node
  Serial.print("3");
  funcs.push_back(&frontLift); //0
  Serial.print("4");
  root->node3(&funcs);          //pass it the function pointers
  Serial.print("5");
  node* oneLifted = (node*) malloc (sizeof(*oneLifted));//create oneLifted node
  Serial.print("6");
  root->child.push_back(oneLifted); //0, point root to this node
  funcs.push_back(&backLift); //0
  oneLifted->node3(funcs); //pass it the function pointers
}

node* curr = root;

const int numSavedStates = 10;
state states[numSavedStates];
int frontOfTable = 0;

void loop() {
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
    Serial.println("This tick, all of the tests returned false.");
  }
}


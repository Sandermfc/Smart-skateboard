
template<typename Data>

class vector {
    size_t d_size; // Stores no. of actually stored objects
    size_t d_capacity; // Stores allocated capacity
    Data *d_data; // Stores data
  public:
    vector() : d_size(0), d_capacity(0), d_data(0) {}; // Default constructor
    vector(vector const &other) : d_size(other.d_size), d_capacity(other.d_capacity), d_data(0) {
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

int ultrasound1;
int ultrasound2;

struct node
{
  vector<node*> child;
  vector<bool(*)()> tests;
  node() {}
  void node2(bool(*f)())
  {
    this->tests.push_back(f);
  }
  void node3(vector<bool(*)()> tests)
  {
    this->tests = tests;
  }
  int loop()
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
          
        }
        else
        {
          Serial.print("Test ");
          Serial.print((int)i);
          Serial.println(" didnt work");
        }
      }
    }
  }
};

bool frontLift()
{
  //check if either of the ultrasound sensors increases abruptly
  //return true; //placeholder
  
  return true;
  
}
bool backLift()
{
  //check if either of the ultrasound sensors increases abruptly
  //return false; //placeholder
  return false;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(300);

  vector<bool(*)()> funcs;
  
  node* root = (node*) malloc (sizeof(*root)); //create the root node
  
  funcs.push_back(&frontLift); //0
  root->node3(funcs);          //pass it the function pointers

  node* oneLifted = (node*) malloc (sizeof(*oneLifted));//create oneLifted node
  root->child.push_back(oneLifted); //0
  
  funcs.push_back(&backLift); //0
  oneLifted->node3(funcs); //pass it the function pointers

  root->loop();
}

void loop() {
  // put your main code here, to run repeatedly:
  //root->loop();
}

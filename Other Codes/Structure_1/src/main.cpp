#include <Arduino.h>
struct student
{
  String name;
  int age;
  int roll_no;
};

student increaseAge(student *B)
{
  B->age = B->age + 1;
  return *B;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();

  student A = {"Yash", 18, 26};
  Serial.println(A.age);
  student D = increaseAge(&A);
  Serial.println(A.age);
  Serial.println(D.age);
}
void loop()
{
  // put your main code here, to run repeatedly:
}
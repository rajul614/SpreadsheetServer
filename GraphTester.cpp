#include <string>
#include <iostream>
#include "DependencyGraph.h"
using namespace std;


int main(int arc, char *argv[]){
  bool TestFailed = false;
  
  cout << "Tests Started" << endl;

  DependencyGraph c; // Declares graph
  c.AddDependency("A1", "B3");
  c.AddDependency("A2", "B4");
  c.AddDependency("A2", "C4");
  c.AddDependency("A2", "C5");

  //if(){
  //  cout << "First Add Test Failed" << endl;
  //  TestFailed = truel;
  //}
  TestFailed = true;
  c.AddDependency("C" , "C" );


  std::cout << "Tests Ended" << std::endl;

  if(TestFailed){
    cout << "Tests Did Not all pass" << endl;
    return 1;
  }

  return 0;

}

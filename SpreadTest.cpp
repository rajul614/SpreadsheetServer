#include "Spreadsheet.h"
#include <string.h>
#include <iostream>

int main(){
 
  Spreadsheet e("test.txt");
  //e.setCell("A1", "Testing 123");
  e.setCell("B2", "=A2 + 1");
  try
    {
      e.setCell("A2", "=B2");
    }
  catch (int i)
    {
      std::cout << "dsfds" << std::endl;
      if (i == 11)
	std::cout << "Circular dependency caught" << std::endl;
    }
}
